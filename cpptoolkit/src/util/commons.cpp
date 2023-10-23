/*
 * commons.cpp
 * 公共工具类方法
 *
 *  Created on: 2019年9月28日
 *      Author: william
 */

#include <chrono>
#include <util/commons.h>
#include <fstream>
#include <thread>

namespace util {

/**
 * 判断原始报文是否是以指定字符串开始
 * 参数：
 * 		str 完整的字符串
 * 		s_str 开始的字符串
 * 返回值：
 * 		true 以指定字符串开始；false 不是以指定字符串开始
 */
bool startWith(const string& str, const string& s_str) {
	bool result{false};

	//str的长度必须大于等于s_str
	if(str.size() < s_str.size()) return result;

	size_t pos = str.find(s_str);
	if(pos == string::npos || pos != 0) {
		//没有找到字符串或者所在的位置不是0，表示不是以指定字符串开始
		result = false;
	}
	else {
		result = true;
	}

	return result;
}

bool endWith(const string& str, const string& e_str) {
	bool result{false};

	//str的长度必须大于等于e_str
	if(str.size() < e_str.size()) return result;

	size_t pos = str.rfind(e_str);
	if(pos == string::npos || pos != (str.size() - e_str.size())) {
		//没有找到字符串或者所在的位置不是0，表示不是以指定字符串开始
		result = false;
	}
	else {
		result = true;
	}

	return result;
}

/**
 * 将以指定字符(串)分隔的文字转化成vector动态数组
 */
vector<string> split(const string& str, const string& delim) {
	vector<string> res;

	if("" == str) return res;

	size_t str_len = str.size();
	size_t delim_len = delim.size();

	size_t old_pos = 0;
	size_t pos = str.find(delim);
	while(pos != string::npos) {
		//从上次查找完毕后的指针到这次发现的位置之间的字符串，就是要加入到vector中的内容
		string s = str.substr(old_pos, (pos - old_pos));
		res.push_back(s);

		//新的位置=发现分隔符的位置+分隔符长度
		if(pos + delim_len <= str_len) {
			old_pos = pos + delim_len;
			//继续查找下一个
			pos = str.find(delim, old_pos);
		}
		else {
			break;
		}
	}

	//字符串中如果没有查到分隔符，返回的是空的数组，但是最后一个分隔符后面是空的，还是要在最后增加一个空字符串
	if(old_pos > 0 && pos == string::npos) {
		//将结尾的所有字符串写入到数组最后
		string s = str.substr(old_pos);
		res.push_back(s);
	}

	return res;
}

/**
 * 求指定数组的校验和，校验和就是将byte数组的所有数据逐个异或后得到
 * 参数：
 * 		buf 指定的数组
 * 		len 参与校验和的byte长度
 * 返回值：
 * 		校验和
 */
uint8_t checksum(const uint8_t* buf, size_t len) {
	uint8_t check_sum = 0;

	for(size_t i=0; i<len; i++) {
		check_sum ^= buf[i];
	}

	return check_sum;
}

/**
 * 求指定字符串的校验和
 * 参数：
 * 		str string 需要求校验和的字符串
 * 参数：
 * 		求取的校验和,返回16进制文本
 */
string checksum(const string& str) {
	const uint8_t* buf = (const uint8_t*)str.c_str();
	uint8_t check_sum = checksum(buf, str.size());
	//sprintf输出会带一个\0，所以要多定义一个字节
	char s_checksum[3]{0};
	std::sprintf(s_checksum, "%02X", (unsigned char)check_sum);
	return string(s_checksum, 2);
}

//获取从纪元时间到现在的毫秒数
int64_t curentTimeMillis(void) {
	//获取从纪元时间到现在的duration
	chrono::system_clock::duration duration = chrono::system_clock::now().time_since_epoch();
	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(duration);

	return ms.count();
}

/**@brief 控制LED灯亮
* @param[in]  open				LED亮状态
* @retval		None
*/
void blueLedCtrl(bool open)
{
	fstream fs;
	string path("/sys/class/leds/LED1/brightness");
	fs.open(path.c_str(), fstream::out);
	fs << (open?1:0);
	fs.close();
}

/**@brief 控制LED灯亮闪周期
* @param[in]  duration				亮灭间隔
* @param[in]  times						持续时长
* @retval		None
*/
void led_lighting(int duration, int times)
{
	int consumetimes = 0;

	while(consumetimes < times) {
		blueLedCtrl(1);
		this_thread::sleep_for(std::chrono::milliseconds(duration));
		consumetimes += duration;
		if(consumetimes >= times)
			break;
		blueLedCtrl(0);
		this_thread::sleep_for(std::chrono::milliseconds(duration));
		consumetimes += duration;
	}
	blueLedCtrl(0);
}

enum TIME_DEF
{
    SEC = 1,
    MIN = SEC * 60,
    HOUR = MIN * 60,
    DAY = HOUR * 24,
    YEAR = DAY * 365,
};

time_t time_difference = 8 * HOUR;
static time_t mon_yday[2][12] =
{
    {0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

int leap(int year) {
    if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0)) {
        return 1;
    }
    return 0;
}
long long get_day(int year) {
    year = year - 1;
    int leap_year_num = year / 4 - year / 100 + year / 400;
    long long tol_day = year * 365 + leap_year_num;
    return tol_day;
}

time_t mymktime(int year,int mon,int day,int hour,int min,int sec) {
    long long tol_day = 0;
    year += 1900;
    tol_day = get_day(year) - get_day(1970);
    tol_day += mon_yday[leap(year)][mon];
    tol_day += day - 1;

    long long ret = 0;
    ret += tol_day * DAY;
    ret += hour * HOUR;
    ret += min * MIN;
    ret += sec * SEC;

    return ret + time_difference;
}

time_t mymktime(const string &timestr) {
	tm mk;

	strptime(timestr.data(), "%Y-%m-%d %H:%M:%S", &mk);

	if (!(mk.tm_sec>= 0 && mk.tm_sec <= 59)) {
	 return -1;
	}
	if (!(mk.tm_min>= 0 && mk.tm_min <= 59)) {
	 return -1;
	}
	if (!(mk.tm_hour>= 0 && mk.tm_hour <= 23)) {
	 return -1;
	}
	if (!(mk.tm_mday>= 1 && mk.tm_hour <= 31)) {
	 return -1;
	}
	if (!(mk.tm_mon>= 0 && mk.tm_mon <= 11)) {
	 return -1;
	}
	if (!(mk.tm_year>= 70)) {
	 return -1;
	}

	return mymktime(mk.tm_year,mk.tm_mon,mk.tm_mday,mk.tm_hour,mk.tm_min,mk.tm_sec);
}

}
