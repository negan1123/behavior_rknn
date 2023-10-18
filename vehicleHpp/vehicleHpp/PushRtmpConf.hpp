/*
 * PushRtmpConf.hpp
 *
 *  Created on: 2021年4月14日
 *      Author: syukousen
 */

#ifndef PUSHRTMPCONF_HPP_
#define PUSHRTMPCONF_HPP_
#include <VStreamRecorderConf.hpp>

/**@class PushRtmpConf
 * @brief 视频采集设置文件管理类，主要加载json配置的视频采集的参数
 * */
class PushRtmpConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	PushRtmpConf*		PushRtmpConf单例
	*/
	static PushRtmpConf * Instance(std::string path = std::string())
	{
		static PushRtmpConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new PushRtmpConf(path);
		}
		return _Instance;
	}

	/**@brief 加载json配置文件到内存, 目前工程主要依赖VStreamRecorder.json的数据，自己的配置目前还没有。
	* @param	None
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int loadConfig()
	{
		int ret = -1;
		std::string configFile = "VStreamRecorder.json";

		// 先加载全局的配置文件
		GlobalConf::loadConfig(cfgPath);

		std::string::size_type found = cfgPath.rfind('/');
		if(found != std::string::npos)
		{
			configFile = cfgPath.substr(0, found+1) + "VStreamRecorder.json";
		}
		ret = VStreamRecorderConf::Instance(configFile)->loadConfig();
		if(-1 == ret)
			return ret;
		else
		{
			port = VStreamRecorderConf::Instance()->getPort();
			recordEnable = VStreamRecorderConf::Instance()->isRecordEnable();
			subject = VStreamRecorderConf::Instance()->getSubject();
			auth = VStreamRecorderConf::Instance()->isAuth();
			userName = VStreamRecorderConf::Instance()->getUserName();
			password = VStreamRecorderConf::Instance()->getPassword();
			cameraList = VStreamRecorderConf::Instance()->getCameraList();
			sdAutoDetect = VStreamRecorderConf::Instance()->isSdAutoDetect();
			videoPath = VStreamRecorderConf::Instance()->getvideoPath();

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
				std::cerr << "parse PushStream.json file err" << std::endl;
				return -1;
			}
			return 0;
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
			json mqtt;

			getJsonInfo(jInfo, "mqtt", mqtt);
			getJsonInfo(mqtt, "reportTopic", reportTopic);
			getJsonInfo(mqtt, "subTopic", subTopic);
			getJsonInfo(jInfo, "dstIp", dstIp);
			getJsonInfo(jInfo, "dstPort", dstPort);
			getJsonInfo(jInfo, "nginxPath", nginxPath);
		}
		catch(...)
		{
			std::cerr << "parse PushStream.json file err" << std::endl;
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
			json mqtt;
			mqtt["reportTopic"] = reportTopic;
			mqtt["subTopic"] = subTopic;
			jInfo["mqtt"] = mqtt;
			jInfo["dstIp"] = dstIp;
			jInfo["dstPort"] = dstPort;
			jInfo["nginxPath"] = nginxPath;

			return 0;
		}
		catch(...)
		{
			std::cerr << "save PushStream.json file err" << std::endl;
			return -1;
		}
	}

	/**@brief 获取rtsp的端口
	* @param	None
	* @return	unsigned short		rtsp的端口
	*/
	unsigned short getPort(){return port;}

	/**@brief 是否保存录像
	* @param	None
	* @return	bool		是否保存录像
	*/
	bool isRecordEnable(){return recordEnable;}

	/**@brief 获取RTSP服务器的子目录
	* @param	None
	* @return	std::string		子目录
	*/
	const std::string &getSubject() const {return subject;}

	/**@brief RTSP服务器是否支持加密
	* @param	None
	* @return	bool		是否支持加密
	*/
	bool isAuth(){return auth;}

	/**@brief 获取RTSP服务器用户名
	* @param	None
	* @return	std::string		用户名
	*/
	const std::string & getUserName() const {return userName;}

	/**@brief 获取RTSP服务器密码
	* @param	None
	* @return	std::string		密码
	*/
	const std::string &getPassword() const {return password;}

	/**@brief 获取摄像头列表
	* @param	None
	* @return	std::vector<CameraInfo>	摄像头列表
	*/
	std::vector<CameraInfo> & getCameraList(){return cameraList;}

	/**@brief 获取接收检索结果的主题
	* @param	None
	* @return	std::string		接收检索结果的主题
	*/
	const std::string &getReportTopic() const {return reportTopic;}

	/**@brief 获取推流的主题，录像机订阅，接收检索请求的主题
	* @param	None
	* @return	std::string		推流的主题
	*/
	const std::string &getSubTopic() const {return subTopic;}

	/**@brief 设置接收检索结果的主题
	* @param	report		接收检索结果的主题
	* @return	None
	*/
	void setReportTopic(const std::string & report){reportTopic = report;}

	/**@brief 设置推流的主题，录像机订阅，接收检索请求的主题
	* @param	sub			推流的主题
	* @return	None
	*/
	void setSubTopic(const std::string & sub){subTopic = sub;}

	/**@brief 是否开启sd卡自动检测
	* @param	None
	* @return	bool		是否开启sd卡自动检测
	*/
	bool isSdAutoDetect() const {return sdAutoDetect;}

	/**@brief 获取视频保存路径
	* @param	None
	* @return	std::string  视频保存路径
	*/
	const std::string & getvideoPath() const {return videoPath;}

	const std::string& getDstIp() const {
		return dstIp;
	}

	int getDstPort() const {
		return dstPort;
	}

	const std::string& getNginxPath() const {
		return nginxPath;
	}

private:
	/**@brief 构造函数
	* @param[in]		path	json配置文件路径
	* @return	None
	*/
	PushRtmpConf(const std::string & path){cfgPath = path;}

private:
	unsigned short	port;					///< rtsp端口号，是否是http端口取决于rtpoverhttp设置
	bool			recordEnable;			///< 是否开启视频录像功能
	std::string		subject;				///< rtsp主题，rtsp的url：rtsp://设备ip:port/主题/摄像机通道号
	bool			auth;					///< 访问rtsp的时候是否要进行用户名密码认证
	std::string		userName;				///< 访问认证的用户名
	std::string		password;				///< 访问认证的密码
	std::vector<CameraInfo>	cameraList;		///< 摄像头队列
	std::string reportTopic;				///< 接收检索结果的主题
	std::string subTopic;					///< 推流的主题，录像机订阅，接收检索请求的主题
	bool			sdAutoDetect;			///< 是否开启sd卡检测功能，开启则视频文件放在sd卡中，不检测则放在videoPath中
	std::string 	videoPath;				///< 视频保存路径
	std::string 	cfgPath;				///< 配置文件路径
	std::string dstIp;						///< 推流目标ip
	int dstPort;							///< 推流目标端口
	std::string nginxPath;					///< nginx地址
};


#endif /* PUSHRTMPCONF_HPP_ */
