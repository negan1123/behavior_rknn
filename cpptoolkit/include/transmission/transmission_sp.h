/*
 * transmission_sp.h
 * 串口传输类，可以实现与从串口读取或者写入数据
 *
 *  Created on: 2019年9月25日
 *      Author: william
 */

#ifndef TRANSMISSION_SP_H_
#define TRANSMISSION_SP_H_

#include <asio.hpp>
#include <serial/serial.h>
#include <thread_raii.h>
#include "transmission.h"

class TransmissionSP: public Transmission {
public:
	TransmissionSP(const std::string &port = "", uint32_t baudrate = 9600);
	virtual ~TransmissionSP();

	//向串口发送消息，返回0表示成功，其它表示失败
	int send(string msg);

	//向串口发送消息，返回0表示成功，其它表示失败
	int send(const char* buf, int len);

	/**
	 * @brief 从串口读取数据
	 * @param buf 接收的数据指针
	 * @param len 最大能接收的数据长度
	 * @return -1表示接收失败，>0表示接收到的数据长度
	*/
	int read(char *buf, int len);

	//设置接收到串口消息后的回调函数，当收到消息的时候，回调函数将会被调用
	void onRecieve(recieveCallback callback);

	//设置接收到消息后的回调函数，这个回调函数优先级高于onRecieve
	void onRecieveChar(RECV_CHAR_CALLBACK callback);

	//启动接收数据线程，开始接收数据
	void startRecv();

	//停止接收数据线程
	void stopRecv();

	//检查指定的串口是否已经存在(驱动程序已经加载)
	bool checkPort();

	//打开串口，true打开成功，false打开失败
	bool openPort();

	//关闭串口，true关闭成功，false关闭失败
	bool closePort();

private:
	/**
 	* @brief 接收数据线程处理函数
 	*/
	void recvHandler();

private:
	//串口名称
	std::string port;

	//波特率
	uint32_t baudrate;

	//串口对象
	serial::Serial* serial=nullptr;

	//接收回调函数指针
	recieveCallback recvCallback = nullptr;

	//接收回调函数指针，这个回调函数优先级高于recvCallback
	RECV_CHAR_CALLBACK recvCharCallback = nullptr;

	//不会发生线程冲突，线程安全
	//接收数据线程是否处于运行状态，默认是运行状态
	atomic<bool> isRuning{ false };

  	//串口数据接收线程
	libcpp::Thread thread_recv;
};

#endif /* TRANSMISSION_SP_H_ */
