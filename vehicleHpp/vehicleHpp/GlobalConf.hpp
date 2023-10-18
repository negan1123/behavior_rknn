/*
 * GlobalConf.hpp
 *
 *  Created on: 2021年4月14日
 *      Author: syukousen
 */

#ifndef GLOBALCONF_HPP_
#define GLOBALCONF_HPP_

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

/**
 * @brief 获取wifi密码
 */
static std::string getWpaPw()
{
	std::string wpaPw;
	std::string searchName = "wpa_passphrase=";
	std::string cmd = "cat /etc/hostapd/hostapd.conf |grep " + searchName + "|tr -d \"\\r\\n\"";
	int len = -1;
	char buf[512];
	FILE * fp = NULL;
	fp =  popen(cmd.c_str(), "r");
	if(fp)
	{
		if (fgets(buf, 512, fp) != NULL)
		{
			len = strlen(buf);
		}
	}
	pclose(fp);

	if(len != -1)
		wpaPw = std::string(buf+searchName.size());

	return wpaPw;
}

class GlobalConf
{
public:
	/**@brief 加载json配置文件到内存
	* @param	path	全局配置的路径
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int loadConfig(const std::string &path)
	{
		std::string configFile = "global.json";
		std::string::size_type found = path.rfind('/');
		if(found != std::string::npos)
		{
			configFile = path.substr(0, found+1) + "global.json";
		}
		std::ifstream ifile((const char *)configFile.c_str());
		if (!ifile.is_open())
		{
			std::cerr << "未能成功加载 " << configFile << std::endl;
			return -1;
		}
		cfgPath = configFile;
		json jInfo;
		try
		{
			ifile >> jInfo;
			return loadConfig(jInfo);
		}
		catch(...)
		{
			std::cerr << "parse global.json file err" << std::endl;
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
			json log;
			json mqtt;
			getJsonInfo(jInfo, "log", log);
			getJsonInfo(jInfo, "mqtt", mqtt);
			// 获取设备相关信息
			getJsonInfo(jInfo, "deviceId", DEVICE_DEVID);
			getJsonInfo(jInfo, "imei", IMEI);
			getJsonInfo(jInfo, "imsi", IMSI);
			getJsonInfo(jInfo, "ccid", CCID);
			getJsonInfo(jInfo, "mac", MAC);
			getJsonInfo(jInfo, "license", License);
			getJsonInfo(jInfo, "wpa_passphrase", wpa_passphrase);
			if(wpa_passphrase.empty())
				wpa_passphrase = getWpaPw();
			// 获取日志相关信息
			getJsonInfo(log, "logFile", LOG_FILE);
			getJsonInfo(log, "logLevel", LOG_LEVEL);
			getJsonInfo(log, "logFormat", LOG_FORMAT);
			getJsonInfo(log, "logDay", LOG_DAY);
			// 获取mqtt相关信息
			getJsonInfo(mqtt, "ip", MQTT_HOST);
			getJsonInfo(mqtt, "port", MQTT_PORT);
			getJsonInfo(mqtt, "user", MQTT_USER);
			getJsonInfo(mqtt, "passWord", MQTT_PWD);

		}
		catch(...)
		{
			std::cerr << "parse global.json file err" << std::endl;
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
			//保存wifi密码
			std::string cmd = "sed -i 's/wpa_passphrase=.*/wpa_passphrase="+ wpa_passphrase +"/g' /etc/hostapd/hostapd.conf";
			system(cmd.c_str());
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
			json log;
			json mqtt;

			jInfo["deviceId"] = DEVICE_DEVID;
			jInfo["imei"] = IMEI;
			jInfo["imsi"] = IMSI;
			jInfo["ccid"] = CCID;
			jInfo["mac"] = MAC;
			jInfo["license"] = License;

			log["logFile"] = LOG_FILE;
			log["logLevel"] = LOG_LEVEL;
			log["logFormat"] = LOG_FORMAT;
			log["logDay"] = LOG_DAY;

			mqtt["ip"] = MQTT_HOST;
			mqtt["port"] = MQTT_PORT;
			mqtt["user"] = MQTT_USER;
			mqtt["passWord"] = MQTT_PWD;

			jInfo["log"] = log;
			jInfo["mqtt"] = mqtt;
			return 0;
		}
		catch(...)
		{
			std::cerr << "save global.json file err" << std::endl;
			return -1;
		}
	}

	/**@brief 获取设备编号
	* @param	None
	* @return	std::string  设备编号
	*/
	std::string getDeviceDevId() const {return DEVICE_DEVID;}

	/**@brief 获取设备IMEI
	* @param	None
	* @return	std::string  设备IMEI
	*/
	std::string getImei() const {return IMEI;}

	/**@brief 获取设备IMSI
	* @param	None
	* @return	std::string  设备IMSI
	*/
	std::string getImsi() const {return IMSI;}

	/**@brief 获取SIM卡的ccid
	* @param	None
	* @return	std::string  SIM卡的ccid
	*/
	std::string getCCID() const {return CCID;}

	/**@brief 获取设备MAC
	* @param	None
	* @return	std::string  设备MAC
	*/
	std::string getMac() const {return MAC;}

	/**@brief 获取日志文件路径
	* @param	None
	* @return	std::string  日志文件路径
	*/
	std::string getLogFile() const {return LOG_FILE;}

	/**@brief 获取日志级别
	* @param	None
	* @return	int  日志级别
	*/
	int getLogLevel() const {return LOG_LEVEL;}

	/**@brief 获取日志输出格式
	* @param	None
	* @return	std::string  日志输出格式
	*/
	std::string getLogFormat() const {return LOG_FORMAT;}

	/**@brief 获取管理中心服务器地址
	* @param	None
	* @return	std::string		管理中心服务器地址
	*/
	std::string getMqttHost(){return MQTT_HOST;}

	/**@brief 获取服务器端口
	* @param	None
	* @return	int		服务器端口
	*/
	int getMqttPort(){return MQTT_PORT;}

	/**@brief 获取管理中心用户名
	* @param	None
	* @return	std::string		用户名
	*/
	std::string  getMqttUserName(){return MQTT_USER;}

	/**@brief 获取管理中心密码
	* @param	None
	* @return	std::string		密码
	*/
	std::string  getMqttPassword(){return MQTT_PWD;}

	/**@brief 设置设备编号
	* @param	id			设备编号
	* @return	Nones
	*/
	void setDeviceDevId(const std::string &id){DEVICE_DEVID = id;}

	/**@brief 设置设备IMEI
	* @param	imei	设备IMEI
	* @return	None
	*/
	void setImei(const std::string &imei){IMEI = imei;}

	/**@brief 设置设备IMSI
	* @param	imsi	设备IMSI
	* @return	None
	*/
	void setImsi(const std::string &imsi){IMSI = imsi;}

	/**@brief 设置SIM卡的ICCID
	* @param	ccid	SIM卡的ICCID
	* @return	None
	*/
	void setCCID(const std::string &ccid){CCID = ccid;}

	/**@brief 设置设备MAC
	* @param	mac	 	设备MAC
	* @return	None
	*/
	void setMac(const std::string &mac){MAC = mac;}

	/**@brief 设置日志文件路径
	* @param	logfile	日志文件路径
	* @return	None
	*/
	void setLogFile(const std::string &logfile){LOG_FILE = logfile;}

	/**@brief 设置日志级别
	* @param	level	日志级别
	* @return	None
	*/
	void setLogLevel(int level){LOG_LEVEL = level;}

	/**@brief 设置日志输出格式
	* @param	format	日志输出格式
	* @return	None
	*/
	void setLogFormat(const std::string &format){LOG_FORMAT = format;}

	/**@brief 设置管理中心服务器地址
	* @param	host	管理中心服务器地址
	* @return	None
	*/
	void setMqttHost(const std::string &host){MQTT_HOST = host;}

	/**@brief 设置服务器端口
	* @param	port	服务器端口
	* @return	None
	*/
	void setMqttPort(int port){MQTT_PORT = port;}

	/**@brief 设置管理中心用户名
	* @param	user		用户名
	* @return	None
	*/
	void setMqttUserName(const std::string &user){MQTT_USER = user;}

	/**@brief 设置管理中心密码
	* @param	pwd			密码
	* @return	None
	*/
	void setMqttPassword(const std::string &pwd){MQTT_PWD = pwd;}

	/**
	 * @brief 获取授权码
	 * @return 返回授权码
	 */
	const std::string& getLicense() const {
		return License;
	}

	/*
	 * @brief 获取日志保留天数
	 * return 返回日志保留天数
	 */
	int getLogDay() const {
		return LOG_DAY;
	}

	const std::string& getWpaPassphrase() const {
		return wpa_passphrase;
	}

	template<typename ValueT>
	void getJsonInfo(json &jInfo, std::string key, ValueT &value)
	{
		if(jInfo.contains(key))
			value = jInfo.at(key);
	}
protected:
	std::string 	DEVICE_DEVID;		///< 设备编号
	std::string		IMEI;				///< imei
	std::string		IMSI;				///< imsi
	std::string 	CCID;				///< SIM的ICCID
	std::string		MAC;				///< mac
	std::string		License;			///< 授权码
	std::string		LOG_FILE;			///< 日志文件路径
	int				LOG_LEVEL;			///< 日志级别
	std::string		LOG_FORMAT;			///< 日志输出格式
	int 			LOG_DAY;			///< 日志保留天数
	std::string 	MQTT_HOST;			///< mqtt管理中心服务器地址
	int 			MQTT_PORT;			///< mqtt服务器端口
	std::string 	MQTT_USER;			///< mqtt管理中心用户名
	std::string 	MQTT_PWD;			///< mqtt管理中心密码
	std::string wpa_passphrase;			///< wifi密码
private:
	std::string 	cfgPath;			///< 配置文件路径
};

#endif /* GLOBALCONF_HPP_ */
