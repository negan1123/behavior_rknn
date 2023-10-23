/*
 * UploadBehavior.h
 *
 *  Created on: 2021年1月8日
 *      Author: syukousen
 */

#ifndef _UPLOADBEHAVIOR_H_
#define _UPLOADBEHAVIOR_H_

#include <queue>
#include <mutex>
#include <stdio.h>
#include <memory.h>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>

#include <RtspVideoCircleRecorder.h>
#include <BehaviorConf.hpp>
#include <VAITerminal_JTConf.hpp>
#include <util.h>

/**@class UploadBehavior
 * @brief 上传驾驶员行为识别产生的记录图片和视频
 * */
class UploadBehavior
{
public:
	/**@brief 构造函数。
	* @param[in]		server  	上传服务器ip
	* @param[in]		port  		上传服务器端口
	* @param[in]		savePath  	记录图片和视频的目录
	* @return	None
	*/
	UploadBehavior(std::string server, int port, std::string savePath);

	/**@brief 析构函数，释放申请的资源
	* @param	None
	* @return	None
	*/
	~UploadBehavior();

	/**@brief 传入一张产生的记录图片，加入发送队列
	* @param[in]		behaviorpath  记录图片路径
	* @return	None
	*/
	void queueBehavior(std::string behaviorpath);

	/**@brief 开始记录图片上传循环
	* @param	None
	* @return	None
	*/
	void start();
private:
	/**@brief 根据记录图片，生成记录发生时前后时间段的视频
	* @param	None
	* @return	None
	*/
	void pic2Video();

	/**@brief 向服务器发送记录图片
	* @param	None
	* @return	None
	*/
	void sendBehavior();

	/**@brief 向服务器发送记录视频
	* @param	None
	* @return	None
	*/
	void sendBehaviorVideo();

	/**@brief 打包记录图片并发送
	* @param[in]	time				记录时间
	* @param[in]	behavior			驾驶行为类型
	* @param[in]	pictype				记录图片类型
	* @param[in]	picdata				记录图片数据
	* @param[in]	picdatalen			记录图片数据长度
	* @return	int		发送成功与否
	*/
	int packetDataAndSend(time_t time, int behavior, char pictype, char *picdata, int picdatalen);

	/**@brief 打包记录图像并发送
	* @param[in]	time				记录时间
	* @param[in]	behavior			驾驶行为类型
	* @param[in]	picdata				记录图像数据
	* @param[in]	picdatalen			记录图像数据长度
	* @return	int		发送成功与否
	*/
	int packetVideoDataAndSend(time_t time, int behavior, char *picdata, int picdatalen);

	/**@brief 开机时将上次没有发送完的记录图片重新加载到发送队列
	* @param	None
	* @return	None
	*/
	void initLastData();
	std::string server;			///< 记录服务器ip
	int port;					///< 记录服务器端口
	int mSaveVideo;				///< 是否保存记录录像
	std::string IMEI;			///< 上网卡IMEI
	std::string IMSI;			///< 上网卡IMSI
	std::string VER;			///< 程序版本号
	std::string savePath;			///< 驾驶行为保存路径
	unsigned int MAC[6];			///< 上网卡MAC	需要用4字节的类型，sscanf针对的是4字节的指针
	std::queue<std::string> behaviorQueue;		///< 发送记录图片队列
	std::queue<std::string> behaviorPic2VideoQueue;	///< 将记录图片生存记录录像队列
	std::queue<std::string> behaviorVideoQueue;
	std::mutex mtx;				///< behaviorQueue队列锁
	std::mutex mtxVideo;		///< behaviorVideoQueue队列锁
	char *sendbuf{nullptr};		///< 发送buf，固定，不用每次都申请释放
};
#endif /* _UPLOADBEHAVIOR_H_ */
