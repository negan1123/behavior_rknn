/*
 * recvbuf.h
 * 数据接收缓冲，对于udp、tcp通用
 *  Created on: 2019年9月24日
 *      Author: william
 */

#ifndef RECVBUF_H_
#define RECVBUF_H_

#include <iostream>
#include <ByteBuffer.hpp>

class RecvBuf: public bb::ByteBuffer {
public:
	RecvBuf(size_t size = BB_DEFAULT_SIZE);
	RecvBuf(char* arr, size_t size);
	//C++11 标准引入了一个新特性："=default"函数。程序员只需在函数声明后加上“=default;”，就可将该函数声明为 "=default"函数，编译器将为显式声明的 "=default"函数自动生成函数体。
	virtual ~RecvBuf()=default;

	void getBytes(char* buf, size_t len);
	std::string getString(size_t len);

	void putBytes(char* buf, size_t len);
	void putString(std::string s);

	RecvBuf& operator <<(std::string s);
	//根据指定的分界符获取一行字符串，如果没有查到返回false，有满足条件的情况返回0
	bool getLine(std::string& line, const char* delimeter=nullptr, size_t len=2);
};

#endif /* RECVBUF_H_ */
