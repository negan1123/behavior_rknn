/*
 * thermalRunaway.hpp
 *
 *  Created on: Jun 21, 2022
 *      Author: lyf
 */

#ifndef INCLUDE_CanProxyConf_HPP_
#define INCLUDE_CanProxyConf_HPP_

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
using json = nlohmann::json;

//热失控配置
class CanProxyConf
{
public:
	const std::string& getCfgPath() const {
		return cfgPath;
	}

	const std::string& getLocationTopic() const {
		return locationTopic;
	}

	int getReportTime() const {
		return reportTime;
	}

	const std::string& getReportTopic() const {
		return reportTopic;
	}

	const std::string& getVersion() const {
		return version;
	}

public:
	/**
	* @brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	ConfigJson*		ConfigJson单例
	*/
	static CanProxyConf * Instance(std::string path = std::string())
	{
		static CanProxyConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new CanProxyConf(path);
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
			std::cerr << "parse CanProxyConf.json file err" << std::endl;
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
			if(jInfo.contains("canName"))
				canName = jInfo["canName"];
			else
				canName = "can0";
			if(jInfo.contains("bitrate"))
				bitrate = jInfo["bitrate"];
			else
				bitrate = 250000;
			if(jInfo.contains("locationTopic"))
				locationTopic = jInfo["locationTopic"];
			else
				locationTopic = "location_report";

			if(jInfo.contains("reportTopic"))
				reportTopic = jInfo["reportTopic"];

			if(jInfo.contains("reportTime"))
				reportTime = jInfo["reportTime"];
			else
				reportTime = 3;

			if(jInfo.contains("filterEnable"))
				filterEnable = jInfo["filterEnable"];

			if(jInfo.contains("version"))
				version = jInfo["version"];
			else
				version = "1.0";

			if(jInfo.contains("canIDList"))
			{
				int canIDListSize = jInfo["canIDList"].size();
				if(canIDListSize > 0)
					canIDList = jInfo["canIDList"].get<std::vector<std::string>>();
			}

			if(jInfo.contains("mqtt"))
			{
				json mqtt;
				mqtt = jInfo["mqtt"];
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
			std::cout<<"CanProxyConf json error"<<std::endl;
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
			jInfo["canName"] = canName;
			jInfo["bitrate"] = bitrate;
			jInfo["locationTopic"] = locationTopic;
			jInfo["reportTopic"] = reportTopic;
			jInfo["reportTime"] = reportTime;
			jInfo["filterEnable"] = filterEnable;

			if(canIDList.size() > 0)
			{
				jInfo["canIDList"] = canIDList;
			}

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

	const std::string& getCanName() const {
		return canName;
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

	const std::vector<std::string>& getCanIdList() const {
		return canIDList;
	}

	int getBitrate() const {
		return bitrate;
	}

	bool isFilterEnable() const {
		return filterEnable;
	}

private:
	/**
	 * @brief 带参构造函数
	 * @param path 配置文件路径
	 */
	CanProxyConf(const std::string & path)
	{
		cfgPath = path;
	}

private:
	std::string cfgPath;				///< 配置文件路径
	std::string locationTopic;			///< 位置信息主题
	std::string reportTopic;			///< 热失控上报主题
	int reportTime;						///< 热失控推送频率
//	std::string simNum;					///< 终端sim卡号
	std::string version;				///< 协议版本
	std::vector<std::string>canIDList;  ///< can消息白名单
	std::string canName;				///< can总线
	std::string mqttHost;				///< mqtt地址
	int mqttPort;						///< mqtt端口
	std::string userName;				///< mqtt用戶名
	std::string password;				///< nqtt密碼
	int bitrate;						///< can比特率
	bool filterEnable;					///< 是否启用can消息白名单
};

#endif /* INCLUDE_CanProxyConf_HPP_ */
