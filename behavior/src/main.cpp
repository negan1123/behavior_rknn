/**@mainpage  博能新一代智能终端驾驶行为识别
* <table>
* <tr><th>Project  <td>BSV100_behavior
* <tr><th>Author   <td>syukousen
* <tr><th>Source   <td>svn
* <tr><th>Copyright   <td>北京博能科技股份有限公司
* </table>
* @section   项目详细描述
* 本工程实现rtsp拉取本地NVR视频流，将视频流图像进行驾驶行为分析，得出非法的驾驶行为，保存记录图片和录像，上传服务器功能。
*
* @section   功能描述
* -# 本工程基于rk3399pro，ffmpeg拉取本地NVR提供的视频流，利用npu对视频流进行驾驶行为分析，加载了人像和头部检查，驾驶行为检查，欧拉角检查3个rknn模型。\n
			将分析后的驾驶行为保存照片和mp4录像，上传服务器。

* @section   固件更新
* <table>
* <tr><th>Date        <th>H_Version  <th>S_Version  <th>Author    <th>Description  </tr>
* <tr><td>2021/05/21  <td>1.0    <td>1.0.1   <td>syukousen  <td>创建初始版本 </tr>
* </table>
**********************************************************************************
*/


/*
 * main.cpp
 *
 *  Created on: 2020年12月22日
 *      Author: syukousen
 */
#include <iostream>
#include <sstream>
#include <memory>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "RetinaDriver.h"
#include "UploadBehavior.h"
#include <BehaviorConf.hpp>
#include <versionConfig.h>
#include <CameraCapture.h>
#include <RtspCapture.h>
#include "DriverBehaviorService.h"
#include <HwInfo.h>

/**@brief 获取当前时间，us
* @param		None
* @return	long long 当前的微秒数
*/
long long what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (long long)time.tv_sec*1000*1000 + time.tv_usec;
}

/**@brief 获取当前执行文件的绝对路径和名称
* @param[in]	processdir 返回的绝对路径
* @param[in]	processname 返回的应用程序名称
* @param[in]	len processdir的长度
* @return	size_t 绝对路径的长度
*/
size_t get_executable_path( char* processdir, char* processname, size_t len)
{
	char* path_end;
	if(readlink("/proc/self/exe", processdir, len) <=0)
		return -1;

	//截取最后一个“/”之前的字符串，这部分就是路径，截取后返回的是"/"的位置指针
	path_end = strrchr(processdir, '/');
	if(path_end == NULL)
		return -1;

	//获取到应用名称，处理完毕将path_end设置成0，保证路径字符串有结束位
	++path_end;
	strcpy(processname, path_end);
	*path_end = '\0';

	return (size_t)(path_end - processdir);
}

/**@brief 判断原始报文是否是以指定字符串开始
* @param[in]	str 完整的字符串
* @param[in]	s_str 开始的字符串
* @return	bool true 以指定字符串开始；false 不是以指定字符串开始
*/
bool startWith(const std::string& str, const std::string& s_str) {
	bool result{false};

	//str的长度必须大于等于s_str
	if(str.size() < s_str.size()) return result;

	size_t pos = str.find(s_str);
	if(pos == std::string::npos || pos != 0) {
		//没有找到字符串或者所在的位置不是0，表示不是以指定字符串开始
		result = false;
	}
	else {
		result = true;
	}

	return result;
}

/**@brief 日志初始化
* @param	None
* @return	bool 日志初始化结果
*/
bool log_init() {
	bool ret = false;
	std::string logfile = BehaviorConf::Instance()->getLogFile();
	logfile = logfile+"behavior.txt";
	try {
		//同时通过控制台和文件输出日志
		auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logfile, 0, 0);
		std::vector<spdlog::sink_ptr> sinks{stdout_sink, daily_sink};
		auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());

		//输出那个级别日志后刷新日志
		logger->flush_on((spdlog::level::level_enum)BehaviorConf::Instance()->getLogLevel());
		spdlog::set_default_logger(logger);

		//设置日志模式
		//spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
		spdlog::set_pattern(BehaviorConf::Instance()->getLogFormat());

		//设置日志输出级别
		spdlog::set_level((spdlog::level::level_enum)BehaviorConf::Instance()->getLogLevel());

		ret = true;
	}
	catch (const spdlog::spdlog_ex &ex)
	{
		std::cout << "日志初始化失败: " << ex.what() << std::endl;
	}

	return ret;
}

/**@brief 测试文件加下所有图片的行为分析
* @param[in]	dirppath 	图片存放路径
* @param[in]	detector 	驾驶行为分析对象
* @return	None
*/
void detectPictureInDir(std::string dirppath, RetinaDriver &detector)
{
	DIR *dir = nullptr;
	struct dirent *ptr = nullptr;
	struct stat statbuf;
	// 查找文件夹下的图片，批次处理
	dir = opendir(dirppath.c_str());
	while((ptr = readdir(dir)) != NULL)
	{
		lstat(ptr->d_name, &statbuf);

		spdlog::error("read dir name{}",ptr->d_name);
		// 过滤掉文件夹
		//if (!S_ISDIR(statbuf.st_mode))
		if(strcmp(ptr->d_name,".") != 0 && strcmp(ptr->d_name,"..") != 0)
		{
			int ret = 0;
			std::string picPath = dirppath + "/" + std::string(ptr->d_name);
			std::string picpersonPath = dirppath + "/person_" + std::string(ptr->d_name);
			std::string picheadPath = dirppath + "/head_" + std::string(ptr->d_name);
			cv::Mat detectimg;
			// 读取图片
			detectimg = cv::imread(picPath);
			// 输入图像开始检测
			detector.input(detectimg);
			ret = detector.inference();
			if(0 == ret)
			{
				cv::Mat personimg = detector.getPersonImage();
				if(!personimg.empty())
					cv::imwrite(picpersonPath,personimg);
				cv::Mat headimg = detector.getHeadImage();
				if(!headimg.empty())
					cv::imwrite(picheadPath,headimg);
				cv::imwrite(picPath,detector.getLabelledImage());
			}
		}
	}
	closedir(dir);
}

/**
 * @brief 验证授权码 验证不通过直接退出程序
 * @return 验证通过返回true，否则返回false
 */
bool verifyLicense()
{
	util::HwInfo hwinfo;
	hwinfo.init();
	std::string license = BehaviorConf::Instance()->getLicense();
	return hwinfo.verifyFinger(license);
}

/**@brief 程序主函数入口
* @param[in]	argc 	参数个数
* @param[in]	argv 	参数列表
* @return	int 程序返回结果
*/
int main(int argc, char *argv[])
{
	//设置信号回调函数，运行程序后键入ctrl + C回调,这里使用了lambda表达式定义回调函数
	signal(SIGINT, [](int signal){
		//由于捕获了SININT信号，不调用exit，程序将不会退出
		//exit的参数不为零都表示异常退出
		exit(-1);
	});

	//应用程序的根目录
	char app_path[PATH_MAX]{0};
	//获取可执行文件的工作目录
	char processname[1024]{0};
	get_executable_path(app_path, processname, sizeof(app_path));

	std::string conffile = std::string(app_path) +"/../conf/behavior.json";
	BehaviorConf *confjson = BehaviorConf::Instance(conffile);
	confjson->loadConfig();

	std::string conffile2 = std::string(app_path) +"/../conf/VAITerminal_JTConf.json";
	VAITerminal_JTConf *confjson2 = VAITerminal_JTConf::Instance(conffile2);
	confjson2->loadConfig();

	//判断日志对象是否初始化成功，初始化成功后可以通过spdlog::debug等来进行输出日志
	if(log_init()) {
		spdlog::debug("日志初始化成功");
	}

	//验证授权码 验证不通过直接退出程序
	if(!verifyLicense())
	{
		spdlog::error("验证授权码不通过");
		return EXIT_SUCCESS;
	}

	DriverBehaviorService detector;
	detector.start();
	//detector.testpic("./testpic");
	std::unique_ptr<Capture> capture{nullptr};
	if(0 == confjson->getVideoType())
		capture = std::make_unique<CameraCapture>(confjson->getVideoDev(),confjson->getVideoWidth(),confjson->getVideoHeight(),confjson->getVideoFmt(), confjson->getVideoInterval());
	else
		capture = std::make_unique<RtspCapture>(confjson->getVideoDev(),confjson->getVideoWidth(),confjson->getVideoHeight());
	capture->init();
	capture->startCapture([&](cv::Mat& detectimg){
		detector.enqueueMat(detectimg);
	});

	// 向miniserver发送自己的版本号
	std::thread sendVersionThread([&](){
		struct msgtype {
		    long mtype;
		    char buf[32];
		};
		static int msgid = -1;
		struct msgtype msg[3];
		if(msgid == -1)
		{
			msgid = msgget((key_t)1234, IPC_CREAT | 0666);
		}
		if(msgid == -1)
			return;
		msg[0].mtype = 1;
		msg[1].mtype = 8;
		msg[2].mtype = 9;
		sprintf(msg[0].buf, "%s", BEHAVIOR_VERSION);
		sprintf(msg[1].buf, "%s", detector.getRknnApiVer().data());
		sprintf(msg[2].buf, "%s", detector.getRknnDrvVer().data());
		while(1)
		{
			msgsnd(msgid, &msg[0], sizeof(msg[0])-sizeof(long), IPC_NOWAIT);
			msgsnd(msgid, &msg[1], sizeof(msg[1])-sizeof(long), IPC_NOWAIT);
			msgsnd(msgid, &msg[2], sizeof(msg[2])-sizeof(long), IPC_NOWAIT);
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	});
	sendVersionThread.detach();

	while(1)
	{
		usleep(60*1000*1000);
	}
	return 0;
}


