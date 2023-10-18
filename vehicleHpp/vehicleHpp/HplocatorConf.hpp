/*
 * HplocatorConf.hpp
 *
 *  Created on: 2021年4月14日
 *      Author: syukousen
 */

#ifndef HPLOCATORCONF_HPP_
#define HPLOCATORCONF_HPP_

#include <GlobalConf.hpp>
/**@class HplocatorConf
 * @brief 高精度定位工程配置文件
 * */
class HplocatorConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	HplocatorConf*		HplocatorConf单例
	*/
	static HplocatorConf * Instance(std::string path = std::string())
	{
		static HplocatorConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new HplocatorConf(path);
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
			std::cerr << "parse hplocator.json file err" << std::endl;
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
			json rtk;
			json gps;
			json device;
			json data;
			json report;
			json nm0183;	// nm0183参数配置

			getJsonInfo(jInfo, "server", server);
			getJsonInfo(jInfo, "rtk", rtk);
			getJsonInfo(jInfo, "gps", gps);
			getJsonInfo(jInfo, "device", device);
			getJsonInfo(jInfo, "data", data);
			getJsonInfo(jInfo, "report", report);
			getJsonInfo(jInfo, "nm0183", nm0183);

			getJsonInfo(server, "host", SERVER_HOST);
			getJsonInfo(server, "port", SERVER_PORT);
			getJsonInfo(server, "mode", SERVER_MODE);

			getJsonInfo(rtk, "serverIp", CASTER_HOST);
			getJsonInfo(rtk, "serverPort", CASTER_PORT);
			getJsonInfo(rtk, "mountpoint", CASTER_MOUNT_POINT);
			getJsonInfo(rtk, "user", CASTER_USER);
			getJsonInfo(rtk, "password", CASTER_PWD);

			getJsonInfo(gps, "device", GPS_DEVICE);
			getJsonInfo(gps, "baudrate", GPS_BAUDRATE);
			getJsonInfo(gps, "dgps", GPS_DGPS);

//			getJsonInfo(device, "devType", DEVICE_DEVTYPE);
//			getJsonInfo(device, "netCardType", netCardType);
			getJsonInfo(device, "power12VEnable", power12VEnable);
			getJsonInfo(device, "gpioId", gpioId);

			getJsonInfo(data, "format", DATA_FORMAT);
			getJsonInfo(data, "interval", UP_DATA_INTERVAL);
			getJsonInfo(data, "CacheDuration", CacheDuration);

			getJsonInfo(report, "socket", SOCKET_TYPE);
			getJsonInfo(report, "JT808", JT808_TYPE);
			getJsonInfo(report, "file", FILE_TYPE);

			// 从配置文件中加载NM0183相关配置
			getJsonInfo(nm0183, "output", NM0183_OUTPUT);
			getJsonInfo(nm0183, "device", NM0183_DEVICE);
			getJsonInfo(nm0183, "baudrate", NM0183_BAUDRATE);
		}
		catch(...)
		{
			std::cerr << "parse hplocator.json file err" << std::endl;
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
			json rtk;
			json gps;
			json device;
			json data;
			json report;

			server["host"] = SERVER_HOST;
			server["port"] = SERVER_PORT;
			server["mode"] = SERVER_MODE;

			rtk["serverIp"] = CASTER_HOST;
			rtk["serverPort"] = CASTER_PORT;
			rtk["mountpoint"] = CASTER_MOUNT_POINT;
			rtk["user"] = CASTER_USER;
			rtk["password"] = CASTER_PWD;

			gps["device"] = GPS_DEVICE;
			gps["baudrate"] = GPS_BAUDRATE;
			gps["dgps"] = GPS_DGPS;

//			device["devType"] = DEVICE_DEVTYPE;
//			device["netCardType"] = netCardType;
			device["power12VEnable"] = power12VEnable;
			device["gpioId"] = gpioId;

			data["format"] = DATA_FORMAT;
			data["interval"] = UP_DATA_INTERVAL;
			data["CacheDuration"] = CacheDuration;

			report["socket"] = SOCKET_TYPE;
			report["JT808"] = JT808_TYPE;
			report["file"] = FILE_TYPE;

			jInfo["server"] = server;
			jInfo["rtk"] = rtk;
			jInfo["gps"] = gps;
			jInfo["device"] = device;
			jInfo["data"] = data;
			jInfo["report"] = report;
			return 0;
		}
		catch(...)
		{
			std::cerr << "save hplocator.json file err" << std::endl;
			return -1;
		}
	}

	/**@brief 获取服务器地址
	* @param	None
	* @return	std::string		服务器地址
	*/
	std::string getServerHost() const {return SERVER_HOST;}

	/**@brief 获取服务器端口
	* @param	None
	* @return	int		服务器端口
	*/
	int getServerPort()const {return SERVER_PORT;}

	/**@brief 获取上传服务器模式 0 udp  1 tcp
	* @param	None
	* @return	int		rtp是否通过tcp传输
	*/
	int getServerMode() const {return SERVER_MODE;}

	/**@brief 获取caster服务器地址
	* @param	None
	* @return	std::string		caster服务器地址
	*/
	std::string getCasterHost() const {return CASTER_HOST;}

	/**@brief 获取caster端口
	* @param	None
	* @return	int		caster端口
	*/
	int getCasterPort() const {return CASTER_PORT;}

	/**@brief 获取caster挂载点
	* @param	None
	* @return	std::string		caster挂载点
	*/
	std::string getCasterMountPoint() const {return CASTER_MOUNT_POINT;}

	/**@brief 获取caster用户名
	* @param	None
	* @return	std::string		caster用户名
	*/
	std::string getCasterUser() const {return CASTER_USER;}

	/**@brief 获取caster密码
	* @param	None
	* @return	std::string		caster密码
	*/
	std::string getCasterPwd() const {return CASTER_PWD;}

	/**@brief 获取定位模块设备名，及定位模块的串口名称
	* @param	None
	* @return	std::string		定位模块设备名
	*/
	std::string  getGpsDevice() const {return GPS_DEVICE;}

	/**@brief 获取定位模块串口波特率
	* @param	None
	* @return	int		串口波特率
	*/
	int  getGpsBaudrate() const {return GPS_BAUDRATE;}

	/**@brief 获取定位模式
	* @param	None
	* @return	string	A	自主定位
	* 					D	差分定位
	*/
	std::string  isGpsDGps() const {return GPS_DGPS;}

//	/**@brief 获取设备类型，千寻差分需要
//	* @param	None
//	* @return	std::string		设备类型
//	*/
//	std::string  getDeviceType() const {return DEVICE_DEVTYPE;}
//
//	/**@brief 获取4G上网卡类型
//	* @param	None
//	* @return	int		4G设备类型
//	*/
//	int  getNetCardType() const {return netCardType;}

	/**@brief 获取上传服务器的格式  bin,json
	* @param	None
	* @return	std::string		上传服务器的格式
	*/
	std::string  getDataFormat() const {return DATA_FORMAT;}

	/**@brief 获取上传服务器的间隔时间
	* @param	None
	* @return	int		上传服务器的间隔时间
	*/
	int  getUpDataInterval() const {return UP_DATA_INTERVAL;}

	/**@brief 获取定位数据断网时保存的时间
	* @param	None
	* @return	int		定位数据断网时保存的时间
	*/
	int  getCacheDuration() const {return CacheDuration;}

	/**@brief 获取是否按私有协议方式发送位置信息
	* @param	None
	* @return	bool	true代表是，false代表否
	*/
	bool getSocketType() const {return SOCKET_TYPE;}

	/**@brief 获取是否按JT808协议方式发送位置信息
	* @param	None
	* @return	bool	true代表是，false代表否
	*/
	bool getJT808Type() const {return JT808_TYPE;}

	/**@brief 获取是否将位置信息保存到本地文件中
	* @param	None
	* @return	bool	true代表是，false代表否
	*/
	bool getFileType() const {return FILE_TYPE;}

	/**@brief 设置服务器地址
	* @param	host	服务器地址
	* @return	None
	*/
	void setServerHost(const std::string& host){SERVER_HOST = host;}

	/**@brief 设置服务器端口
	* @param	port	服务器端口
	* @return	None
	*/
	void setServerPort(int port){SERVER_PORT = port;}

	/**@brief 设置上传服务器模式 1udp  2 tcp短链接  3 tcp长链接
	* @param	mode	rtp是否通过tcp传输
	* @return	None
	*/
	void setServerMode(int mode){SERVER_MODE = mode;}

	/**@brief 设置caster服务器地址
	* @param	host		caster服务器地址
	* @return	None
	*/
	void setCasterHost(const std::string& host) {CASTER_HOST = host;}

	/**@brief 设置caster端口
	* @param	port	caster端口
	* @return	None
	*/
	void setCasterPort(int port) {CASTER_PORT = port;}

	/**@brief 设置caster挂载点
	* @param	mountpoint		caster挂载点
	* @return	None
	*/
	void setCasterMountPoint(const std::string& mountpoint) {CASTER_MOUNT_POINT = mountpoint;}

	/**@brief 设置caster用户名
	* @param	user			caster用户名
	* @return	None
	*/
	void setCasterUser(const std::string & user) {CASTER_USER = user;}

	/**@brief 设置caster密码
	* @param	pwd				caster密码
	* @return	None
	*/
	void setCasterPwd(const std::string & pwd) {CASTER_PWD = pwd;}

	/**@brief 设置定位模块设备名，及定位模块的串口名称
	* @param	dev		定位模块设备名
	* @return	None
	*/
	void setGpsDevice(const std::string & dev){GPS_DEVICE = dev;}

	/**@brief 获取定位模块串口波特率
	* @param	rate	串口波特率
	* @return	None
	*/
	void setGpsBaudrate(int rate){GPS_BAUDRATE = rate;}

	/**@brief 设置模块是否差分定位
	* @param	dgps	是否差分定位
	* @return	None
	*/
	void setGpsDGps(const std::string &dgps){GPS_DGPS = dgps;}

//	/**@brief  设置设备类型
//	* @param	devtype		设备类型，千寻差分需要
//	* @return	None
//	*/
//	void setDeviceType(const std::string &devtype) {DEVICE_DEVTYPE = devtype;}

	/**@brief 设置上传服务器的格式  bin,json
	* @param	fmt	上传服务器的格式
	* @return	None
	*/
	void setDataFormat(const std::string &fmt){DATA_FORMAT = fmt;}

	/**@brief 设置上传服务器的间隔时间
	* @param	interval	上传服务器的间隔时间
	* @return	None
	*/
	void setUpDataInterval(int interval){UP_DATA_INTERVAL = interval;}

	/**@brief 设置设备id
	* @param	id			设备id
	* @return	None
	*/
	void setDeviceId(std::string id) {DEVICE_DEVID = id;}

	/**@brief 设置IMEI和IMSI
	* @param[in]	IMEI			imei
	* @param[in]	IMSI			imsi
	* @param[in]	MAC				mac
	* @return	None
	*/
	void saveGlobalImeiImsiMac(const std::string &IMEI, const std::string &IMSI,const std::string &MAC)
	{
		setImei(IMEI);
		setImsi(IMSI);
		setMac(MAC);
		GlobalConf::saveConfig();
	}

	/**@brief 设置是否按私有协议方式发送位置信息
	* @param	socket_type	true代表是，false代表否
	* @return	None
	*/
	void setSocketType(const bool &socket_type){SOCKET_TYPE = socket_type;}

	/**@brief 设置是否按JT808协议方式发送位置信息
	* @param	jt808_type	true代表是，false代表否
	* @return	None
	*/
	void setJT808Type(const bool &jt808_type){JT808_TYPE = jt808_type;}

	/**@brief 设置是否将位置信息保存到本地文件中
	* @param	file_type	true代表是，false代表否
	* @return	None
	*/
	void setFileType(const bool &file_type){FILE_TYPE = file_type;}

	/**
	 * @brief 获取控制输入电源状态
	 * @return bool 返回是否输入12v电源
	 */
	bool isPower12VEnable() const {return power12VEnable;}

	/**
	 * @brief 获取GPIO编号
	 * @return GPIO编号
	 */
	int getGpioId() const {return gpioId;}

private:
	/**@brief 构造函数
	* @param[in]		path	json配置文件路径
	* @return	None
	*/
	HplocatorConf(const std::string & path){cfgPath = path;}

private:
	std::string SERVER_HOST;				///< 服务器地址
	int SERVER_PORT;						///< 服务器端口
	int SERVER_MODE;						///< 终端连接服务器模式：0/1
	std::string CASTER_HOST;				///< caster服务器地址
	int CASTER_PORT;						///< caster端口
	std::string CASTER_MOUNT_POINT;			///< 挂载点
	std::string CASTER_USER;				///< 用户名
	std::string CASTER_PWD;					///< 密码
	std::string GPS_DEVICE;					///< GPS设备名(串口名)
	int GPS_BAUDRATE;						///< 波特率
	std::string GPS_DGPS;					///< 定位模式
//	std::string DEVICE_DEVTYPE;				///< 设备类型，千寻差分需要
	std::string DATA_FORMAT;				///< 上传定位数据格式
	int UP_DATA_INTERVAL;					///< 上传定位数据间隔
	int CacheDuration;						///< 定位数据断网时保存的时间
	std::string 	cfgPath;				///< 配置文件路径
//	int netCardType{0};						///< 4G上网卡类型
	bool SOCKET_TYPE;						///< 按私有协议方式发送位置信息
	bool JT808_TYPE;						///< 按JT808协议方式发送位置信息
	bool FILE_TYPE;							///< 将位置信息保存到本地文件中
	bool power12VEnable;					///< 是否提供12V电源
	int gpioId;								///< gpio编号

public:
	bool NM0183_OUTPUT;						///< 是否通过串口将定位信息输出
	std::string NM0183_DEVICE;				///< 输出NM0183报文串口设备名(串口名)
	int NM0183_BAUDRATE;					///< 输出NM0183报文串口波特率
};

#endif /* HPLOCATORCONF_HPP_ */
