/*
 * calendar.h
 *
 *  Created on: 2019年9月30日
 *      Author: william
 */

#ifndef SRC_UTIL_CALENDAR_H_
#define SRC_UTIL_CALENDAR_H_

#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace std;

namespace util {

class Calendar {
public:
	Calendar()=default;
	//传入本地日期时间，构造Calendar，格式：yyyy-mm-dd hh:mm:ss.SSS
	Calendar(const string &date_time);
	virtual ~Calendar()=default;

	//从时间点解析出Calendar,得到的是本地时区的数据
	//tp是utc时间
	bool fromTimePoint(std::chrono::system_clock::time_point& tp);
	//从描述时间点的字符串解析数据到Calendar，格式：yyyy-mm-dd hh:mm:ss.SSS
	//传入的是本地时间
	bool fromString(const string& str);
	//根据UTC纪元开始的毫秒数解析信息到Calendar
	//millisecond是utc时间
	bool fromEpochMillisecond(int64_t millisecond);
	//将当前的信息写入到Calendar，并且返回UTC纪元开始的毫秒数
	int64_t now();

	int getDay() const;
	//获取从纪元时间到现在的毫秒数(UTC,从1970-01-01 00:00:00开始)
	int64_t getEpochMillisecond() const;
	//得到的是本地时间小时
	int getHour() const;
	int getMillisecond() const;
	int getMinute() const;
	int getMonth() const;
	int getSecond() const;
	int getYear() const;
	void setDay(int day);
	//设置的是本地时间
	void setHour(int hour);
	void setMillisecond(int millisecond);
	void setMinute(int minute);
	void setMonth(int month);
	void setSecond(int second);
	void setYear(int year);

	//将Calener转成string
	const string toString();
	//输出UTC时间格式(只输出时间部分，和GPS定位的格式一致)，格式：hhmmss.SSS
	const string toUTCTime();
	//获取当前日历的时间点，有了时间点就可以调用c++11的api进行时间的比较、运算等
	std::chrono::system_clock::time_point getTimePoint();
	//当日期、时间等改变后，调用refresh后将更新自纪元时间的毫秒数、日历时间点信息
	void refresh();

public:
	//计算UTC和本地时间相差的小时数，本地时间-UTC时间
	static int diffUTCWithLocal();

private:
	//小时
	int hour{0};

	//分钟
	int minute{0};

	//秒
	int second{0};

	//毫秒
	int millisecond{0};

	//日，固定两位数字，取值范围 01~31
	int day{0};

	//月，固定两位数字，取值范围 01~12
	int month{0};

	//年，固定四位数字
	int year{0};

	//从纪元时间到当前日历的毫秒数
	int64_t epoch_millisecond{0};

	//calender对应的时间点
	std::chrono::system_clock::time_point tp;
};

} /* namespace util */

#endif /* SRC_UTIL_CALENDAR_H_ */
