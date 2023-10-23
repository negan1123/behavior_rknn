/*
 * transmission_tcp.cpp
 * 利用tcp协议进行传输的类
 *
 *  Created on: 2019年9月26日
 *      Author: william
 */

#include <thread>
#include <util/log.h>
#include <util/crc24q.h>
#include <HwInfo.h>
#include "pty-socket.h"

const int max_length = 1024;

/**
 * 构造函数。初始化socket并和tcp服务器连接
 */
PtySocket::PtySocket(const string sid, const string host, int port) {
	this->sessionId = sid;
	//保存服务器连接信息，重连的时候需要用到
	this->host = host;
	this->port = port;
	// 创建socket指针
	socket = std::make_shared<ip::tcp::socket>(io_context);
}

/**
 * 析构函数。由于用了智能指针，内存会自动释放，暂时不用作处理
 */
PtySocket::~PtySocket() {
	spdlog::debug("~PtySocket()正在释放资源");

//	close();
}

/**
 * @brief 根据传入的4bytes数据，结合设备的指纹得到验证码，这个验证码需要和客户端传过来的进行比对
 *
 * @param data 字节的随机数据
 * @return unsigned类型的数字
 */
unsigned getVerifyCode(char* data) {
        util::HwInfo hw;
        // 首先获取到硬件指纹
        string finger = hw.getFinger();

        // 分配比指纹多4个字节，将信息拼起来
        size_t len = finger.length();
        unsigned char tmp_data[len + 4];
        memcpy(tmp_data, finger.c_str(), len);
        memcpy(tmp_data, data, 4);

        // 根据硬件指生成crc24的摘要
        unsigned hwHash = crc24q_hash(tmp_data, sizeof(tmp_data));

        return hwHash;
}

/**
 * 断线重连
 */
bool PtySocket::connect() {
	try {
		asio::error_code error;

		if(!isConnect && socket != nullptr) {
			ip::tcp::endpoint remote_ep(ip::address_v4::from_string(host), port);
			//与服务器连接
			socket->connect(remote_ep, error);
			//判断是否建立了连接
			if(error) {
				spdlog::error("连接服务器失败！");
				spdlog::error(error.message());
			}
			else {
				//设置option
				socket->set_option(ip::tcp::socket::reuse_address(true));
				socket->set_option(ip::tcp::socket::keep_alive(true));

				char sData[]{0x05, 0x05};
				// 向服务器发送0x05, 0x05表示开始请求认证
				socket->write_some(asio::buffer(sData, 2), error);
				if(error) throw asio::system_error(error);

				char data[8]{0};
				size_t len = socket->read_some(asio::buffer(data, 8), error);
				// 如果接收失败或者收到了认证失败的信息，则退出
				if (error || (data[0] == 0x18 && data[1] == 0x18)) throw asio::system_error(error);

				if(len == 4) {
					// 根据收到的生成验证码，发到服务器进行验证
					unsigned verify_code = getVerifyCode(data);
					socket->write_some(asio::buffer((char *)&verify_code, len), error);
					if(error) throw asio::system_error(error);

					// 如果服务器返回0x04, 0x04，表示认证成功；返回0x18、0x18表示认证失败，会主动断开连接
					len = socket->read_some(asio::buffer(data, max_length), error);
					if (error) throw asio::system_error(error);
					if(data[0] == 0x04 && data[1] == 0x04) {
						isConnect = true;
					}
					else {
						isConnect = false;
					}
				}
			}
		}
	}
	catch (asio::system_error &e) {
		// 如果socket出问题，关闭通信，直接返回
		perror(e.what());
		isConnect = false;
	}

	return isConnect;
}

size_t PtySocket::write(const char *data, size_t len) {
	int result = -1;

	if(isConnect) {
		asio::error_code error;
		result = socket->write_some(asio::buffer(data, len), error);
		if(error) {
			result = -1;
			spdlog::error(error.message());
		}
	}

	return result;

}

void PtySocket::startRead() {
	if(isConnect) {
		std::thread t_recv([=](){
			try {
				for(;;) {
					asio::error_code error;
					char data[max_length];
					int rlen = socket->available();
					if(rlen > 0) {
						size_t len = socket->read_some(buffer(data, rlen), error);

						if(error) {
							throw asio::system_error(error);
						}

						// 成功接收到数据，调用回调函数
						if(onRecieve != nullptr) {
							onRecieve(sessionId, data, len);
						}

					}

					//休眠10ms，如果不休眠，cpu占用会很高
					this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			}
			catch(asio::system_error& e) {
				spdlog::error(e.what());
				isConnect = false;
			}
		});

		t_recv.detach();
	}
}

void PtySocket::close() {
	isConnect = false;
	socket->close();
}
