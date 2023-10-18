/*
 * transmission.h
 * 定位终端与服务器传输的类
 *  Created on: 2019年9月22日
 *      Author: william
 */

#ifndef TRANSMISSION_H_
#define TRANSMISSION_H_

#include <iostream>
#include <functional>
#include <util/recvbuf.h>

using namespace std;

// 定义收到消息的回调函数
// 接收消息的回调函数带有一个参数：接收到的消息
typedef function<void(const std::string& msg)> recieveCallback;
// 接收二进制数据的回调函数，这个回调函数优先级高于接收字符串
using RECV_CHAR_CALLBACK = function<void(const char *buf, size_t len)>;

class Transmission {
public:
	//默认构造函数，创建recvBuf对象
	Transmission() {
		recvBuf = make_unique<RecvBuf>();
	}

	//析构函数，释放资源(虚函数，释放时调用的是子类析构函数)
	virtual ~Transmission() {};

	//向服务端发送消息，返回0表示成功，其它表示失败
	virtual int send(string msg)=0;
	virtual int send(vector<char> &msg){return 0;}
	//设置接收到消息后的回调函数，当收到消息的时候，回调函数将会被调用
	virtual void onRecieve(recieveCallback callback)=0;

	//启动接收数据线程，开始接收数据
	virtual void startRecv()=0;

	//停止接收数据线程
	virtual void stopRecv()=0;

protected:
	//接收数据缓冲，通过这个缓冲可以得到每行数据
	unique_ptr<RecvBuf> recvBuf{nullptr};
};

#endif /* TRANSMISSION_H_ */
