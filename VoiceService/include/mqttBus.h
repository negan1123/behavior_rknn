

#ifndef MQTT_BUS_H_
#define MQTT_BUS_H_

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <util/log.h>
#include <mqtt_manage.h>
#include <SignalConf.hpp>
#include <VAITerminal_JTConf.hpp>
#include <VoiceService.hpp>


using namespace std;

using json = nlohmann::json;
using callBack = std::function<void(const json &js)>;

class mqttBus
{
public:
	/**@brief 构造函数
	* @param	None
	* @return	None
	*/
	mqttBus();

	/**@brief 析构函数
	* @param	None
	* @return	None
	*/
	~mqttBus(){}

	/**@brief 初始化函数
	* @param	None
	* @return	int			初始化成功与否
	*/
	int init();

	/*
	 * @brief 向指定主题发送消息
	 * @param topic 主题
	 * @param msg 发送的消息
	 * @return 发送成功返回0，失败返回-1
	 */
	bool pushMsg(std::string topic, std::string msg);

	void msgCallBack(callBack _cb);

private:
	mq::MQTT_Manage* mq_ptr{nullptr};				///<	mqtt服务指针
	callBack cb1{nullptr};
};
#endif /* MQTT_BUS_H_ */
