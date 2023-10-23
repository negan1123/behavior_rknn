/*
 * mqtt_manage.h
 * MQTT管理类，实现包含MQTT客户端连接、断开、发布、订阅等基本MQTT操作
 * 同时需要实现客户端发送请求、接收响应的方法，以及客户端接收服务器的指令、对服务器响应的方法
 *
 *  Created on: 2019年11月6日
 *      Author: bnkj
 */

#ifndef MQTT_MANAGE_H_
#define MQTT_MANAGE_H_
#include <iostream>
#include <functional>
#include <thread_raii.h>
#include <spdlog/spdlog.h>

#include <mqtt/async_client.h>

namespace mq {

class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		spdlog::error(name_ + " failure");
		if (tok.get_message_id() != 0) {
			spdlog::error(" for token: [" + std::to_string(tok.get_message_id()) + "]");
		}
	}

	void on_success(const mqtt::token& tok) override {
		spdlog::debug(name_ + " success");
		if (tok.get_message_id() != 0) {
			spdlog::debug(" for token: [" + std::to_string(tok.get_message_id()) + "]");
		}
		auto topics = tok.get_topics();
		if (topics && !topics->empty()) {
			for(size_t i=0; i < topics->size(); i++) {
				spdlog::debug("token topic: '" + (*topics)[i] + "'");
			}
		}
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};

//MQ订阅消息达到的回调函数
using ON_MESSAGE_ARRIVED = std::function<void(const std::string& topic, const std::string& msg)>;

class MQTT_Manage {
public:
	MQTT_Manage()=default;
	//拷贝构造函数
	MQTT_Manage(const MQTT_Manage& manage);
	virtual ~MQTT_Manage();

	//使用服务器地址、端口、用户名等与MQTT服务器建立连接
	//is_loop=true，在经过10次尝试连接后如果没有连接成功，将一直进行尝试，每9次之间等待1分钟
	bool connect(bool is_loop = false);
	//断开与服务器的连接。
	bool disconnect();
	//向服务器指定的主题发送一个消息。
	bool publish(std::string topic, std::string msg, int qos=2, bool retained=false);
	//订阅指定topic。
	bool subscribe(std::string topic, int qos=2);
	//订阅指定topic集合，使用方法如下(同时订阅两个主题)。
	/**
	 * 		std::initializer_list<string> topics{"test", "device_recv_topic"};
	 * 		auto collection_ptr = std::make_shared<mqtt::string_collection>(topics);
	 * 		if(gvar::mq_ptr->subscribe(collection_ptr)) {
	 * 			spdlog::debug("订阅主题成功");
	 * 		}
	 */
	bool subscribe(mqtt::const_string_collection_ptr topicFilters, int qos=2);
	//取消订阅某个topic。
	bool unsubscribe(std::string topic);
	//取消订阅topic集合。
	bool unsubscribe(mqtt::const_string_collection_ptr topicFilters);

private:
	//尝试与服务器建立连接，连接成功返回true，失败返回false
	bool try_connect(mqtt::connect_options connOpts);
	//当客户端连接服务器失败或者断开连接后重新尝试多次连接
	//try_nums代表重试次数，默认是9次
	bool try_reconnect(const int try_nums=9);
	//监听订阅消息是否到达，如果到达接收消息并调用回调函数
	void wait_message_arrived();
	//释放资源
	void destroy();
	
	/**
 	* @brief 连接监控线程处理线程
 	*/
	void connMonitor();

	/**
 	* @brief 订阅消息监控处理线程
 	*/
	void messageConsume();

public:
	//订阅消息达到的回调函数
	ON_MESSAGE_ARRIVED onMessageArrived{nullptr};

	//MQTT服务器地址
	std::string host;
	//端口号
	int port;
	//客户端编号
	std::string clientId;
	//用户名
	std::string user;
	//认证密码
	std::string password;

private:
	//MQTT客户端对象
	mqtt::async_client* cli{nullptr};
	//订阅标识，当这个标识设置成false，取消订阅
	std::atomic<bool> subscribe_flag{false};
	//是否启动了MQTT连接监控
	std::atomic<bool> conn_monitor{false};

	//连接监控线程处理线程
	libcpp::Thread thread_monitor;
	//获取订阅消息的线程
	libcpp::Thread thread_message;

	//订阅的主题
	std::string sub_topic;
	//订阅的主题集合
	mqtt::const_string_collection_ptr sub_topics;
};

} /* namespace mq */

#endif /* MQTT_MANAGE_H_ */
