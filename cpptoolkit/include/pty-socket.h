/*
 * transmission_tcp.h
 * 利用tcp协议进行传输的类
 *
 *  Created on: 2019年9月26日
 *      Author: william
 */

#ifndef PTY_SOCKET_H_
#define PTY_SOCKET_H_

#include <iostream>
#include <asio.hpp>

using namespace asio;
using namespace std;

/**
 * @brief 收到来自socket数据后的回调函数
 *
 * @param sid std::string 会话id，信令服务器创建一个新pty将传入一个sessionId
 * @param data char* 接收到的数据指针
 * @param len size_t 接收到的数据长度
 */
using  receiveCallback = function<void(const std::string& sid, char* data, size_t len)> ;

class PtySocket {
public:
	PtySocket(const string sid, const string host, int port);

	virtual ~PtySocket();

	/**
	 * @brief 与服务器建立tcp连接
	 *
	 * @returns true 连接成功，false连接失败；如果已经连接成功再次调用connect()，什么都不会做，直接返回true
	 */
	bool connect();

	/**
	 * @brief 通过socket向对方写入数据
	 *
	 * @param data char* 要发送的数据
	 * @param len size_t 发送的数据长度
	 * @returns 发送的数据长度
	 *
	 * @throws asio::system_error 发送失败抛出异常
	 */
	size_t write(const char *data, size_t len);

	/**
	 * @brief 启动接收数据的线程，从socket接收数据，收到数据后调用回调函数
	 *
	 * @throws asio::system_error 接收失败抛出异常
	 */
	void startRead();

	/**
	 * @brief 关闭ptySocket
	 */
	void close();

public:
	//接收回调函数指针
	receiveCallback onRecieve{nullptr};

private:
	asio::io_context io_context;

	// 会话ID,每一个ptySocket有一个固定的sessionID，这个值由信令服务器传过来
	std::string sessionId;
	// 服务器地址和端口
	std::string host;
	int port;

	// 定义tcp的socket智能指针
	std::shared_ptr<ip::tcp::socket> socket{nullptr};

	// 是否和服务器建立了连接
	bool isConnect{false};
};

#endif /* PTY_SOCKET_H_ */
