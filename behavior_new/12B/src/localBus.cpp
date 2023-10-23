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
	mq_ptr->clientId = "behavior";
	mq_ptr->host = std::string("127.0.0.1");
	mq_ptr->port = 1883;
	mq_ptr->user = std::string();
	mq_ptr->password = std::string();
	std::string sub_topic = "location_report";
	mq_ptr->onMessageArrived = [&](const std::string& topic, const std::string& msg){
		json mqttJs;
		try
		{
			mqttJs = json::parse(msg);
			json head;
			head = mqttJs.at("head");
			std::string event = head.at("event");
			if(event == "_LOCATION")
			{
				sessionID = head.at("sessionID");
				json body;
				body = mqttJs.at("body");
				if(body["republish"] == 0)
				{
					if(cb)
					{
						cb(body);
					}
				}
			}
		}
		catch(...)
		{
			printf("json parse error!");
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

void LocalBus::publish(const int &behaviorInt)
{
//	std::string text;
//	if(behaviorInt & 0x01)
//		text += "发现闭眼行为 ";
//	if(behaviorInt & 0x02)
//		text += "发现打哈欠行为 ";
//	if(behaviorInt & 0x04)
//		text += "发现抽烟行为 ";
//	if(behaviorInt & 0x08)
//		text += "发现打电话行为 ";
//	if(behaviorInt & 0x10)
//		text += "发现喝水行为 ";
//	if(behaviorInt & 0x10)
//		text += "发现遮挡摄像头行为 ";
//	if(behaviorInt & 0x10)
//		text += "发现左顾右盼行为 ";
//	if(!text.empty())
//	{
//		json head;
//		json body;
//		json pub;
//		time_t now;
//		now = time(NULL);
//		head["event"] = std::string("_VOICE");
//		head["sessionID"] = std::to_string(now);
//		body["text"] = text;
//		body["speed"] = 5;
//		body["pitch"] = 5;
//		body["volume"] = 5;
//		body["voiceType"] = 3;
//		pub["head"] = head;
//		pub["body"] = body;
//		mq_ptr->publish(std::string("signal_down"), pub.dump());
//	}
}

void LocalBus::setOnRecieveMsg(onRecieveMsg _cb)
{
	cb = _cb;
}


