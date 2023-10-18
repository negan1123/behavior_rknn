/*
 * commons.h
 * 公共工具类方法
 *
 *  Created on: 2019年9月28日
 *      Author: william
 */

#ifndef SRC_UTIL_COMMONS_H_
#define SRC_UTIL_COMMONS_H_

#include <iostream>
#include <vector>

using namespace std;

namespace util {

/**@union uBigLittleEndianChange
	* @brief 整形，float大小端转化联合
*/
typedef union _uBigLittleEndianChange_ {
	struct{
		char byte3;
		char byte2;
		char byte1;
		char byte0;
	}sBytes;				///<	转化的字节结构
	float fData;		///<	需转换的float值
	int iData;			///<	需转换的int值
}uBigLittleEndianChange;

//判断是否是以一个字符串开始
bool startWith(const string& str, const string& s_str);
//判断是否以一个字符串结束
bool endWith(const string& str, const string& e_str);
//将以指定字符分隔的字符串转化成数组
vector<string> split(const string& str, const string& delim=",");
//传入一个buf，指定长度，求校验和
uint8_t checksum(const uint8_t* buf, size_t len);
//对指定的string求校验和
string checksum(const string& str);
//获取从纪元时间到现在的毫秒数
int64_t curentTimeMillis(void);

/**@brief 控制LED灯亮
* @param[in]  open				LED亮状态
* @retval		None
*/
void blueLedCtrl(bool open);
/**@brief 控制LED灯亮闪周期
* @param[in]  duration				亮灭间隔
* @param[in]  times						持续时长
* @retval		None
*/
void led_lighting(int duration, int times);

/**@brief 将字符串的UTC时间转化为东八区的时间秒数
* @param[in]  timestr				数据字符串 "2020-12-09 12:12:02"
* @retval	time_t			从1970-01-01 00:00:00到timestr的秒数 +东八区
*/
time_t mymktime(const string &timestr);

}

#endif /* SRC_UTIL_COMMONS_H_ */
