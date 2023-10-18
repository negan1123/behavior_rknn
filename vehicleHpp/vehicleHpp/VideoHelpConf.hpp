/*
 * VideoHelpConf.hpp
 *
 *  Created on: Jul 7, 2022
 *      Author: lyf
 */

#ifndef INCLUDE_VIDEOHELPCONF_HPP_
#define INCLUDE_VIDEOHELPCONF_HPP_

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

//视频监控助手配置
class VideoHelpConf
{
	public:
	/**
	* @brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	ConfigJson*		ConfigJson单例
	*/
	static VideoHelpConf * Instance(std::string path = std::string())
	{
		static VideoHelpConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new VideoHelpConf(path);
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
			std::cerr << "parse videoHelp.json file err" << std::endl;
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
			if(jInfo.contains("requestTopic"))
				requestTopic = jInfo["requestTopic"];
			if(jInfo.contains("responseTopic"))
				responseTopic = jInfo["responseTopic"];

			if(jInfo.contains("ftp"))
			{
				json ftp = jInfo["ftp"];
				if(ftp.contains("userName"))
					ftpUserName = ftp["userName"];
				if(ftp.contains("password"))
					ftpPw = ftp["password"];
				if(ftp.contains("port"))
					ftpPort = ftp["port"];
				if(ftp.contains("localRoot"))
					localRoot = ftp["localRoot"];
			}
		}
		catch(...)
		{
			std::cout<<"videoHelp json error"<<std::endl;
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
			jInfo["requestTopic"] = requestTopic;
			jInfo["responseTopic"] = responseTopic;

			json ftp;
			ftp["userName"] = ftpUserName;
			ftp["password"] = ftpPw;
			ftp["port"] = ftpPort;
			ftp["localRoot"] = localRoot;
			jInfo["ftp"] = ftp;
		}
		catch(...)
		{
			std::cout<<"将内存里面的配置加载json中 error"<<std::endl;
			return -1;
		}

		return 0;
	}

	int getFtpPort() const {
		return ftpPort;
	}

	std::string getFtpPw() const {
		return ftpPw;
	}

	std::string getFtpUserName() const {
		return ftpUserName;
	}

	std::string getLocalRoot() const {
		return localRoot;
	}

	std::string getRequestTopic() const {
		return requestTopic;
	}

	std::string getResponseTopic() const {
		return responseTopic;
	}

private:
	/**
	 * @brief 带参构造函数
	 * @param path 配置文件路径
	 */
	VideoHelpConf(const std::string & path)
	{
		cfgPath = path;
	}

private:
	std::string cfgPath;				///< 配置文件路径
	std::string requestTopic;			///< 视频监控助手请求主题
	std::string responseTopic;			///< 视频监控助手请求响应主题
	std::string ftpUserName;			///< ftp账号
	std::string ftpPw;					///< ftp密码
	int ftpPort;						///< ftp监听端口
	std::string localRoot;				///< ftp文件根目录
};

#endif /* INCLUDE_VIDEOHELPCONF_HPP_ */
