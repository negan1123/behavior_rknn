/*
 * VStreamRecorderConf.hpp
 *
 *  Created on: 2021年4月14日
 *      Author: syukousen
 */

#ifndef VSTREAMRECORDERCONF_HPP_
#define VSTREAMRECORDERCONF_HPP_

#include <GlobalConf.hpp>
typedef struct _CameraInfo_
{
	std::string 	id;							///< 摄像机编号,主码流对应通道号101,子码流102
	unsigned int 	type;						///< 摄像机类型，1表示ipc, 2表示uvc, 3表示mipi摄像机
	struct _uDevice_
	{
		std::string	device;					///< uvc和mini对应的设备，ipc没有这个key
		bool		subStream;				///< 摄像头的时候是否支持子码流
		bool		showTitle;				///< 是否显示摄像头名
		std::string title;					///< 摄像头名
		int			titleFont;				///< 字体大小
		std::string urlmain;				///< ipc对应的rtsp地址主码流地址
		std::string urlsub;					///< ipc对应的rtsp地址子码流地址
	}uDevice;
	unsigned int 	frameRate;					///< 摄像机的帧率，仅对UVC和MIPI有效
	unsigned int 	bitRate;					///< 摄像机的码率，仅对UVC和MIPI有效
	unsigned int 	iGap;						///< 摄像机的I帧间隔，仅对UVC和MIPI有效
	unsigned int 	subBitRate;					///< 摄像机的子码率，仅对UVC和MIPI有效
	unsigned int 	subIGap;					///< 摄像机的子I帧间隔，仅对UVC和MIPI有效
	bool			recordEnable;				///< 这一路摄像机是否开启视频录像功能，默认表示不录像
	unsigned int	recordDays;					///< 录像片段保存天数，超过这个天数就删除
	bool			cameraEnable;				///< 这一路camera是否启用
	struct _size_								///< uvc和mipi视频的宽度和高度,主码流和子码流,ipc的主码流和子码流以摄像机为准,不做变换
	{
		unsigned int mainWidth;
		unsigned int mainHeight;
		unsigned int subWidth;
		unsigned int subHeight;
	}size;
}CameraInfo;

typedef struct _MqttInfo_
{
	bool enable;							///<	是否支持MQ录像文件检索
	std::string ip;							///<	MQTT服务器地址
	int			port;						///<	MQTT服务器端口
	std::string user;						///<	MQTT登录的用户名
	std::string passWord;					///<	MQTT登录的密码
	std::string reportTopic;				///<	接收检索结果的主题
	std::string recorderTopic;				///<	录像机的主题，录像机订阅，接收检索请求的主题
}MqttInfo;

/**@brief 重载json的from_json，让其能获取CameraInfo结构体信息
* @param[in]	j		json对象
* @param[out]	t	从json获取的camerainfo信息
* @return	None
*/
inline void from_json(const json&j, CameraInfo &t)
{
	j.at("id").get_to(t.id);
	j.at("type").get_to(t.type);
	// type是1的时候代表IPC, 这时候需要代理rtsp服务器地址
	if(t.type == 1)
	{
		j.at("urlmain").get_to(t.uDevice.urlmain);
		if(j.contains("urlsub"))
			j.at("urlsub").get_to(t.uDevice.urlsub);
		else
			t.uDevice.urlsub = std::string();
	}
	else
	{
		j.at("device").get_to(t.uDevice.device);
		if(j.contains("subStream"))
			j.at("subStream").get_to(t.uDevice.subStream);
		else
			t.uDevice.subStream = false;

		j.at("showTitle").get_to(t.uDevice.showTitle);
		if(t.uDevice.showTitle)
		{
			j.at("title").get_to(t.uDevice.title);
		}
		j.at("titleFont").get_to(t.uDevice.titleFont);

		j.at("mainWidth").get_to(t.size.mainWidth);
		j.at("mainHeight").get_to(t.size.mainHeight);
		if(t.uDevice.subStream)
		{
			j.at("subWidth").get_to(t.size.subWidth);
			j.at("subHeight").get_to(t.size.subHeight);
			j.at("subIGap").get_to(t.subIGap);
			j.at("subBitRate").get_to(t.subBitRate);
		}
		j.at("frameRate").get_to(t.frameRate);
		j.at("bitRate").get_to(t.bitRate);
		j.at("iGap").get_to(t.iGap);
	}

	j.at("recordEnable").get_to(t.recordEnable);
	// 这一路摄像头是否启用
	if(j.contains("enable"))
		j.at("enable").get_to(t.cameraEnable);
	else
		t.cameraEnable = true;
}

/**@brief 重载json的to_json，让其将CameraInfo结构体信息写入json
* @param[in]	j		json对象
* @param[out]	t	从json获取的camerainfo信息
* @return	None
*/
inline void to_json(json&j, const CameraInfo &t)
{
	j["id"] = t.id;
	j["type"] = t.type;
	// type是1的时候代表IPC, 这时候需要代理rtsp服务器地址
	if(t.type == 1)
	{
		j["urlmain"] = t.uDevice.urlmain;
		if(!t.uDevice.urlsub.empty())
			j["urlsub"] = t.uDevice.urlsub;
	}
	else
	{
		j["device"] = t.uDevice.device;
		j["subStream"] = t.uDevice.subStream;
		j["showTitle"] = t.uDevice.showTitle;
		j["title"] = t.uDevice.title;
		j["titleFont"] = t.uDevice.titleFont;

		j["mainWidth"] = t.size.mainWidth;
		j["mainHeight"] = t.size.mainHeight;

		if(t.uDevice.subStream)
		{
			j["subWidth"] = t.size.subWidth;
			j["subHeight"] = t.size.subHeight;
			j["subIGap"] = t.subIGap;
			j["subBitRate"] = t.subBitRate;
		}
		j["frameRate"] = t.frameRate;
		j["bitRate"] = t.bitRate;
		j["iGap"] = t.iGap;
	}

	j["recordEnable"] = t.recordEnable;
	j["enable"] = t.cameraEnable;
}

/**@class VStreamRecorderConf
 * @brief 视频采集设置文件管理类，主要加载json配置的视频采集的参数，以提供给rtsp使用
 * */
class VStreamRecorderConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	ConfigJson*		ConfigJson单例
	*/
	static VStreamRecorderConf * Instance(std::string path = std::string())
	{
		static VStreamRecorderConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new VStreamRecorderConf(path);
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
			std::cerr << "parse VStreamRecorder.json file err" << std::endl;
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
			json mqtt;

			getJsonInfo(jInfo, "port", port);
			getJsonInfo(jInfo, "rtspoverhttp", rtspoverhttp);
			getJsonInfo(jInfo, "rtpovertcp", rtpovertcp);
			getJsonInfo(jInfo, "recordEnable", recordEnable);
			getJsonInfo(jInfo, "sdAutoDetect", sdAutoDetect);
			getJsonInfo(jInfo, "duration", duration);
			getJsonInfo(jInfo, "recordDays", recordDays);
			getJsonInfo(jInfo, "storageSize", storageSize);
			getJsonInfo(jInfo, "subject", subject);
			getJsonInfo(jInfo, "auth", auth);
			getJsonInfo(jInfo, "userName", userName);
			getJsonInfo(jInfo, "password", password);

			if(jInfo.contains("videoPath"))
				videoPath = jInfo.at("videoPath");
			else
				videoPath = std::string("../video");

			if(jInfo.contains("videoTmpPath"))
				videoTmpPath = jInfo.at("videoTmpPath");
			else
				videoTmpPath = std::string("/tmp");

			// cameras数据由from_json加载
			if(jInfo.contains("cameras"))
			{
				int tilength = jInfo["cameras"].size();
				if(tilength > 0)
				{
					cameraList = jInfo["cameras"].get<std::vector<CameraInfo>>();
				}
			}

			getJsonInfo(jInfo, "mqtt", mqtt);
			getJsonInfo(mqtt, "enable", mMqttInfo.enable);
			mMqttInfo.ip = MQTT_HOST;
			mMqttInfo.port = MQTT_PORT;
			mMqttInfo.user = MQTT_USER;
			mMqttInfo.passWord = MQTT_PWD;
			getJsonInfo(mqtt, "reportTopic", mMqttInfo.reportTopic);
			getJsonInfo(mqtt, "recorderTopic", mMqttInfo.recorderTopic);
		}
		catch(...)
		{
			std::cerr << "parse VStreamRecorder.json file err" << std::endl;
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
			json cameras;

			jInfo["port"] = port;
			jInfo["rtspoverhttp"] = rtspoverhttp;
			jInfo["rtpovertcp"] = rtpovertcp;
			jInfo["recordEnable"] = recordEnable;
			jInfo["sdAutoDetect"] = sdAutoDetect;
			jInfo["duration"] = duration;
			jInfo["recordDays"] = recordDays;
			jInfo["storageSize"] = storageSize;
			jInfo["subject"] = subject;
			jInfo["auth"] = auth;
			jInfo["userName"] = userName;
			jInfo["password"] = password;
			jInfo["videoPath"] = videoPath;
			jInfo["videoTmpPath"] = videoTmpPath;

			mqtt["enable"] = mMqttInfo.enable;
			mqtt["reportTopic"] = mMqttInfo.reportTopic;
			mqtt["recorderTopic"] = mMqttInfo.recorderTopic;

			jInfo["mqtt"] = mqtt;

			if(cameraList.size() > 0)
			{
				jInfo["cameras"] = cameraList;
			}
		}
		catch(...)
		{
			std::cerr << "save VStreamRecorder.json file err" << std::endl;
			return -1;
		}
		return 0;
	}

	/**@brief 获取rtsp的端口
	* @param	None
	* @return	unsigned short		rtsp的端口
	*/
	unsigned short getPort() const {return port;}

	/**@brief rtsp是否在http端口上透传
	* @param	None
	* @return	bool		是否在http端口上透传
	*/
	bool isRtspOverHttp() const {return rtspoverhttp;}

	/**@brief rtp是否通过tcp传输
	* @param	None
	* @return	bool		rtp是否通过tcp传输
	*/
	bool isRtpOverTcp() const {return rtpovertcp;}

	/**@brief 是否保存录像
	* @param	None
	* @return	bool		是否保存录像
	*/
	bool isRecordEnable() const {return recordEnable;}

	/**@brief 是否开启sd卡自动检测
	* @param	None
	* @return	bool		是否开启sd卡自动检测
	*/
	bool isSdAutoDetect() const {return sdAutoDetect;}

	/**@brief 获取保存录像的时间间隔(分钟)
	* @param	None
	* @return	unsigned int		保存录像的时间间隔
	*/
	unsigned int getDuration() const {return duration;}

	/**@brief 获取保存录像片段保存天数
	* @param	None
	* @return	unsigned int		保存录像的时间天数
	*/
	unsigned int getRecordDays() const {return recordDays;}

	/**@brief 获取保存录像片段最大容量
	* @param	None
	* @return	unsigned int		保存录像片段最大容量
	*/
	unsigned int getStorageSize() const {return storageSize;}

	/**@brief 获取RTSP服务器的子目录
	* @param	None
	* @return	std::string		子目录
	*/
	const std::string & getSubject() const {return subject;}

	/**@brief RTSP服务器是否支持加密
	* @param	None
	* @return	bool		是否支持加密
	*/
	bool isAuth() const {return auth;}

	/**@brief 获取RTSP服务器用户名
	* @param	None
	* @return	std::string		用户名
	*/
	const std::string & getUserName() const {return userName;}

	/**@brief 获取RTSP服务器密码
	* @param	None
	* @return	std::string		密码
	*/
	const std::string & getPassword() const {return password;}

	/**@brief 获取摄像头列表
	* @param	None
	* @return	std::vector<CameraInfo>	摄像头列表
	*/
	const std::vector<CameraInfo> & getCameraList() const {return cameraList;}

	/**@brief 获取视频保存路径
	* @param	None
	* @return	std::string  视频保存路径
	*/
	const std::string & getvideoPath() const {return videoPath;}

	/**@brief 获取视频保存临时路径
	* @param	None
	* @return	std::string  视频保存路径
	*/
	const std::string & getvideoTmpPath() const {return videoTmpPath;}

	/**@brief 获取mqtt相关信息
	* @param	None
	* @return	MqttInfo  mqtt相关信息
	*/
	const MqttInfo &getMqttInfo() const {return mMqttInfo;}

	/**@brief 设置rtsp的端口
	* @param	p			rtsp的端口
	* @return	None
	*/
	void setPort(unsigned short p){port = p;}

	/**@brief 设置rtsp是否在http端口上透传
	* @param	b			是否在http端口上透传
	* @return	None
	*/
	void setRtspOverHttp(bool b){rtspoverhttp = b;}

	/**@brief 设置rtp是否通过tcp传输
	* @param	b			rtp是否通过tcp传输
	* @return	None
	*/
	void setRtpOverTcp(bool b){rtpovertcp = b;}

	/**@brief 设置是否保存录像
	* @param	b		是否保存录像
	* @return	None
	*/
	void setRecordEnable(bool b){recordEnable = b;}

	/**@brief 设置是否开启sd卡自动检测
	* @param	b		是否开启sd卡自动检测
	* @return	None
	*/
	void setSdAutoDetect(bool b){sdAutoDetect = b;}

	/**@brief 设置保存录像的时间间隔(分钟)
	* @param	d			保存录像的时间间隔
	* @return	None
	*/
	void setDuration(unsigned int d) {duration =d;}

	/**@brief 设置保存录像片段保存天数
	* @param	d			保存录像的时间天数
	* @return	None
	*/
	void setRecordDays(unsigned int d){recordDays = d;}

	/**@brief 设置保存录像片段最大容量
	* @param	s			保存录像片段最大容量
	* @return	None
	*/
	void setStorageSize(unsigned int s){storageSize =s;}

	/**@brief 设置RTSP服务器的子目录
	* @param	sub			子目录
	* @return	None
	*/
	void setSubject(const std::string &sub){subject = sub;}

	/**@brief 设置RTSP服务器是否支持加密
	* @param	b			是否支持加密
	* @return	None
	*/
	void setAuth(bool b){auth = b;}

	/**@brief 设置RTSP服务器用户名
	* @param	user			用户名
	* @return	None
	*/
	void setUserName(const std::string & user){userName = user;}

	/**@brief 设置RTSP服务器密码
	* @param	pwd			密码
	* @return	None
	*/
	void setPassword(const std::string & pwd){password = pwd;}

	/**@brief 设置摄像头列表
	* @param	list		摄像头列表
	* @return	None
	*/
	void setCameraList(const std::vector<CameraInfo> & list){cameraList = list;}

	/**@brief 设置视频保存路径
	* @param	path		视频保存路径
	* @return	None
	*/
	void setvideoPath(const std::string & path){videoPath = path;}

	/**@brief 设置视频保存临时路径
	* @param	path		视频保存临时路径
	* @return	None
	*/
	void setvideoTmpPath(const std::string & path){videoTmpPath = path;}

	/**@brief 设置mqtt相关信息
	* @param	info		mqtt相关信息
	* @return	None
	*/
	void setMqttInfo(const MqttInfo & info){mMqttInfo.enable = info.enable; mMqttInfo.recorderTopic = info.recorderTopic; mMqttInfo.reportTopic = info.reportTopic;}

private:
	/**@brief 构造函数
	* @param[in]		path	json配置文件路径
	* @return	None
	*/
	VStreamRecorderConf(const std::string & path){cfgPath = path;}

private:
	unsigned short	port;				///< rtsp端口号，是否是http端口取决于rtpoverhttp设置
	bool			rtspoverhttp;		///< 如果rtpoverhttp=true，则port表示http的端口，rtsp客户端可以通过rtpoverhttp方式请求
	bool			rtpovertcp;			///< 是否开启rtpovertcp，如果开启这个功能，推送rtp的时候，对方也得基于同样的方式接收
	bool			recordEnable;		///< 是否开启视频录像功能
	bool			sdAutoDetect;		///< 是否开启sd卡检测功能，开启则视频文件放在sd卡中，不检测则放在videoPath中
	unsigned int	duration;			///< 录像片段时长,每个录像片段存储成一个.264文件，单位：分钟
	unsigned int	recordDays;			///< 录像片段保存天数，超过这个天数就删除
	unsigned int	storageSize;		///< 录像记录占用的最大空间，超过这个数值按照时间顺序删除前面的录像
	std::string		subject;			///< rtsp主题，rtsp的url：rtsp://设备ip:port/主题/摄像机通道号
	bool			auth;				///< 访问rtsp的时候是否要进行用户名密码认证
	std::string		userName;			///< 访问认证的用户名
	std::string		password;			///< 访问认证的密码
	std::vector<CameraInfo>	cameraList;	///< 摄像头队列
	std::string 	videoPath;			///< 视频保存路径
	std::string 	videoTmpPath;		///< 视频保存临时路径
	std::string 	cfgPath;			///< 配置文件路径
	MqttInfo		mMqttInfo;			///< mqtt相关配置
};

#endif /* VSTREAMRECORDERCONF_HPP_ */
