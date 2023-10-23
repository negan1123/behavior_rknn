/*
 * HwInfo.cpp
 *
 *  Created on: Sep 23, 2021
 *      Author: wy
 */

#include <iostream>
#include <fstream>
#include <openssl/md5.h>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <HwInfo.h>
#include <base64.h>

namespace util{

HwInfo::HwInfo():salt1(SALT1), salt2(SALT2)
{

}

HwInfo::~HwInfo()
{

}

bool HwInfo::init()
{
	std::string file_path = "/proc/cpuinfo";	//开发板使用
	std::ifstream cpuInfo(file_path.c_str());
	if(!cpuInfo)
	{
		std::cout << "打开cpuinfo文件失败!" << std::endl;
		return false;
	}

	// 逐行读取cpuinfo文件，直到Serial所在行，将Serial行内容存入数据成员cpuID
	std::string file_data;
	int loc;
	while(!cpuInfo.eof())
	{
		getline(cpuInfo, file_data);
		loc = file_data.find("Serial");
		if(loc != std::string::npos)
		{
			loc += 10;	//串口号前面有10个字符，需要跳过
			cpuID = file_data.substr(loc);
		}
	}
	cpuInfo.close();

	// 读取MAC地址，存入数据成员mac中
	file_path = "/sys/class/net/wlan0/address";	//开发板使用
	std::ifstream Mac(file_path.c_str());
	if(!Mac)
	{
		std::cout << "打开address文件失败!" << std::endl;
		return false;
	}
	if(!Mac.eof())
	{
		getline(Mac, file_data);
	}
	mac = file_data;

	// 用cpuid和mac地址生成指纹，存入数据成员finger中
	finger = genarateFinger(cpuID, mac);

	return true;
}

std::string HwInfo::getCPUID()
{
	return cpuID;
}


std::string HwInfo::getMac()
{
	return mac;
}

std::string HwInfo::getFinger()
{
	return finger;
}

std::string HwInfo::genarateFinger(const std::string &id, const std::string &mac)
{
	// 去除MAC地址当中的“：”，供加密算法使用
	int loc = 0;
	std::string _mac;
	_mac = mac;
	loc = _mac.find(':');
	while(loc != std::string::npos)
	{
		_mac.erase(loc,1);
		loc = _mac.find(':',loc);
	}

	MD5_CTX ctx;
	std::string md5_string;
	unsigned char md[16] = {0};
	char tmp[33] = {0};
	MD5_Init(&ctx);

	// 使用MD5加密算法对id加密
	MD5_Update(&ctx, id.c_str(), id.size());
	MD5_Final(md, &ctx);
	for( int i = 0; i < 16; ++i )
	{
		memset(tmp, 0x00, sizeof(tmp));
		sprintf(tmp, "%02X", md[i]);
		md5_string += tmp;
	}

	md5_string += salt1;	//加点盐

	// 使用MD5加密算法对_mac加密
	//MD5_Init(&ctx);
	memset(md, 0, sizeof(md));
	MD5_Update(&ctx, _mac.c_str(), _mac.size());
	MD5_Final(md, &ctx);

	for( int i = 0; i < 16; ++i )
	{
		memset(tmp, 0x00, sizeof(tmp));
		sprintf(tmp, "%02X", md[i]);
		md5_string += tmp;
	}

	md5_string += salt2;	//加点盐

	// 将加过盐的字符串再次进行MD5加密
	//MD5_Init(&ctx);
	memset(md, 0, sizeof(md));
	MD5_Update(&ctx, md5_string.c_str(), md5_string.size());
	MD5_Final(md, &ctx);
	md5_string.clear();
	for( int i = 0; i < 16; ++i )
	{
		memset(tmp, 0x00, sizeof(tmp));
		sprintf(tmp, "%02X", md[i]);
		md5_string += tmp;
	}
	// 对MD5加密结果进行base64编码
	// base64编码是对字符串进行重新编码，实际上，源字符串的每个字符是八个二进制位表示，
	// base64编码将整个字符串每六个二进制位组成一个新的base64编码下的字符，最后得到新编码规则下的字符串
	std::string final_code;
	final_code = base64_encode(reinterpret_cast<const unsigned char*>(md5_string.c_str()), md5_string.size());

	return final_code;
}

bool HwInfo::verifyFinger(const std::string &check)
{
	/*
	if(check == finger)
	{
		std::cout << "指纹正确!" <<std::endl;
		return true;
	}
	else
	{
		std::cout << "指纹错误!" <<std::endl;
		return false;
	}
	*/

	return true;
}

}


