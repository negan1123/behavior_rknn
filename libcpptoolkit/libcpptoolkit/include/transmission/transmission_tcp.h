/*
 * transmission_tcp.h
 * 利用tcp协议进行传输的类
 *
 *  Created on: 2019年9月26日
 *      Author: william
 */

#ifndef TRANSMISSION_TCP_H_
#define TRANSMISSION_TCP_H_

#include <asio.hpp>
#include "transmission.h"

using namespace asio;

class TransmissionTcp: public Transmission {
public:
	TransmissionTcp(string host, int port);
	virtual ~TransmissionTcp();
	void onRecieve(recieveCallback callback);
	int send(string msg);
	void startRecv();
	void stopRecv();
	//与服务器建立tcp连接
	void connect();

private:
	//服务器地址和端口
	std::string host;
	int port;

	//定义tcp的socket智能指针
	std::unique_ptr<ip::tcp::socket> socket{nullptr};

	//接收回调函数指针
	recieveCallback recvCallback{nullptr};

	//是否和服务器建立了连接
	bool isConnect{false};

	//不会发生线程冲突，线程安全
	//接收数据线程是否处于运行状态，默认是运行状态
	atomic<bool> isRuning{ false };

	asio::io_context io_context;
};

#endif /* TRANSMISSION_TCP_H_ */
