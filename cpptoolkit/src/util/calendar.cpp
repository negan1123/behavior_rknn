/*
 * calendar.cpp
 *
 *  Created on: 2019年9月30日
 *      Author: william
 */

#include <exception>
#include <util/calendar.h>

namespace util {

//传入本地日期时间，构造Calendar，格式：yyyy-mm-dd hh:mm:ss.SSS
Calendar::Calendar(const string &date_time) {
	fromString(date_time);
}

//从时间点解析出Calendar需要的数据
bool Calendar::fromTimePoint(std::chrono::system_clock::time_point& tp) {
	try {
		this->tp = tp;

		std::chrono::system_clock::duration duration = tp.time_since_epoch();
		std::chrono::milliseconds ms = std::chrono::duration_cast<chrono::milliseconds>(duration);

		//保存从纪元时间开始到当前的毫秒数
		epoch_millisecond = ms.count();

		//获取今日的年月日、时分秒
		time_t tt = std::chrono::system_clock::to_time_t(tp);
		struct tm * timeinfo = localtime(&tt);
		//timeinfo中的年是参照1900
		this->year = timeinfo->tm_year + 1900;
		//月份从0-11，需要调整成1-12
		this->month = timeinfo->tm_mon + 1;
		this->day = timeinfo->tm_mday;
		this->hour = timeinfo->tm_hour;
		this->minute = timeinfo->tm_min;
		this->second = timeinfo->tm_sec;
		//当前毫秒数除以1000取余得到毫秒
		this->millisecond = (epoch_millisecond % 1000);
	}
	catch(std::exception& e) {
		//log_fatal(e.what());

		return false;
	}

	return true;
}

//从描述时间点的字符串解析数据到Calendar，格式：yyyy-mm-dd hh:mm:ss.SSS
//字符串表示的是本时区时间
bool Calendar::fromString(const string &str) {
	try {
		this->year = std::stoi(str.substr(0, 4));
		this->month = std::stoi(str.substr(5, 2));
		this->day = std::stoi(str.substr(8, 2));
		this->hour = std::stoi(str.substr(11, 2));
		this->minute = std::stoi(str.substr(14, 2));
		this->second = std::stoi(str.substr(17, 2));
		this->millisecond = std::stoi(str.substr(20));

		//更新从纪元时间至今的毫秒数、时间点
		refresh();
	}
	catch(std::exception& e) {
		//log_fatal(e.what());

		return false;
	}

	return true;
}

//根据UTC纪元开始的毫秒数解析信息到Calendar
bool Calendar::fromEpochMillisecond(int64_t millisecond) {
	//创建一个纪元时间
	std::chrono::system_clock::time_point tp;
	//得到ms的duration
	std::chrono::milliseconds tm_ms(millisecond);
	tp += tm_ms;

	//将时间点信息写入到Calendar
	return fromTimePoint(tp);
}

//将当前的信息写入到Calendar，并且返回UTC纪元开始的毫秒数
int64_t Calendar::now() {
	//获取当前时间点
	chrono::system_clock::time_point today = chrono::system_clock::now();

	//将当前时间解析到Calendar的各个数据域
	fromTimePoint(today);

	return this->epoch_millisecond;
}

//将Calener转成string
const string Calendar::toString() {
	stringstream ss;
	char s_item[5]{0};

	if(epoch_millisecond == 0) return "";

	sprintf(s_item, "%04d", (unsigned int)year);
	ss << string(s_item, 4) << "-";
	sprintf(s_item, "%02d", (unsigned char)month);
	ss << string(s_item, 2) << "-";
	sprintf(s_item, "%02d", (unsigned char)day);
	ss << string(s_item, 2) << " ";
	sprintf(s_item, "%02d", (unsigned char)hour);
	ss << string(s_item, 2) << ":";
	sprintf(s_item, "%02d", (unsigned char)minute);
	ss << string(s_item, 2) << ":";
	sprintf(s_item, "%02d", (unsigned char)second);
	ss << string(s_item, 2) << ".";
	sprintf(s_item, "%03d", (unsigned int)millisecond);
	ss << string(s_item, 3);

	return ss.str();
}

//输出UTC时间格式(只输出时间部分，和GPS定位的格式一致)，格式：hhmmss.SSS
const string Calendar::toUTCTime() {
	stringstream ss;
	char s_item[5]{0};

	if(epoch_millisecond == 0) return "";
	sprintf(s_item, "%02d", (unsigned char)hour);
	ss << string(s_item, 2);
	sprintf(s_item, "%02d", (unsigned char)minute);
	ss << string(s_item, 2);
	sprintf(s_item, "%02d", (unsigned char)second);
	ss << string(s_item, 2) << ".";
	sprintf(s_item, "%03d", (unsigned int)millisecond);
	ss << string(s_item, 3);

	return ss.str();
}

//计算UTC和本地时间相差的小时数，本地时间-UTC时间
int Calendar::diffUTCWithLocal() {
	time_t rawtime;
	//获取当前时间
	time(&rawtime);

	//分别得到本地时间和UTC时间，计算出小时差
	struct tm *ltm = localtime(&rawtime);
	int ltm_hour = ltm->tm_hour;
	struct tm *ptm = gmtime(&rawtime);
	int ptm_hour = ptm->tm_hour;
	return ltm_hour - ptm_hour;
}

//获取当前日历的时间点，有了时间点就可以调用c++11的api进行时间的比较、运算等
std::chrono::system_clock::time_point Calendar::getTimePoint() {
	return tp;
}

//当日期、时间等改变后，调用refresh后将更新自纪元时间的毫秒数、日历时间点信息
void Calendar::refresh() {
	struct tm timeinfo;
	timeinfo.tm_year = this->year - 1900;
	//将月份调整成0-11
	timeinfo.tm_mon = this->month - 1;
	timeinfo.tm_mday = this->day;
	//将本地时间的小时调整到UTC的小时
	timeinfo.tm_hour = this->hour;
	timeinfo.tm_min = this->minute ;
	timeinfo.tm_sec = this->second;
	timeinfo.tm_isdst = -1;

	//通过timeinfo构造time_point信息，由于不包含毫秒信息，我们需要增加毫秒到时间点
	time_t tt = mktime(&timeinfo);
	std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(tt);
	std::chrono::milliseconds tm_ms(this->millisecond);
	tp += tm_ms;
	this->tp = tp;

	std::chrono::system_clock::duration duration = tp.time_since_epoch();
	std::chrono::milliseconds ms = std::chrono::duration_cast<chrono::milliseconds>(duration);
	//保存从纪元时间开始到当前的毫秒数
	epoch_millisecond = ms.count();
}

int Calendar::getDay() const {
	return day;
}

int64_t Calendar::getEpochMillisecond() const {
	return epoch_millisecond;
}

int Calendar::getHour() const {
	return hour;
}

int Calendar::getMillisecond() const {
	return millisecond;
}

int Calendar::getMinute() const {
	return minute;
}

int Calendar::getMonth() const {
	return month;
}

int Calendar::getSecond() const {
	return second;
}

int Calendar::getYear() const {
	return year;
}

void Calendar::setDay(int day) {
	this->day = day;
}

void Calendar::setHour(int hour) {
	this->hour = hour;
}

void Calendar::setMillisecond(int millisecond) {
	this->millisecond = millisecond;
}

void Calendar::setMinute(int minute) {
	this->minute = minute;
}

void Calendar::setMonth(int month) {
	this->month = month;
}

void Calendar::setSecond(int second) {
	this->second = second;
}

void Calendar::setYear(int year) {
	this->year = year;
}

} /* namespace util */
