/*
 * LocalBus.cpp
 *
 *  Created on: Dec 14, 2021
 *      Author: wy
 */

#include <vector>
#include <localBus.h>

LocalBus::LocalBus()
{

}

LocalBus::~LocalBus()
{

}

bool LocalBus::init()
{
	mq_ptr = new mq::MQTT_Manage();
	mq_ptr->clientId = "voiceService";
	mq_ptr->host = std::string("127.0.0.1");
	mq_ptr->port = 1883;
	mq_ptr->user = std::string();
	mq_ptr->password = std::string();
	std::string sub_topic = "signal_up";
	mq_ptr->onMessageArrived = [&](const std::string& topic, const std::string& msg){
		json mqttJs;
		try
		{
			mqttJs = json::parse(msg);
			json head;
			head = mqttJs.at("head");
			std::string event = head.at("event");
			if(event == "_VOICE")
			{	
				sessionID = head.at("sessionID");
				json body;
				body = mqttJs.at("body");
				if(cb)
					cb(body);
			}
		}
		catch(...)
		{
			spdlog::error("json 解析错误!");
		}
	};

	if(mq_ptr->connect(true))
	{
		spdlog::debug("设备连接成功");
		std::vector<std::string> topics{sub_topic};
		auto collection_ptr = std::make_shared<mqtt::string_collection>(topics);
		if(mq_ptr->subscribe(collection_ptr)) {
			spdlog::debug("设备订阅主题成功");
		}
	}

	return true;
}

void LocalBus::response(const int &ret)
{
	json head;
	json body;
	json res;
	head["event"] = std::string("_VOICE_");
	head["sessionID"] = sessionID;
	body["resultCode"] = ret;
	res["head"] = head;
	res["body"] = body;
	mq_ptr->publish(std::string("signal_down"), res.dump());
}

void LocalBus::setOnRecieveMsg(onRecieveMsg _cb)
{
	cb = _cb;
}


