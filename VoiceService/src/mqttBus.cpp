#include <mqttBus.h>

/**@brief 构造函数
 * @param	None
 * @return	None
 */
mqttBus::mqttBus()
{
}

/**@brief 初始化函数
 * @param	None
 * @return	int			初始化成功与否
 */
int mqttBus::init()
{
	mq_ptr = new mq::MQTT_Manage();
	mq_ptr->clientId = VAITerminal_JTConf::Instance()->getDeviceDevId() + "_voiceService";
    /*修改*/
	mq_ptr->host = VoiceServiceConf::Instance()->getMqttHost();		  // 获取mqtt服务器地址
	mq_ptr->port = VoiceServiceConf::Instance()->getMqttPort();			  // 获取mqtt服务器端口号
	mq_ptr->user = VoiceServiceConf::Instance()->getMqttUserName();	  // 获取mqtt服务器用户名
	mq_ptr->password = VoiceServiceConf::Instance()->getMqttPassword(); // 获取mqtt服务器密码

	/*
		获取服务器订阅主题
		服务器version >= 2.0 订阅设备号
		服务器version < 2.0 或没有加载到version订阅terminal_signal_down
	*/
	std::string subTopic;
	// 获取信令版本号，根据信令版本号决定订阅的主题
	std::string version = SignalConf::Instance()->getVersion();
	// 没有设置版本号或者版本号是1.0.0，按照1.0.0版本处理
	if(version.size() == 0 || version == "1.0.0")
	{
		// 1.0.0信令订阅的主题：指定主题
		subTopic = "terminal_signal_down";
	}
	else {
		// 2.0.0信令订阅的主题：设备号
		subTopic = VoiceServiceConf::Instance()->getDeviceDevId();
	}

	std::cout<<subTopic<<endl;

	mq_ptr->onMessageArrived = [=](const std::string &topic, const std::string &msg)
	{
		// 此处处理收到的订阅消息
		spdlog::debug("收到服务器响应：{}",msg);
		json mqttJs;
		json head;
        json body;
		try
		{
			
			mqttJs = json::parse(msg);
			head = mqttJs.at("head");
            body = mqttJs.at("body");
			std::cout<<"head:"<<head["event"]<<std::endl;
			// 检查device字段，若设备不同则不执行回执
			if (head["terminalCode"] == VoiceServiceConf::Instance()->getDeviceDevId())
			{
                // 仅接收 “_VOICE_VERSION_” 事件消息
                if (head["event"] == "_VOICE_VERSION_")
                {
                    if (cb1)
					{
						cb1(body);
					}
                }
			}
		}
		catch (...)
		{
			spdlog::debug("json 解析错误!");
		}
	};

	if (mq_ptr->connect(true))
	{
		// 设备端关注主题：terminal_topic
		std::vector<std::string> topics{subTopic};
		auto collection_ptr = std::make_shared<mqtt::string_collection>(topics);
		if (mq_ptr->subscribe(collection_ptr))
		{
			spdlog::info("外部mqtt服务订阅成功！");
		}
	}
	return 0;
}

/*
 * @brief 向指定主题发送消息
 * @param topic 主题
 * @param msg 发送的消息
 * @return 发送成功返回0，失败返回-1
 */
bool mqttBus::pushMsg(std::string topic, std::string msg)
{
	return mq_ptr->publish(topic, msg);
}

void mqttBus::msgCallBack(callBack _cb)
{
	cb1= _cb;
}
