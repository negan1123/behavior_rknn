/*
 * recvbuf.cpp
 * 数据接收缓冲，对于udp、tcp通用
 *  Created on: 2019年9月24日
 *      Author: william
 */

#include <algorithm>
#include <util/recvbuf.h>

/**
 * 构造函数
 * 参数：
 * 		size 缓冲区大小，默认4K
 */
RecvBuf::RecvBuf(size_t size) : ByteBuffer(size){
}

/**
 * 构造函数
 * 参数：
 * 		arr 数据指针
 * 		size 缓冲区大小
 */
RecvBuf::RecvBuf(char* arr, size_t size) : ByteBuffer((uint8_t *)arr, size){
}

/**
 * 从缓冲中提取指定长度的数据
 * 参数：
 * 		buf char* 提取出来的数据保存的位置
 * 		len size_t 提取的数据长度
 */
void RecvBuf::getBytes(char* buf, size_t len) {
	//在一个类里定义了一个const成员函数后，则此函数不能修改类中的成员变量
	bb::ByteBuffer::getBytes((uint8_t *)buf, len);

	//如果rpos和wpos相等，证明数据已经读取完毕，位置归0
	//这样做可以防止内存消耗不回收的问题
	if(rpos == wpos) clear();
}

/**
 * 指定长度，从缓冲中提取字符串
 * 参数：
 * 		len size_t 要提取字符串的长度
 * 返回值：
 *		提取到的字符串
 */
std::string RecvBuf::getString(size_t len) {
	char *buf = new char[len]{0};
	getBytes(buf, len);

	std::string s = std::string(buf, len);
	delete buf;

	return s;
}

/**
 * 将指定长度的数据保存到缓存中
 * 参数：
 * 		buf char* 要写入到缓冲中的数据指针
 * 		len size_t 要写入到缓冲中的数据长度
 */
void RecvBuf::putBytes(char* buf, size_t len) {
	bb::ByteBuffer::putBytes((uint8_t *)buf, len);
}

/**
 * 将字符串写入到缓冲中
 * 参数：
 * 		s string 要写入到缓存中的字符串
 */
void RecvBuf::putString(std::string s) {
	putBytes((char *)s.c_str(), s.length());
}

/**
 * 重载了运算符，使得可以通过<<将字符串写入到缓存中
 */
RecvBuf& RecvBuf::operator <<(std::string s) {
	putString(s);

	return *this;
}

/**
 * 读取一行数据，并将读位置移到新的一行开始位置
 * 参数：
 * 		line 读取到的行字符，当返回值为true的时候有效
 * 		delimeter 分界符指针
 * 		delim_len 分界符长度
 * 	返回值：
 * 		true行数据有效，false行数据无效，没有读取到有效的分界符
 */
bool RecvBuf::getLine(std::string& line, const char* delimeter, size_t delim_len) {
	bool result = false;

	const char default_delim[2] = {'\r', '\n'};
	if(delimeter==nullptr) delimeter = default_delim;

	auto s_it = buf.begin() + rpos;
	auto e_it = buf.begin() + wpos;
	auto it = std::search(s_it, e_it, delimeter, delimeter + delim_len);

	int rlen = 0;
	if (it != e_it) {
		rlen = it - s_it;
		if(rlen > 0) {
			line = getString(rlen);
		}
		else {
			line = "";
		}

		//新的位置=读开始位置+读的字符长度+分界符长度
		rpos += delim_len;

		result = true;
	}

	return result;
}
