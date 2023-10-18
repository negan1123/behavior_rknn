/*
 * transmission_udp.h
 * 用UDP协议向服务器发送数据以及接收来自服务器数据的类
 *
 *  Created on: 2019年9月22日
 *      Author: william
 */

#ifndef TRANSMISSION_UDP_H_
#define TRANSMISSION_UDP_H_

#include <asio.hpp>
#include "transmission.h"

using namespace asio;
using namespace std;

class TransmissionUDP: public Transmission {
public:
	//udp传输方式的构造函数，需要传入host和port
	TransmissionUDP(string host, int port);

	//析构函数，释放资源
	~TransmissionUDP();

	//向服务端发送消息，返回0表示成功，其它表示失败
	int send(string msg);
	int send(vector<char> &msg);

	//设置接收到消息后的回调函数，当收到消息的时候，回调函数将会被调用
	void onRecieve(recieveCallback callback);

	//启动接收数据线程，开始接收数据
	void startRecv();

	//停止接收数据线程
	void stopRecv();

private:
	bool checkNetState();

private:
	//服务器udp端口
	ip::udp::endpoint remote_endpoint;

	//udp socket对象，包含了本地端口信息
	ip::udp::socket *udpsocket = nullptr;

	//接收回调函数指针
	recieveCallback recvCallback = nullptr;

	//不会发生线程冲突，线程安全
	//接收数据线程是否处于运行状态，默认是运行状态
	atomic<bool> isRuning{ false };
	//判断网络是否在线
	atomic<bool> isNetConnect{ false };
	//服务器地址和端口
	std::string host;
	int port;
	time_t lastCheckTime;
};

#endif /* TRANSMISSION_UDP_H_ */
