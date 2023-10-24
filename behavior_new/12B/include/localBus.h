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

class LocalBus
{
public:

	LocalBus();

	~LocalBus();

	bool init();

	void publish(const int &behaviorInt);

//	void response(const int &ret);

	void setOnRecieveMsg(onRecieveMsg _cb);
private:
	mq::MQTT_Manage* mq_ptr{nullptr};
	onRecieveMsg cb{nullptr};
	std::string sessionID;
};



#endif /* INCLUDE_LOCALBUS_H_ */
