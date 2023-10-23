/*
 * transmission_tcp.cpp
 * 利用tcp协议进行传输的类
 *
 *  Created on: 2019年9月26日
 *      Author: william
 */

#include <thread>
#include <util/log.h>
#include <transmission/transmission_tcp.h>

/**
 * 构造函数。初始化socket并和tcp服务器连接
 */
TransmissionTcp::TransmissionTcp(string host, int port) : Transmission() {
	//保存服务器连接信息，重连的时候需要用到
	this->host = host;
	this->port = port;

	//与服务器进行连接
	connect();
}

/**
 * 析构函数。由于用了智能指针，内存会自动释放，暂时不用作处理
 */
TransmissionTcp::~TransmissionTcp() {
	spdlog::debug("~TransmissionTcp()正在释放资源");

	isRuning = false;
	socket->close();
}

/**
 * 将回调函数保存到类变量中
 */
void TransmissionTcp::onRecieve(recieveCallback callback) {
	recvCallback = callback;
}

/**
 * 通过socket发送消息，发送成功返回>0的数据表示发送成功的数量，-1表示发送失败
 * 参数：
 * 		msg string 要发送的数据
 * 返回值：
 * 		-1 发送失败，其它表示发送成功的字符数量
 */
int TransmissionTcp::send(string msg) {
	int result = -1;

	if(isConnect) {
		asio::error_code error;
		result = socket->write_some(buffer(msg), error);
		if(error) {
			result = -1;
			spdlog::error(error.message());

			// 断线重连
			isConnect = false;
			isRuning = false;
		}
	}

	return result;
}

/**
 * 启动数据接收线程
 */
void TransmissionTcp::startRecv() {
	if(!isRuning && isConnect) {
		// 线程运行的标识
		isRuning = true;
		std::thread t_recv([=](){
			while(isRuning && isConnect) {
				try {
					asio::error_code error;
					char buff[1024];
					int rlen = socket->available();
					if(rlen > 0) {
						size_t len = socket->read_some(buffer(buff, rlen), error);

						if(error) {
							throw asio::system_error(error);
						}

						// 将接收到的数据写入到recvBuf
						recvBuf->putBytes(buff, len);

						string line;
						// 从recvBuf中读取每一行数据，然后调用回调
						while(recvBuf->getLine(line)) {
							if(recvCallback != nullptr) {
								recvCallback(line);
							}
						}
					}

					// 休眠10ms，如果不休眠，cpu占用会很高
					this_thread::sleep_for(std::chrono::milliseconds(10));
				}
				catch(asio::system_error& e) {
					spdlog::error(e.what());

					isConnect = false;
					connect();

					break;
				}
			}
		});

		t_recv.detach();
	}
}

/**
 * 停止接收线程
 */
void TransmissionTcp::stopRecv() {
	if(isConnect) {
		isRuning = false;
	}
}

/**
 * 断线重连
 */
void TransmissionTcp::connect() {
	asio::error_code error;

	//初次连接，创建socket指针
	if(socket == nullptr) {
		socket = std::make_unique<ip::tcp::socket>(io_context);
	}

	while(!isConnect && socket != nullptr) {
		// 重连的时候需要关闭socket，释放socket指针，然后重新创建指针
		if(socket->is_open()) {
			socket->close(error);
			if(error){
				spdlog::error(error.message());
			}

			// 释放智能指针
			socket.reset();
			socket = std::make_unique<ip::tcp::socket>(io_context);
		}

		// 代表服务器的endpoint
		ip::tcp::endpoint remote_ep(ip::address_v4::from_string(host), port);
		// 与服务器连接
		socket->connect(remote_ep, error);
		// 判断是否建立了连接
		if(error) {
			spdlog::error("连接服务器失败！");
			spdlog::error(error.message());
		}
		else {
			isConnect = true;

			// 设置option
			socket->set_option(ip::tcp::socket::reuse_address(true));
			socket->set_option(ip::tcp::socket::keep_alive(true));

			if(isRuning) {
				isRuning = false;
				// 如果之前处于接收状态，恢复接收状态
				this->startRecv();
			}

			break;
		}

		// 每隔5s尝试重新连接
		this_thread::sleep_for(std::chrono::seconds(5));
		spdlog::debug("正在尝试重连服务器。。。");
	}
}
