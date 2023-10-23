#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <DetectorService.h>
#include <RtspCapture.h>
#include <versionConfig.h>

#include "DetectorService.h"
#include "VAITerminal_JTConf.hpp"

using namespace std;


/**
 * @brief 获取当前执行文件的绝对路径和名称
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

/**
 * @brief 日志初始化
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

int main()
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
    DetectorService detector;

    detector.start();


	// 摄像头获取图片
	unique_ptr<Capture> capture{nullptr};
    capture = make_unique<RtspCapture>(confjson->getVideoDev(),
									   confjson->getVideoWidth(),
									   confjson->getVideoHeight());
	capture->init();
	capture->startCapture([&](cv::Mat& detectimg){
		// cv::Mat test = cv::imread("../img/smoke2.jpg");
		detector.enqueueMat(detectimg);
	});

	// detector.test_start();	// 测试用

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
		sprintf(msg[1].buf, "%s", detector.getRknnApiV().data());
		sprintf(msg[2].buf, "%s", detector.getRknnDrvV().data());
		while(1)
		{
			msgsnd(msgid, &msg[0], sizeof(msg[0])-sizeof(long), IPC_NOWAIT);
			msgsnd(msgid, &msg[1], sizeof(msg[1])-sizeof(long), IPC_NOWAIT);
			msgsnd(msgid, &msg[2], sizeof(msg[2])-sizeof(long), IPC_NOWAIT);
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	});
	sendVersionThread.detach();

	while (1)
	{
		usleep(60*1000*1000);
	}
    return 0;


    // RockxDetector rockx;
    // rockx.initFaceDetector();
    // cv::Mat res = cv::imread("../img/test.jpg");
    // rockx_image_t input = rockx.input("../img/test.jpg");
    // rockx.faceDetect(input);

    // faceAngle angle;
    // vector<landmark> landmarks;
    // faceBox box;
    // rockx.getResults(angle, landmarks, box);

    // int count = 0;
    // for (auto it = landmarks.begin(); it != landmarks.end(); it++)
    // {
    //     // 定义点的位置
    //     cv::Point land(it->x, it->y);

    //     // 绘制一个红色的点
    //     cv::circle(res, land, 2, cv::Scalar(0, 0, 255), -1);
    //     cv::putText(res, to_string(count), land, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255),1, 8, false);
    //     count++;
    // }
    // cv::imwrite("../img/res.jpg",res);

    // // rockx.destoryDetector();
    return 0;
}