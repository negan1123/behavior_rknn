/*
 * VAITerminal_JTConf.hpp
 *
 *  Created on: 2021年7月20日
 *      Author: syukousen
 */

#ifndef VAITERMINAL_JTCONF_HPP_
#define VAITERMINAL_JTCONF_HPP_

#include <GlobalConf.hpp>
/**@class HplocatorConf
 * @brief 高精度定位工程配置文件
 * */
class VAITerminal_JTConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	HplocatorConf*		HplocatorConf单例
	*/
	static VAITerminal_JTConf * Instance(std::string path = std::string())
	{
		static VAITerminal_JTConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new VAITerminal_JTConf(path);
		}
		return _Instance;
	}

	/**@brief 加载json配置文件到内存
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
		// 先加载全局的配置文件
		GlobalConf::loadConfig(cfgPath);
		json jInfo;
		try
		{
			ifile >> jInfo;
			return loadConfig(jInfo);
		}
		catch(...)
		{
			std::cerr << "parse VAITerminal_JTConf.json file err" << std::endl;
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
			json server;
			json data;
			json jt808;

			getJsonInfo(jInfo, "server", server);
			getJsonInfo(jInfo, "data", data);
			getJsonInfo(jInfo, "jt808", jt808);

			///< 808信令服务器ip地址和端口
			getJsonInfo(server, "host", SERVER_HOST);
			getJsonInfo(server, "port", SERVER_PORT);

			getJsonInfo(data, "interval", UP_DATA_INTERVAL);

			///< 以下是 808注册信息，是否已经注册，注册成功会得到鉴权码authentication
			getJsonInfo(jt808, "phone", phone);							///<  设备对应的手机号码(由于部标机需要上一张手机卡，通过手机卡可以接收短消息，现在用物联网卡，这个参数通常只是一个标识而已)
			getJsonInfo(jt808, "manufacturer", manufacturer);		///< 制造商
			getJsonInfo(jt808, "terminalModel", terminalModel);	///< 终端型号
			getJsonInfo(jt808, "carPlateColor", carPlateColor);	///< 车辆颜色
			getJsonInfo(jt808, "carPlateNum", carPlateNum);		///< 车牌号码

			///< 是否注册成功：true表示已经注册
			getJsonInfo(jt808, "isRegistered", isRegistered);
			///< 注册成功得到得鉴权码
			getJsonInfo(jt808, "authentication", authentication);
		}
		catch(...)
		{
			std::cerr << "parse VAITerminal_JTConf.json file err" << std::endl;
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
			json server;
			json data;
			json jt808;

			server["host"] = SERVER_HOST;
			server["port"] = SERVER_PORT;

			data["interval"] = UP_DATA_INTERVAL;

			jt808["phone"] = phone;
			jt808["manufacturer"] = manufacturer;
			jt808["terminalModel"] = terminalModel;
			jt808["carPlateColor"] = carPlateColor;
			jt808["carPlateNum"] = carPlateNum;
			jt808["isRegistered"] = isRegistered;
			jt808["authentication"] = authentication;

			jInfo["server"] = server;
			jInfo["data"] = data;
			jInfo["jt808"] = jt808;
			return 0;
		}
		catch(...)
		{
			std::cerr << "save VAITerminal_JTConf.json file err" << std::endl;
			return -1;
		}
	}

private:
	/**@brief 构造函数
	* @param[in]		path	json配置文件路径
	* @return	None
	*/
	VAITerminal_JTConf(const std::string & path){cfgPath = path;}

public:
	std::string SERVER_HOST;				///< 服务器地址
	int SERVER_PORT;							///< 服务器端口
	int UP_DATA_INTERVAL;				///< 上传定位数据间隔
	std::string		phone;							///< 手机号
	std::string		manufacturer;				///< 制造商ID
	std::string		terminalModel;			///< 产品型号
	int				carPlateColor;				///< 车牌颜色
	std::string		carPlateNum;				///< 车牌号
	bool			isRegistered;					///< JT808是否已经注册
	std::string		authentication;			///< 鉴权码
private:
	std::string 	cfgPath;							///< 配置文件路径
};

#endif /* VAITERMINAL_JTCONF_HPP_ */
