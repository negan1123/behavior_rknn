/*
 * BleProxy.hpp
 *
 *  Created on: Jul 7, 2022
 *      Author: lyf
 */

#ifndef BLEPROXYCONF_HPP_
#define BLEPROXYCONF_HPP_

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class BleProxyConf
{
public:
	/**
	* @brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	ConfigJson*		ConfigJson单例
	*/
	static BleProxyConf * Instance(std::string path = std::string())
	{
		static BleProxyConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new BleProxyConf(path);
		}
		return _Instance;
	}

	/**
	* @brief 加载json配置文件到内存
	* @param	None
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int loadConfig()
	{
		std::ifstream ifile((const char *)cfgPath.c_str());
		if (!ifile.is_open())
		{
			std::cerr << "未能成功加载 " << cfgPath << std::endl;
			return -1;
		}

		json jInfo;
		try
		{
			ifile >> jInfo;
			return loadConfig(jInfo);
		}
		catch(...)
		{
			std::cerr << "parse BleProxyConf.json file err" << std::endl;
			return -1;
		}
		return 0;
	}

	/**@brief 加载json配置到内存
	* @param	jInfo	json字段
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int loadConfig(json &jInfo)
	{
		try
		{
			if(jInfo.contains("locationTopic"))
				locationTopic = jInfo["locationTopic"];
			if(jInfo.contains("bleTopic"))
				bleTopic = jInfo["bleTopic"];
			if(jInfo.contains("bleDevice"))
				bleDevice = jInfo["bleDevice"];
			if(jInfo.contains("baudrate"))
				baudrate = jInfo["baudrate"];
			if(jInfo.contains("mqtt"))
			{
				json mqtt = jInfo["mqtt"];
				if(mqtt.contains("mqttHost"))
					mqttHost = mqtt["mqttHost"];
				if(mqtt.contains("mqttPort"))
					mqttPort = mqtt["mqttPort"];
				if(mqtt.contains("userName"))
					userName = mqtt["userName"];
				if(mqtt.contains("password"))
					password = mqtt["password"];
			}
		}
		catch(...)
		{
			std::cout<<"BleProxyConf json error"<<std::endl;
			return -1;
		}

		return 0;
	}

	/**@brief 将内存里面的配置加载保存到文件中
	* @param	None
	* @return	None
	*/
	void saveConfig()
	{
		//如果没有设置保存路径，直接返回
		if(cfgPath.empty())
			return;

		json jInfo;
		if(0 == configToJson(jInfo))
		{
			std::ofstream ofile((const char *)cfgPath.c_str());
			ofile << jInfo;
		}
	}

	/**@brief 将内存里面的配置加载json中
	* @param	jInfo		加载后的json
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int configToJson(json &jInfo)
	{
	try
	{
		jInfo["locationTopic"] = locationTopic;
		jInfo["bleTopic"] = bleTopic;
		jInfo["bleDevice"] = bleDevice;
		jInfo["baudrate"] = baudrate;

		json mqtt;
		mqtt["mqttHost"] = mqttHost;
		mqtt["mqttPort"] = mqttPort;
		mqtt["userName"] = userName;
		mqtt["password"] = password;
		jInfo["mqtt"] = mqtt;
	}
	catch(...)
	{
		std::cout<<"将内存里面的配置加载json中 error"<<std::endl;
		return -1;
	}

	return 0;
	}

	int getBaudrate() const {
		return baudrate;
	}

	const std::string& getBleDevice() const {
		return bleDevice;
	}

	const std::string& getBleTopic() const {
		return bleTopic;
	}

	const std::string& getCfgPath() const {
		return cfgPath;
	}

	const std::string& getLocationTopic() const {
		return locationTopic;
	}

	const std::string& getMqttHost() const {
		return mqttHost;
	}

	int getMqttPort() const {
		return mqttPort;
	}

	const std::string& getPassword() const {
		return password;
	}

	const std::string& getUserName() const {
		return userName;
	}

private:
	/**
	 * @brief 带参构造函数
	 * @param path 配置文件路径
	 */
	BleProxyConf(const std::string & path)
	{
		cfgPath = path;
	}

private:
	std::string cfgPath;				///< 配置文件路径
	std::string mqttHost;				///< mqtt地址
	int mqttPort;						///< mqtt端口
	std::string userName;				///< mqtt用戶名
	std::string password;				///< nqtt密碼
	std::string locationTopic;			///< 上报位置主题
	std::string bleTopic;				///< 上报蓝牙信息主题
	std::string bleDevice;				///< 蓝牙串口
	int baudrate;						///< 串口波特率
};

#endif /* BLEPROXYCONF_HPP_ */
