/*
 * localBus.h
 *
 *  Created on: Dec 14, 2021
 *      Author: wy
 */

#ifndef INCLUDE_LOCALBUS_H_
#define INCLUDE_LOCALBUS_H_

#include <functional>
#include <string>
#include <nlohmann/json.hpp>
#include "mqtt_manage.h"

using json = nlohmann::json;
using onRecieveMsg = std::function<void(const json &js)>;

/**@class	LocalBus
 * @brief	内部总线管理类
 */
class LocalBus
{
public:
	/**@brief	构造函数
	 * @param	None
	 * @return	None
	 */
	LocalBus();

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~LocalBus();

	/**@brief	初始化函数
	 * @details	初始化mqtt实例，订阅signal_down主题，设置回调函数，处理_VOICE事件
	 * @param	None
	 * @return	None
	 */
	bool init();

	/**@brief	处理响应函数
	 * @param	ret	处理结果：0 成功，1 失败
	 * @return	None
	 */
	void response(const int &ret);

	/**@brief	设置回调函数
	 * @param	_cb	回调函数
	 * @return	None
	 */
	void setOnRecieveMsg(onRecieveMsg _cb);
private:
	mq::MQTT_Manage* mq_ptr{nullptr};	///mqtt实例指针
	onRecieveMsg cb{nullptr};			///回调函数
	std::string sessionID;				///通信会话ID
};



#endif /* INCLUDE_LOCALBUS_H_ */
