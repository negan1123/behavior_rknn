#ifndef BEHAVIOR_STATUS_H_
#define BEHAVIOR_STATUS_H_

#include <fstream>
#include <thread>
#include <spdlog/spdlog.h>

/**
 * @brief 驾驶行为程序状态检测，将程序状态写入临时文件中，供守护进程检测程序运行状态
 */
class BehaviorStatus {
public:
	BehaviorStatus() = default;
	~BehaviorStatus() = default;

	/**
	 * @brief 将行为识别客户端的状态保存到指定文件
	 * @param[in] flag 行为识别客户端的状态，0正在加载模型，1加载成功可以正常检测
	 */
	static void setBehaviorFlag(int flag) {
		std::ofstream ofs("/tmp/behavior_flag");
		ofs << flag;
		ofs.close();
	}

	/**
	 * @brief 获取行为识别客户端状态
	 * @return 行为识别客户端客户端状态，0正在加载模型，1加载成功可以正常检测
	 */
	static int getBehaviorFlag() {
		int flag = 0;
		
		std::ifstream ifs("/tmp/behavior_flag");
	    	if (!ifs.is_open()){
	    		spdlog::debug("OPERATION FAILED:");
	    		spdlog::debug("Unable to get value of behavior_flag");
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	    	}
		else {
	    		ifs >> flag;
			ifs.close();
		}

	    	return flag;
	}
};

#endif /* BEHAVIOR_STATUS_H_ */
