/*
 * main.cpp
 *
 *  Created on: Dec 14, 2021
 *      Author: wy
 */

#include <iostream>
#include <localBus.h>
#include <voiceDAO.h>
#include <voiceSynthesis.h>
#include <voicePlayer.h>
#include <volumeController.h>
#include <VoiceService.hpp>
#include <thread>
#include <HwInfo.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <sys/msg.h>
#include <signal.h>
#include <versionConfig.h>
#include <localVoice.h>

/**@brief	验证授权码 验证不通过直接退出程序
 * @param	None
 * @return	验证通过返回true，否则返回false
 */
bool verifyLicense()
{
	util::HwInfo hwinfo;
	hwinfo.init();
	std::string license = VoiceServiceConf::Instance()->getLicense();
	return hwinfo.verifyFinger(license);
}

/**@brief 获取当前执行文件的绝对路径和名称
 * @param[in]	processdir 返回的绝对路径
 * @param[in]	processname 返回的应用程序名称
 * @param[in]	len processdir的长度
 * @return	size_t 绝对路径的长度
 */
size_t get_executable_path(char *processdir, char *processname, size_t len)
{
	char *path_end;
	if (readlink("/proc/self/exe", processdir, len) <= 0)
		return -1;

	// 截取最后一个“/”之前的字符串，这部分就是路径，截取后返回的是"/"的位置指针
	path_end = strrchr(processdir, '/');
	if (path_end == NULL)
		return -1;

	// 获取到应用名称，处理完毕将path_end设置成0，保证路径字符串有结束位
	++path_end;
	strcpy(processname, path_end);
	*path_end = '\0';

	return (size_t)(path_end - processdir);
}

/**@brief 日志初始化
 * @param	None
 * @return	bool 日志初始化结果
 */
bool log_init()
{
	bool ret = false;
	std::string logfile = VoiceServiceConf::Instance()->getLogFile();
	logfile = logfile + "voiceService.txt";
	try
	{
		// 同时通过控制台和文件输出日志
		auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logfile, 0, 0);
		std::vector<spdlog::sink_ptr> sinks{stdout_sink, daily_sink};
		auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());

		// 输出那个级别日志后刷新日志
		logger->flush_on((spdlog::level::level_enum)VoiceServiceConf::Instance()->getLogLevel());
		spdlog::set_default_logger(logger);

		// 设置日志模式
		// spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
		spdlog::set_pattern(VoiceServiceConf::Instance()->getLogFormat());

		// 设置日志输出级别
		spdlog::set_level((spdlog::level::level_enum)VoiceServiceConf::Instance()->getLogLevel());

		ret = true;
	}
	catch (const spdlog::spdlog_ex &ex)
	{
		std::cout << "日志初始化失败: " << ex.what() << std::endl;
	}

	return ret;
}

// 声卡插拔初始化
int voiceCard_init()
{
	// 寻找声卡usb
	std::string cmd = "dmesg | grep 'USB PnP Sound Device' | sed '2,$d' |awk  '{printf $4}'";
	FILE *f = NULL;
	char cardUsb[1024] = {0};
	if (f = popen(cmd.c_str(), "r"))
	{
		if (fread(cardUsb, 1, 1024, f) > 0)
		{
		}
		pclose(f);
	}

	std::string usb(cardUsb);
	if (usb == "")
	{
		return 0;
	}
	usb.pop_back(); // 去除最后的冒号:
	std::cout << "usb:" << usb << std::endl;

	std::string unbind = "echo '" + usb + "' | tee /sys/bus/usb/drivers/usb/unbind";
	std::string bind = "echo '" + usb + "' | tee /sys/bus/usb/drivers/usb/bind";

	std::cout << "unbind cmd:" << unbind << std::endl;
	std::cout << "bind cmd:" << bind << std::endl;

	char buffer[1024] = {0};
	if (f = popen(unbind.c_str(), "r"))
	{
		if (fread(buffer, 1, 1024, f) > 0)
		{
		}
		pclose(f);
	}

	if (f = popen(bind.c_str(), "r"))
	{
		if (fread(buffer, 1, 1024, f) > 0)
		{
		}
		pclose(f);
	}

	return 0;
}

int main()
{
	// 设置信号回调函数，运行程序后键入ctrl + C回调,这里使用了lambda表达式定义回调函数
	signal(SIGINT, [](int signal)
		   {
		//由于捕获了SININT信号，不调用exit，程序将不会退出
		//exit的参数不为零都表示异常退出
		exit(-1); });

	// 应用程序的根目录
	char app_path[PATH_MAX]{0};
	// 获取可执行文件的工作目录
	char processname[1024]{0};
	get_executable_path(app_path, processname, sizeof(app_path));

	std::string conffile = std::string(app_path) + "/../conf/voiceService.json";
	VoiceServiceConf *confjson = VoiceServiceConf::Instance(conffile);
	confjson->loadConfig();

	std::string conffile1 = std::string(app_path) + "/../conf/signal.json";
	SignalConf *confjson1 = SignalConf::Instance(conffile1);
	confjson1->loadConfig();

	// 判断日志对象是否初始化成功，初始化成功后可以通过spdlog::debug等来进行输出日志
	if (log_init())
	{
		spdlog::debug("日志初始化成功");
	}

	// // 验证授权码 验证不通过直接退出程序
	// if (!verifyLicense())
	// {
	// 	spdlog::error("验证授权码不通过");
	// 	return EXIT_SUCCESS;
	// }

	// 向miniserver发送自己的版本号
	std::thread sendVersionThread([]()
								  {
		struct msgtype {
		    long mtype;
		    char buf[32];
		};
		static int msgid = -1;
		struct msgtype msg;
		if(msgid == -1)
		{
			msgid = msgget((key_t)1234, IPC_CREAT | 0666);
		}
		if(msgid == -1)
			return;

		msg.mtype = 10;
		sprintf(msg.buf, "%s", VOICESERVICE_VERSION);
		while(1)
		{
			msgsnd(msgid, &msg, sizeof(msg)-sizeof(long), IPC_NOWAIT);
			std::this_thread::sleep_for(std::chrono::seconds(10));
		} });
	sendVersionThread.detach();

	// 默认声卡插拔操作
	voiceCard_init();

	
	bool needSynthesis = confjson->getSynthesis();
	std::string db_path = confjson->getDbPath();
	std::string voice_path = confjson->getVoicePath();
	std::string app_id = confjson->getAppID();
	std::string api_key = confjson->getApiKey();
	std::string secret_key = confjson->getSecretKey();

	std::string fileName_rec = "";

	volumeController volControl;
	volControl.updateSysVolume(conffile);

	VoiceDAO db(db_path);
	VoicePlayer player;
	VoiceSynthesis vs(app_id, api_key, secret_key);
	LocalBus localBus;
	localBus.init();
	localBus.setOnRecieveMsg([&](json js)
							 {
			recordInfo recInfo;

			fileName_rec = js.at("fileName");
			if(fileName_rec != "")
			{
				std::string file = voice_path + fileName_rec;
				player.play(file);
				return;
			}
			else
			{
				recInfo.text = js.at("text");
				if(js.contains("speed"))
					recInfo.speed = js.at("speed");
				else
					recInfo.speed = confjson->getSpeed();
				if(js.contains("pitch"))
					recInfo.pitch = js.at("pitch");
				else
					recInfo.pitch = confjson->getPitch();
				if(js.contains("volume"))
					recInfo.volume = js.at("volume");
				else
					recInfo.volume = confjson->getVolume();
				if(js.contains("voiceType"))
					recInfo.voiceType = js.at("voiceType");
				else
					recInfo.voiceType = confjson->getVoiceType();

				bool ret = false;
				ret = db.checkRecord(recInfo);
				if(ret){
					spdlog::debug("数据库有记录，准备播放语音");
					player.play(voice_path + recInfo.fileName);
					db.updateRecord(recInfo);
					localBus.response(0);
				}
				else if(needSynthesis){
					spdlog::debug("数据库无记录，准备合成语音");
					vs.setOpts(recInfo.text, recInfo.speed, recInfo.pitch, recInfo.volume, recInfo.voiceType);
					time_t now;
					now = time(NULL);
					recInfo.fileName = std::to_string(now) + ".mp3";
					bool ret =false;
					ret = vs.synthesis(voice_path + recInfo.fileName);
					if(ret){
						spdlog::debug("合成语音成功，准备播放并将记录入库");
						player.play(voice_path + recInfo.fileName);
						db.addRecord(recInfo);
						localBus.response(0);
					}
					else{
						localBus.response(1);
					}
				}
				else{
					spdlog::debug("数据库无记录，且不要求合成语音");
					localBus.response(1);
				}
			} });

	
	// 查询文件播放模式
	int playType = VoiceServiceConf::Instance()->getPlayType(); // 0---播放本地文件，1---播放百度语音
	if (playType == 0)
	{
		localVoice localVoi;
		// 向mqtt服务器确认语音版本
		localVoi.start();
	}
	while (1)
	{
		usleep(1000 * 1000 * 30);
	}
	return 0;
}
