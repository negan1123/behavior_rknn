/*
 * mqtt_manage.cpp
 *
 *  Created on: 2019年11月6日
 *      Author: bnkj
 */

#include <mqtt_manage.h>

namespace mq {
//拷贝构造函数
MQTT_Manage::MQTT_Manage(const MQTT_Manage& _manage) {
	this->host = _manage.host;
	this->port = _manage.port;
	this->user = _manage.user;
	this->password = _manage.password;
	this->onMessageArrived = _manage.onMessageArrived;
	sub_topics = nullptr;
}

MQTT_Manage::~MQTT_Manage() {
	destroy();
}

void MQTT_Manage::destroy() {
	//退出订阅循环
	subscribe_flag = false;
	//退出连接监控线程
	conn_monitor = false;

	if(cli != nullptr) {
		cli->stop_consuming();

		thread_monitor.join();
		thread_message.join();

		if(disconnect()) {
			spdlog::debug("正在释放mqtt客户端资源...");
			delete cli;
			cli = nullptr;
			spdlog::debug("mqtt客户端资源释放完毕...");
		}
	}
}

/**
 * 当客户端连接服务器失败或者断开连接后重新尝试多次连接
 * 参数：
 * 		try_nums代表重试次数，默认是9次
 * 返回值：
 * 		true 重连接成功， false 重连接失败
 */
bool MQTT_Manage::try_reconnect(const int try_nums)
{
	//默认重连9次，加上首次连接，一共尝试10次
	for (int i=0; i< try_nums && !cli->is_connected(); ++i) {
		try {
			spdlog::debug("第" + std::to_string(i + 1) + "次尝试与MQTT server建立连接...");
			cli->reconnect();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		catch (const mqtt::exception& e) {
			spdlog::error("连接MQTT server失败: '" + cli->get_server_uri() + "'");
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}

	bool ret = cli->is_connected();
	return ret;
}

/**
 * 尝试与服务器建立连接，连接成功返回true，多次尝试连接失败返回false
 */
bool MQTT_Manage::try_connect(mqtt::connect_options connOpts)
{
	bool result = false;

	try {
		spdlog::debug("正在与MQTT server建立连接...");
		cli->connect(connOpts)->wait();
	}
	catch (const mqtt::exception& e) {
		spdlog::error("连接MQTT server失败: '" + cli->get_server_uri() + "'");
	}

	result = cli->is_connected();
	return result;
}

//使用服务器地址、端口、用户名等与MQTT服务器建立连接
//is_loop=true 一直进行尝试，每次之间等待15分钟; false只连接一次
//返回0表示成功，-1表示失败
bool MQTT_Manage::connect(bool is_loop) {
	bool ret = false;

	// 设置连接参数
	mqtt::connect_options connOpts;
	// 与MQ服务器保持心跳的时间：20s
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);
	mqtt::string_ref user_name{user};
	mqtt::string_ref user_pwd{password};
	connOpts.set_user_name(user_name);
	connOpts.set_password(user_pwd);

	std::string server_address = "tcp://" + host + ":" + std::to_string(port);
	if(cli == nullptr) {
		std::string cid = clientId + "-" + std::to_string(time(NULL));
		spdlog::info("MQTT clientId:{}", cid);
		cli = new mqtt::async_client(server_address, cid);
	}

	//首次连接失败，继续尝试若干次
	ret = try_connect(connOpts);

	//如果没有成功，如果is_loop=true，将不断尝试连接，直到成功，每一个9次连接之间等待15s
	while(!ret && is_loop) {
		//等待15秒
		std::this_thread::sleep_for(std::chrono::seconds(15));
		//ret = try_reconnect();
		ret = try_connect(connOpts);
	}
	
	//是否启动了连接监控线程
	if(!conn_monitor) {
		conn_monitor = true;
		// 开启连接监控线程
		thread_monitor.reset(&MQTT_Manage::connMonitor, this);
	}

	return ret;
}

/**
	* @brief 连接监控处理线程
	*/
void MQTT_Manage::connMonitor() {
	while(conn_monitor) {
		//每5秒检查一次MQTT连接，如果断开立刻重连
		std::this_thread::sleep_for(std::chrono::seconds(5));
		if (!cli->is_connected()) {
			spdlog::error("与服务器连接意外中断，重新尝试连接。。。");

			//一直连接，直到连接成功
			if(connect(true)) {
				spdlog::debug("与服务器连接已经恢复");
				if(sub_topics && subscribe(sub_topics)) {
					spdlog::debug("设备订阅主题成功");
				}
			}
		}
	}
}

//断开与服务器的连接。返回0表示成功，-1表示失败
bool MQTT_Manage::disconnect() {
	bool ret = false;

	try {
		spdlog::debug("断开与MQTT server的连接...");

		if(cli->is_connected()) {
			cli->disconnect()->wait();
		}

		spdlog::debug("OK");

		ret = true;
	}
	catch (const mqtt::exception& exc) {
		spdlog::error(exc.what());
	}

	return ret;
}

//向服务器指定的主题发送一个消息。返回0表示成功，-1表示失败
bool MQTT_Manage::publish(std::string topic, std::string msg, int qos, bool retained) {
	bool ret = false;

	try {
		// 只有处于连接状态的情况下才能发送
		if(cli->is_connected()) {
			mqtt::message_ptr pubmsg = mqtt::make_message(topic, msg);
			pubmsg->set_qos(qos);
			pubmsg->set_retained(retained);
			cli->publish(pubmsg)->wait();

			ret = true;
		}
		else {
			spdlog::debug("与服务器连接意外中断，发送失败！");
		}
	}
	catch(mqtt::exception& e) {
		spdlog::error(e.what());
	}

	return ret;
}

//监听订阅消息是否到达，如果到达接收消息并调用回调函数
void MQTT_Manage::wait_message_arrived() {
	//只有还没有订阅任何主题的时候才通过线程接收消息
	if(!subscribe_flag) {
		subscribe_flag = true;
		cli->stop_consuming();
		cli->start_consuming();

		// 开启订阅消息处理线程
		thread_message.reset(&MQTT_Manage::messageConsume, this);
	}
}

/**
 * @brief 订阅消息监控处理线程
 */
void MQTT_Manage::messageConsume() {
	while(subscribe_flag) {
		try {
			//auto msg = cli->consume_message();
			//接收消息，10s等待超时
			auto msg = cli->try_consume_message_for(std::chrono::seconds(10));

			if (msg) {
				//调用回调函数
				if(onMessageArrived != nullptr) {
					onMessageArrived(msg->get_topic(), msg->to_string());
				}
			}
			else {
				spdlog::debug("没有收到订阅消息!");
			}
		}
		catch(mqtt::exception& e) {
			spdlog::error(e.what());
		}

		//每个循环等待5ms
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	
}

//订阅指定topic。返回0表示成功，-1表示失败
bool MQTT_Manage::subscribe(std::string topic, int qos) {
	bool ret = false;

	//开始订阅。subListener_是执行订阅的回调函数
	action_listener subListener_("Subscription");
	try {
		if(cli->is_connected()) {
			sub_topic = topic;
			cli->subscribe(topic, qos, nullptr, subListener_)->wait();
			if(!subscribe_flag) {
				//首次订阅，增加消息监听
				wait_message_arrived();
			}

			ret = true;
		}
	}
	catch(mqtt::exception& e) {
		spdlog::error(e.what());
	}

	return ret;
}

//订阅指定topic集合。返回0表示成功，-1表示失败
bool MQTT_Manage::subscribe(mqtt::const_string_collection_ptr topicFilters, int qos) {
	bool ret = false;

	//我们只传入一个qos，需要根据topicFilters的长度构造数组并且每个值都设置成qos
	int len = topicFilters->size();
	std::vector<int> qos_list;
	for(int i = 0; i < len; i++) {
		qos_list.push_back(qos);
	}

	//开始订阅。subListener_是执行订阅的回调函数
	action_listener subListener_("Subscription");
	try {
		if(cli->is_connected()) {
			//将订阅的主题集合保存起来
			sub_topics = topicFilters;
			cli->subscribe(topicFilters, qos_list, nullptr, subListener_)->wait();
			if(!subscribe_flag) {
				//首次订阅，增加消息监听
				wait_message_arrived();
			}

			ret = true;
		}
	}
	catch(mqtt::exception& e) {
		spdlog::error(e.what());
	}

	return ret;
}

//取消订阅某个topic。返回0表示成功，-1表示失败
bool MQTT_Manage::unsubscribe(std::string topic) {
	bool ret = false;

	try {
		cli->unsubscribe(topic)->wait();

		ret = true;
	}
	catch(mqtt::exception& e) {
		spdlog::error(e.what());
	}

	return ret;
}

//取消订阅topic集合。返回0表示成功，-1表示失败
bool MQTT_Manage::unsubscribe(mqtt::const_string_collection_ptr topicFilters) {
	bool ret = false;

	try {
		cli->unsubscribe(topicFilters)->wait();

		ret = true;
	}
	catch(mqtt::exception& e) {
		spdlog::error(e.what());
	}

	return ret;
}

} /* namespace mq */
