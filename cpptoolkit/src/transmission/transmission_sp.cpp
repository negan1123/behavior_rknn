/*
 * transmission_sp.cpp
 * 串口传输类，可以实现与从串口读取或者写入数据
 *
 *  Created on: 2019年9月25日
 *      Author: william
 */

#include <exception>
#include <thread>
#include <util/log.h>
#include <transmission/transmission.h>
#include <transmission/transmission_sp.h>

/**
 * 构造函数，指定设备名称、波特率创建串口对象。默认波特率9600
 */
TransmissionSP::TransmissionSP(const std::string &port, uint32_t baudrate) : Transmission() {
	this->port = port;
	this->baudrate = baudrate;

	//打开串口
	openPort();
}

/**
 * 析构函数，释放资源
 */
TransmissionSP::~TransmissionSP() {
	//停止接收线程
	stopRecv();
	//等待线程结束
	thread_recv.join();

	spdlog::debug("~TransmissionSP()正在释放资源");
	closePort();
}

//检查指定的串口是否已经存在(驱动程序已经加载)
bool TransmissionSP::checkPort() {
	stringstream ss;

	//先枚举所有的串口，查看要打开的串口是否在列表中
	auto devices_found = serial::list_ports();
	auto iter = devices_found.begin();

	bool is_found = false;
	while( iter != devices_found.end() ) {
		serial::PortInfo device = *iter++;
		if(device.port == port) {
			is_found = true;
			break;
		}
	}

	if(!is_found) {
		ss << "没有发现串口设备：" << port;
		spdlog::error(ss.str());
	}

	return is_found;
}

//打开串口，true打开成功，false打开失败
bool TransmissionSP::openPort() {
	bool ret = false;

	//检查端口是否存在(驱动是否已经加载，准备就绪了)
	if(checkPort()) {
		try {
			if(serial == nullptr) {
				serial = new serial::Serial(port, baudrate, serial::Timeout::simpleTimeout(400));
				if(serial->isOpen()) {
					ret = true;
					spdlog::debug(port + " 已经成功打开");
				}
				else {
					spdlog::debug(port + " 打开失败");
				}
			}
			else {
				//如果是已经打开了，也返回true
				if(serial->isOpen()) {
					ret = true;
				}
			}
		}
		catch(...) {
			spdlog::debug(port + " 打开失败");
		}
	}

	return ret;
}

//关闭串口，true关闭成功，false关闭失败
bool TransmissionSP::closePort() {
	//关闭串口，释放资源
	if(serial != nullptr) {
		serial->close();

		delete serial;
		serial = nullptr;

		spdlog::debug(port + " 已关闭");
	}

	return true;
}

//向串口发送消息，返回-1表示失败
int TransmissionSP::send(string msg) {
	return send(msg.c_str(), msg.length());
}

//向串口发送消息，返回-1表示失败,其它表示成功
int TransmissionSP::send(const char* buf, int len) {
	int s_len = -1;

	try {
		if(serial == nullptr) return -1;

		s_len = serial->write((const uint8_t*)buf, len);
	}
	catch(serial::PortNotOpenedException& e) {
		spdlog::error("串口没有打开");
	}
	catch(serial::SerialException& e) {
		spdlog::error("串口出现异常");
	}
	catch(serial::IOException& e) {
		spdlog::error("IO出现异常");
	}
	catch(...) {
		spdlog::error("SP send出现异常");
	}

	return s_len;
}

/**
 * @brief 从串口读取数据
 * @param buf 接收的数据指针
 * @param len 最大能接收的数据长度
 * @return -1表示接收失败，>0表示接收到的数据长度
*/
int TransmissionSP::read(char *buf, int len) {
	int ret = -1;

	try {
		if(serial == nullptr) return -1;

		ret = serial->read((uint8_t *)buf, len);
	}
	catch(serial::PortNotOpenedException& e) {
		spdlog::error("串口没有打开");
	}
	catch(serial::SerialException& e) {
		spdlog::error("串口出现异常");
	}
	catch(serial::IOException& e) {
		spdlog::error("IO出现异常");
	}
	catch(...) {
		spdlog::error("SP read出现异常");
	}

	return ret;
}

/**
 * 设置接收数据回调函数
 * 参数：
 * 		callback 接收数据会调函数指针，recieveCallback
 */
void TransmissionSP::onRecieve(recieveCallback callback) {
	recvCallback = callback;
}

/**
 * 设置接收数据回调函数
 * 参数：
 * 		callback 接收数据会调函数指针，recieveCallback
 */
void TransmissionSP::onRecieveChar(RECV_CHAR_CALLBACK callback) {
	recvCharCallback = callback;
}

/**
* @brief 接收数据线程处理函数
*/
void TransmissionSP::recvHandler() {
	while(isRuning) {
		try {
			char buff[1024];

			//如果serial没有初始化，则抛出异常
			if(serial == nullptr) {
				this_thread::sleep_for(std::chrono::seconds(3));
				throw new serial::SerialException("串口没有发现或未打开!");
			}
			else {
				if(serial->isOpen()) {
					size_t len = serial->read((uint8_t *)buff, 1024);

					if(len > 0){
						// 如果有recvCharCallback，优先调用这个回调
						if(recvCharCallback) {
							recvCharCallback(buff, len);
						}
						else {
							string data = string(buff,len);
							if(recvCallback != nullptr) {
								recvCallback(data);
							}
						}
					}
				}
			}
		}
		catch(serial::PortNotOpenedException& e) {
			spdlog::error("串口没有打开");
		}
		//catch(serial::SerialException& e) {
		//	spdlog::error("串口出现异常");
		//}
		catch(...) {
			spdlog::error("串口读取出现问题");
			//先关闭串口
			closePort();

			//再重新打开串口
			openPort();
		}
	}
}

//启动接收数据线程，开始接收数据
void TransmissionSP::startRecv() {
	if(!isRuning) {
		isRuning = true;
		// 开启数据接收线程
		thread_recv.reset(&TransmissionSP::recvHandler, this);
	}
}

//停止接收数据线程
void TransmissionSP::stopRecv() {
	isRuning = false;
}

