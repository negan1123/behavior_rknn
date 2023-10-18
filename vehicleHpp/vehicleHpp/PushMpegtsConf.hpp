/*
 * PushMpegtsConf.hpp
 *
 *  Created on: 2022年11月29日
 *      Author: wbf
 */

#ifndef PUSHMPEGTSCONF_HPP_
#define PUSHMPEGTSCONF_HPP_

#include <GlobalConf.hpp>
/**@class HplocatorConf
 * @brief 视频推流工程配置文件
 * */
class PushMpegtsConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	 * @param	path	json配置文件路径
	 * @return	HplocatorConf*		HplocatorConf单例
	 */
	static PushMpegtsConf *Instance(std::string path = std::string())
	{
		static PushMpegtsConf *_Instance = nullptr;
		if (nullptr == _Instance)
		{
			_Instance = new PushMpegtsConf(path);
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
			ifile.close();

			return loadConfig(jInfo);
		}
		catch (...)
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
			json video;

			getJsonInfo(jInfo, "server", server);
			getJsonInfo(jInfo, "video", video);

			// mq信令服务器ip地址和端口
			getJsonInfo(server, "host", PUSH_HOST);
			getJsonInfo(server, "port", PUSH_PORT);
			// 推流参数
			getJsonInfo(video, "stopTimeout", STOP_TIMEOUT);
			getJsonInfo(video, "maxstreams", MAX_STREAMS);
		}
		catch (...)
		{
			std::cerr << "解析 pushmpegts.json 出错" << std::endl;
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
		// 如果没有设置保存路径，直接返回
		if (cfgPath.empty())
			return;
		json jInfo;
		if (0 == configToJson(jInfo))
		{
			std::ofstream ofile((const char *)cfgPath.c_str());
			ofile << jInfo;
			ofile.close();
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
			json video;

			server["host"] = PUSH_HOST;
			server["port"] = PUSH_PORT;

			video["stopTimeout"] = STOP_TIMEOUT;
			video["maxstreams"] = MAX_STREAMS;

			jInfo["server"] = server;
			jInfo["video"] = video;
			
			return 0;
		}
		catch (...)
		{
			std::cerr << "保存 pushmpegts.json 出错" << std::endl;
			return -1;
		}
	}

	/**
	 * @brief 获取推流地址
	 * @param[in] null
	 * @return 推流地址
	 */
	std::string getPushHost() const { return PUSH_HOST; }

	/**
	 * @brief 获取推流端口
	 * @param[in] null
	 * @return 推流端口
	 */
	int getPushPort() const { return PUSH_PORT; }

	/**
	 * @brief 获取心跳检测时间
	 * @param[in] null
	 * @return 心跳检测时间
	 */
	int getStopTimeout() const { return STOP_TIMEOUT; }

	/**
	 * @brief 获取最大播放数量
	 * @param[in] null
	 * @return 最大播放数量
	 */
	int getMaxStreams() const { return MAX_STREAMS; }

	/**
	 * @brief 设置推理地址
	 * @param	pushhost	推理地址
	 * @return	None
	 */
	void setPushHost(const std::string &pushhost) { PUSH_HOST = pushhost; }

	/**
	 * @brief 设置推流端口号
	 * @param	pushport	推流端口号
	 * @return	None
	 */
	void setPushPort(const int &pushport) { PUSH_PORT = pushport; }

	/**
	 * @brief 设置心跳时间
	 * @param	stoptime	心跳时间
	 * @return	None
	 */
	void setStopTimeout(const int &stoptime) { STOP_TIMEOUT = stoptime; }

	/**
	 * @brief 设置最大推流数量
	 * @param	max	最大推流数量
	 * @return	None
	 */
	void setMaxStreams(const int &max) { MAX_STREAMS = max; }

private:
	/**@brief 构造函数
	 * @param[in]		path	json配置文件路径
	 * @return	None
	 */
	PushMpegtsConf(const std::string &path) { cfgPath = path; }

public:
	std::string PUSH_HOST; ///< 推流地址
	int PUSH_PORT;		   ///< 推流端口

	int STOP_TIMEOUT; ///< 心跳检测时间
	int MAX_STREAMS;  ///< 最大推流数量

private:
	std::string cfgPath; ///< 配置文件路径
};

#endif /* PushMpegtsConf_HPP_ */
