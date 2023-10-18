/*
 * VehicleWebrtcClientConf.hpp
 *
 *  Created on: 2021年4月29日
 *      Author: syukousen
 */

#ifndef VEHICLEWEBRTCCLIENTCONF_HPP_
#define VEHICLEWEBRTCCLIENTCONF_HPP_

#include <GlobalConf.hpp>
typedef struct _StreamsInfo_
{
	int 	id;							///< 推流id
	std::string label;					///< 推流标签
	bool	enable;						///< 这路推流是否开启
	std::string url;					///< ipc对应的rtsp地址子码流地址
}StreamsInfo;


/**@brief 重载json的from_json，让其能获取StreamsInfo结构体信息
* @param[in]	j		json对象
* @param[out]	t	从json获取的StreamsInfo信息
* @return	None
*/
inline void from_json(const json&j, StreamsInfo &t)
{
	j.at("id").get_to(t.id);
	j.at("label").get_to(t.label);
	j.at("enable").get_to(t.enable);
	j.at("url").get_to(t.url);
}

/**@brief 重载json的to_json，让其将StreamsInfo结构体信息写入json
* @param[in]	j		json对象
* @param[out]	t	从json获取的StreamsInfo信息
* @return	None
*/
inline void to_json(json&j, const StreamsInfo &t)
{
	j["id"] = t.id;
	j["label"] = t.label;
	j["enable"] = t.enable;
	j["url"] = t.url;
}


/**@class VehicleWebrtcClientConf
 * @brief 视频采集设置文件管理类，主要加载json配置的视频采集的参数，以提供给rtsp使用
 * */
class VehicleWebrtcClientConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	VehicleWebrtcClientConf*		VehicleWebrtcClientConf单例
	*/
	static VehicleWebrtcClientConf * Instance(std::string path = std::string())
	{
		static VehicleWebrtcClientConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new VehicleWebrtcClientConf(path);
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
		int ret = -1;

		// 先加载全局的配置文件
		GlobalConf::loadConfig(cfgPath);

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
			std::cerr << "parse VehicleWebrtcClient.json file err" << std::endl;
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
			getJsonInfo(jInfo, "signalingServer", signalingServer);
			getJsonInfo(jInfo, "license", license);
			getJsonInfo(jInfo, "initString", initString);
			getJsonInfo(jInfo, "channels", channels);
			getJsonInfo(jInfo, "sessionLiveTime", sessionLiveTime);
			if(jInfo.contains("streams"))
			{
				int tilength = jInfo["streams"].size();
				if(tilength > 0)
				{
					StreamsList = jInfo["streams"].get<std::vector<StreamsInfo>>();
				}
			}
		}
		catch(...)
		{
			std::cerr << "parse VehicleWebrtcClient.json file err" << std::endl;
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
			jInfo["signalingServer"] = signalingServer;
			jInfo["license"] = license;
			jInfo["initString"] = initString;
			jInfo["channels"] = channels;
			jInfo["sessionLiveTime"] = sessionLiveTime;

			if(StreamsList.size() > 0)
			{
				jInfo["streams"] = StreamsList;
			}
		}
		catch(...)
		{
			std::cerr << "save VehicleWebrtcClient.json file err" << std::endl;
			return -1;
		}
		return 0;
	}

	/**@brief 获取webrtc推送的服务器
	* @param	None
	* @return	std::string		服务器地址
	*/
	const std::string &getSignalingServer() const {return signalingServer;}

	/**@brief 获取webrtc的license
	* @param	None
	* @return	std::string		license
	*/
	const std::string &getLicense() const {return license;}

	/**@brief 获取webrtc的initstring
	* @param	None
	* @return	std::string		initstring
	*/
	const std::string &getInitString() const {return initString;}

	/**@brief 获取推流列表
	* @param	None
	* @return	std::vector<StreamsInfo>	推流列表
	*/
	std::vector<StreamsInfo> & getStreamsList(){return StreamsList;}

	/**@brief 获取webrtc的channels
	* @param	None
	* @return	int		channels
	*/
	int getChannels() const {return channels;}

	/**@brief 获取webrtc的会话的存活时间
	* @param	None
	* @return	int		sessionLiveTime
	*/
	int getSessionLiveTime() const {return sessionLiveTime;}
private:
	/**@brief 构造函数
	* @param[in]		path	json配置文件路径
	* @return	None
	*/
	VehicleWebrtcClientConf(const std::string & path){cfgPath = path;}

private:
	std::vector<StreamsInfo>	StreamsList;		///< 推流列表
	std::string signalingServer;					///< webrtc推送的服务器
	std::string license;							///< webrtc的license
	std::string initString;							///< webrtc的initstring
	int			channels;							///< 一路推流同时允许多少个客户端
	int			sessionLiveTime;					///< 一个会话存活时间
	std::string 	cfgPath;				///< 配置文件路径
};
#endif /* VEHICLEWEBRTCCLIENTCONF_HPP_ */
