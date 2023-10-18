/*
 * UVMonitorConf.hpp
 *
 *  Created on: Mar 1, 2022
 *      Author: lyf
 */

#ifndef UVMONITOR_INCLUDE_UVMONITORCONF_HPP_
#define UVMONITOR_INCLUDE_UVMONITORCONF_HPP_

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

/**
 * @brief 存储notify属性的结构体
 */
typedef struct _notifyInfo_
{
	std::string name;	///< 软件名
	bool enable;		///< 是否启动软件
	int softType;		///< 软件类型
	int overTime;		///< 等待超时
}notifyInfo;

/**
* @brief 重载json的from_json，让其能获取notifyInfo结构体信息
* @param[in]	j		json对象
* @param[out]	t	从json获取的notifyInfo信息
* @return	None
*/
inline void from_json(const json&j, notifyInfo &t)
{
	j.at("name").get_to(t.name);
	j.at("enable").get_to(t.enable);
	j.at("softType").get_to(t.softType);
	j.at("overTime").get_to(t.overTime);
}

/**
* @brief 重载json的to_json，让其将notifyInfo结构体信息写入json
* @param[in]	j		json对象
* @param[out]	t	从json获取的notifyInfo信息
* @return	None
*/
inline void to_json(json&j, const notifyInfo &t)
{
	j["name"] = t.name;
	j["enable"] = t.enable;
	j["softType"] = t.softType;
	j["overTime"] = t.overTime;
}

//供UVMonitor配置文件使用
class UVMonitorConf
{
public:
	/**
	* @brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	ConfigJson*		ConfigJson单例
	*/
	static UVMonitorConf * Instance(std::string path = std::string())
	{
		static UVMonitorConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new UVMonitorConf(path);
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
			std::cerr << "parse UVMonitor.json file err" << std::endl;
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
		try	{
			getJsonInfo<int>(jInfo, "temperature", temperature);
			getJsonInfo<bool>(jInfo, "uvEnalbe", uvEnalbe);
			getJsonInfo<int>(jInfo, "backupPowerGpio", backupPowerGpio);
			getJsonInfo<int>(jInfo, "uvDetect", uvDetect);

			json mqtt;
			if(jInfo.contains("mqtt")) {
				mqtt = jInfo["mqtt"];
				notifyTopic = mqtt["notifyTopic"];
				reportTopic = mqtt["reportTopic"];
			}

			json pm;
			if(jInfo.contains("powerControl")) {
				pm = jInfo["powerControl"];
				PM_DEVICE = pm["device"];
				PM_BAUDRATE = pm["baudrate"];
				PM_ENABLE = pm["pmEnable"];
				PM_DELAY = pm["pmDelay"];
				HB_TIMEOUT = pm["hbTimeout"];
				BOOST_DELAY = pm["boostDelay"];
			}

			if(jInfo.contains("notifyList"))
			{
				int notifyListSize = jInfo["notifyList"].size();
				if(notifyListSize > 0)
					notifyList = jInfo["notifyList"].get<std::vector<notifyInfo>>();
			}

			if(jInfo.contains("mountList"))
			{
				int mountListSize = jInfo["mountList"].size();
				if(mountListSize > 0)
					mountList = jInfo["mountList"].get<std::vector<std::string>>();
			}

		}
		catch(...)
		{
			std::cout<<"UVMonitor json error"<<std::endl;
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
			jInfo["temperature"] = temperature;
			jInfo["uvEnalbe"] = uvEnalbe;
			jInfo["backupPowerGpio"] = backupPowerGpio;
			jInfo["uvDetect"] = uvDetect;

			json mqtt;
			mqtt["notifyTopic"] = notifyTopic;
			mqtt["reportTopic"] = reportTopic;
			jInfo["mqtt"] = mqtt;

			json pm;
			pm["device"] = PM_DEVICE;
			pm["baudrate"] = PM_BAUDRATE;
			pm["pmEnable"] = PM_ENABLE;
			pm["pmDelay"] = PM_DELAY;
			pm["hbTimeout"] = HB_TIMEOUT;
			pm["boostDelay"] = BOOST_DELAY;
			jInfo["powerControl"] = pm;

			if(notifyList.size() > 0) {
				jInfo["notifyList"] = notifyList;
			}

			if(mountList.size() > 0) {
				jInfo["mountList"] = mountList;
			}

		}
		catch(...) {
			std::cout<<"将内存里面的配置加载json中 error"<<std::endl;
			return -1;
		}

		return 0;
	}

	/**
	 * @brief 返回挂载列表
	 * @return 返回挂载列表
	 */
	const std::vector<std::string>& getMountList() const {
			return mountList;
	}

	/**
	 * @brief 返回通知列表
	 * @return 返回通知列表
	 */
	const std::vector<notifyInfo>& getNotifyList() const {
		return notifyList;
	}

	/**
	 * @brief 返回配置文件路径
	 * @return 返回配置文件路径
	 */
	const std::string& getCfgPath() const {
		return cfgPath;
	}

	/**
	 * @brief 返回通知主题
	 * @return 返回通知主题
	 */
	const std::string& getNotifyTopic() const {
		return notifyTopic;
	}

	/**
	 * @brief 返回报告主题
	 * @return 返回报告主题
	 */
	const std::string& getReportTopic() const {
		return reportTopic;
	}

	/**
	 * @brief 根据softType返回软件名
	 * @return 返回软件名
	 */
	const std::string getSoftName(int softType) const {
		for(auto &iter : notifyList)
		{
			if(iter.softType == softType)
				return iter.name;
		}

		return "终端设备";
	}

	/**
	 * @brief 返回温度阈值
	 * @return 返回温度阈值
	 */
	int getTemperature() const {
		return temperature;
	}

	/**
	 * @brief 返回后备电源gpio编号
	 * @return 返回后备电源gpio编号
	 */
	int getBackupPowerGpio() const {
		return backupPowerGpio;
	}

	/**
	 * @brief 返回低电压检测gpio编号
	 * @return 返回低电压检测gpio编号
	 */
	int getUvDetect() const {
		return uvDetect;
	}

	/**
	 * @brief 返回低电压检测状态
	 * @return 返回低电压检测状态
	 */
	bool isUvEnalbe() const {
		return uvEnalbe;
	}

private:
	/**
	 * @brief 带参构造函数
	 * @param path 配置文件路径
	 */
	UVMonitorConf(const std::string & path)
	{
		cfgPath = path;
	}

	template<typename ValueT>
	void getJsonInfo(json &jInfo, std::string key, ValueT &value)
	{
		if(jInfo.contains(key))
			value = jInfo.at(key);
	}

private:
	std::string cfgPath;				///< 配置文件路径
	std::vector<notifyInfo>notifyList;	///< notify列表信息
	std::vector<std::string>mountList;	///< 挂载列表
	std::string notifyTopic;			///< 通知软件关机的主题
	std::string reportTopic;			///< 报告服务器的主题
	int temperature;					///< 温度阈值
	bool uvEnalbe;						///< 低电压检测状态
	int backupPowerGpio;				///< 后备电源gpio编号
	int uvDetect;						///< 低电压检测gpio编号

public:
	bool PM_ENABLE;						///< 是否启用电源管理功能
	std::string PM_DEVICE;				///< 电源管理串口设备名(串口名)
	int PM_BAUDRATE;					///< 电源管理串口波特率
	int HB_TIMEOUT;						///< 电源管理心跳超时时间,单位：分钟
	int PM_DELAY;						///< 电源管理欠压延时供电时间,单位：分钟
	int BOOST_DELAY;					///< 电源关断前升压延时供电时间,单位：分钟
};

#endif /* UVMONITOR_INCLUDE_UVMONITORCONF_HPP_ */
