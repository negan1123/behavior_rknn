/*
 * transmission_udp.cpp
 * 用UDP协议向服务器发送数据以及接收来自服务器数据的类
 *
 *  Created on: 2019年9月22日
 *      Author: william
 */

#include <exception>
#include <thread>
#include <util/log.h>
#include <transmission/transmission_udp.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
/**
 * UDP传输模式的构造函数，根据传入的参数构造remote_endpoit以及socket对象
 * 参数：
 * 		host 服务器的地址
 * 		port 服务器端口号
 */
TransmissionUDP::TransmissionUDP(string host, int port) : Transmission() {
	io_context io_context;
	this->host = host;
	this->port = port;
	//udp本地端口，自动分配端口
	ip::udp::endpoint local_endpoint(ip::udp::v4(), 0);
	//创建socket对象指针
	udpsocket = new ip::udp::socket(io_context, local_endpoint);
	udpsocket->set_option(ip::udp::socket::reuse_address(true));

	//根据服务器地址和端口构造remote_endpoint对象
	ip::udp::endpoint server_endpoint(ip::address_v4::from_string(host), port);
	remote_endpoint = server_endpoint;
	lastCheckTime = 0;
}

/**
 * 析构函数，释放socket资源
 */
TransmissionUDP::~TransmissionUDP() {
	isRuning = false;

	spdlog::debug("~TransmissionUDP()正在释放资源");

	if(udpsocket != nullptr) {
		delete udpsocket;
		udpsocket = nullptr;
	}
}

/**
 * 向服务端发送消息，返回0表示成功，其它表示失败
 * 参数：
 * 		msg 要发送的字符串
 * 返回值：
 * 		0成功，-1失败
 */
int TransmissionUDP::send(string msg) {
	if(checkNetState() == false)
		return -1;
	try {
		udpsocket->send_to(buffer(msg), remote_endpoint);
	}
	catch(exception &e) {
		spdlog::error(e.what());

		return -1;
	}

	return 0;
}

/**
 * 向服务端发送消息，返回0表示成功，其它表示失败
 * 参数：
 * 		msg 要发送的数据
 * 返回值：
 * 		0成功，-1失败
 */
int TransmissionUDP::send(vector<char> &msg){
	if(checkNetState() == false)
		return -1;
	try {
		if(msg.size() > 0)
			udpsocket->send_to(buffer(msg.data(),msg.size()), remote_endpoint);
	}
	catch(exception &e) {
		spdlog::error(e.what());

		return -1;
	}

	return 0;
}


/**
 * 设置接收数据回调函数
 * 参数：
 * 		callback 接收数据会调函数指针，recieveCallback
 */
void TransmissionUDP::onRecieve(recieveCallback callback) {
	recvCallback = callback;
}

//启动接收数据线程，开始接收数据
void TransmissionUDP::startRecv() {
	if(!isRuning) {
		isRuning = true;
		std::thread t_recv([=](){
			while(isRuning) {
				try {
					char buff[1024];
					size_t len = udpsocket->receive(buffer(buff));

					//将接收到的数据写入到recvBuf
					recvBuf->putBytes(buff, len);

					string line;
					//从recvBuf中读取每一行数据，然后调用回调
					while(recvBuf->getLine(line)) {
						if(recvCallback != nullptr) {
							recvCallback(line);
						}
					}
				}
				catch(asio::error_code& e) {
					spdlog::error(e.message());
				}
			}
		});

		t_recv.detach();
	}
}

//停止接收数据线程
void TransmissionUDP::stopRecv() {
	isRuning = false;
}


bool TransmissionUDP::checkNetState()
{
	time_t timenow = time(NULL);
	if(timenow - lastCheckTime < 10)
	{
		return isNetConnect;
	}
	else
	{
		lastCheckTime = timenow;
	}
#if 0
	io_context io_context;
	asio::error_code error;
	ip::tcp::socket tcpsocket(io_context);
	ip::tcp::endpoint remove_ep(ip::address_v4::from_string(host), port);
	try
	{
		//与服务器连接
		tcpsocket.connect(remove_ep, error);
		//判断是否建立了连接
		if(error) {
			spdlog::error("连接服务器失败！");
			spdlog::error(error.message());
			isNetConnect = false;
		}
		else
		{
			isNetConnect = true;
			tcpsocket.close();
		}
	}
	catch(...)
	{
		isNetConnect = false;
	}
#else
	int fd;
	struct sockaddr_in addr;
	struct timeval timeo = {3, 0};
	socklen_t len = sizeof(timeo);

	fd = socket(AF_INET, SOCK_STREAM,0);
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host.c_str());
	addr.sin_port = htons(port);
	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		if (errno == EINPROGRESS) {
			isNetConnect = false;
		}
		else
			isNetConnect = true;
	}
	else
		isNetConnect = true;
	close(fd);
#endif
	return isNetConnect;
}
