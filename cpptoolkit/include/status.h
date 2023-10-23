/*
 * status.h
 *	跨进程状态管理。指定状态会保存为/tmp文件夹中的一个文件，如：/tmp/lte保存lte当前状态，0表示正在联网，1表示联网成功
 *  Created on: 2022年9月2日
 *      Author: 华为
 */

#ifndef _STATUS_H_
#define _STATUS_H_

#include <fstream>
#include <thread>

class Status {
public:
	Status() = default;
	~Status() = default;

	/**
	 * @brief 将远程总线客户端的状态保存到指定文件
	 * @param[in] flag 远程总线客户端状态，0正在连接，1连接成功可以正常通信
	 */
	static void setRbusFlag(int flag) {
		std::ofstream ofs("/tmp/rbus_flag");
		ofs << flag;
		ofs.close();
	}

	/**
	 * @brief 获取远程总线客户端状态
	 * @return 远程总线客户端状态，0正在连接，1连接成功可以正常通信
	 */
	static int getRbusFlag() {
		int flag = 0;
		
		std::ifstream ifs("/tmp/rbus_flag");
		if (!ifs.is_open()) {
			spdlog::debug("OPERATION FAILED: Unable to get value of rbus_flag");
			// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		else {
			ifs >> flag;
			ifs.close();
		}

		return flag;
	}

	/**
	 * @brief 将LTE联网状态保存到指定文件
	 * @param[in] flag LTE联网状态: 0未联网; 1已联网,还需租借ip; 2已联网，可正常通信
	 */
	static void setLteFlag(int flag) {
		std::ofstream ofs("/tmp/lte_flag");
		ofs << flag;
		ofs.close();
	}

	/**
	 * @brief 获取LTE联网状态
	 * @return LTE联网状态，LTE联网状态: 0未联网; 1已联网,还需租借ip; 2已联网，可正常通信
	 */
	static int getLteFlag() {
		int flag = 0;
		
		std::ifstream ifs("/tmp/lte_flag");
		if (!ifs.is_open()) {
			spdlog::debug("OPERATION FAILED: Unable to get value of lte_flag");
			// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		else {
			ifs >> flag;
			ifs.close();
		}

		return flag;
	}
};

#endif /* _STATUS_H_ */
