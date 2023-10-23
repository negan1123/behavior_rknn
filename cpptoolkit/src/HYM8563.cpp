#include "HYM8563.h"

/**
 * @brief 构造函数，打开设备
 * @param deviceName 设备地址
 */
HYM8563::HYM8563(const char * deviceName)
{
    client = new I2c(deviceName);
    client->addressSet(0x51, 1);
}

/**
 * @brief 析构函数，关闭设备
 */
HYM8563::~HYM8563()
{
    if(client)
    {
        delete client;
        client = NULL;
    } 
}

/**
 * @brief 初始化芯片。清空状态寄存器1的值；状态寄存器2取消警报和定时器中断，清空状态位
 * @return 返回状态寄存器2的值，初始化失败返回-1 
 */
int HYM8563::init()
{
    int ret;
	/* Clear stop flag if present */
	ret = client->writeByte(HYM8563_CTL1, 0);
	if (ret < 0)
		return ret;

	ret = client->readByte(HYM8563_CTL2);
	if (ret < 0)
		return ret;

	/* Disable alarm and timer interrupts */
	ret &= ~HYM8563_CTL2_AIE;
	ret &= ~HYM8563_CTL2_TIE;

	/* Clear any pending alarm and timer flags */
	if (ret & HYM8563_CTL2_AF)
		ret &= ~HYM8563_CTL2_AF;

	if (ret & HYM8563_CTL2_TF)
		ret &= ~HYM8563_CTL2_TF;

	ret &= ~HYM8563_CTL2_TI_TP;

	return client->writeByte(HYM8563_CTL2, ret);
}

/**
 * @brief 获取rtc时间
 * @param tm rtc时间
 * @return 获取成功返回0，失败返回-1
 */
int HYM8563::readTime(struct tm *tm)
{
	uint8_t buf[7];
	int ret;

	ret = client->readBlock(HYM8563_SEC, 7, buf);
    if (ret < 0)
		return ret;

	tm->tm_sec = bcd2bin(buf[0] & HYM8563_SEC_MASK);
	tm->tm_min = bcd2bin(buf[1] & HYM8563_MIN_MASK);
	tm->tm_hour = bcd2bin(buf[2] & HYM8563_HOUR_MASK);
	tm->tm_mday = bcd2bin(buf[3] & HYM8563_DAY_MASK);
	tm->tm_wday = bcd2bin(buf[4] & HYM8563_WEEKDAY_MASK); /* 0 = Sun */
	tm->tm_mon = bcd2bin(buf[5] & HYM8563_MONTH_MASK) - 1; /* 0 = Jan */
	tm->tm_year = bcd2bin(buf[6]) + 100;

	return 0;
}

/**
 * @brief 设置rtc时间
 * @param tm rtc时间
 * @return 设置成功返回0，失败返回-1
 */
int HYM8563::setTime(struct tm *tm)
{
    uint8_t buf[7];
	int ret;

	/* Years >= 2100 are to far in the future, 19XX is to early */
	if (tm->tm_year < 100 || tm->tm_year >= 200)
		return -1;

	buf[0] = bin2bcd(tm->tm_sec);
	buf[1] = bin2bcd(tm->tm_min);
	buf[2] = bin2bcd(tm->tm_hour);
	buf[3] = bin2bcd(tm->tm_mday);
	buf[4] = bin2bcd(tm->tm_wday);
	buf[5] = bin2bcd(tm->tm_mon + 1);

	/*
	 * While the HYM8563 has a century flag in the month register,
	 * it does not seem to carry it over a subsequent write/read.
	 * So we'll limit ourself to 100 years, starting at 2000 for now.
	 */
	buf[6] = bin2bcd(tm->tm_year - 100);

	/*
	 * CTL1 only contains TEST-mode bits apart from stop,
	 * so no need to read the value first
	 */
	ret = client->writeByte(HYM8563_CTL1, HYM8563_CTL1_STOP);
	if (ret < 0)
		return ret;

	ret = client->writeBlockData(HYM8563_SEC, 7, buf);
	if (ret < 0)
		return ret;

	ret = client->writeByte(HYM8563_CTL1, 0);
	if (ret < 0)
		return ret;

    return 0;
}

/**
 * @brief 获取报警中断状态位
 * @return 报警中断状态
 */
bool HYM8563::isAlarmIrqEnable()
{
	int ret = client->readByte(HYM8563_CTL2);
	return ret & HYM8563_CTL2_AIE;
}

/**
 * @brief 警报中断控制
 * @param enabled true 1 报警中断被使能；false 报警中断被禁止
 * @return 设置成功返回0，失败返回-1
 */
int HYM8563::alarmIrqEnable(bool enabled)
{
    int data;

	data = client->readByte(HYM8563_CTL2);
	if (data < 0)
		return data;

	if (enabled)
		data |= HYM8563_CTL2_AIE;
	else
		data &= ~HYM8563_CTL2_AIE;

	return client->writeByte(HYM8563_CTL2, data);
}

/**
 * @brief 获取警报时间 分钟、小时、日、星期有效
 * @param alm 报警时间
 * @return 获取成功返回0，失败返回-1
 */
int HYM8563::readAlarm(struct wkalrm *alm)
{
    struct tm *alm_tm = &alm->time;
	uint8_t buf[4];
	int ret;

	ret = client->readBlock(HYM8563_ALM_MIN, 4, buf);
	if (ret < 0)
		return ret;

	/* The alarm only has a minute accuracy */
	alm_tm->tm_sec = -1;

	alm_tm->tm_min = (buf[0] & HYM8563_ALM_BIT_DISABLE) ?
					-1 :
					bcd2bin(buf[0] & HYM8563_MIN_MASK);
	alm_tm->tm_hour = (buf[1] & HYM8563_ALM_BIT_DISABLE) ?
					-1 :
					bcd2bin(buf[1] & HYM8563_HOUR_MASK);
	alm_tm->tm_mday = (buf[2] & HYM8563_ALM_BIT_DISABLE) ?
					-1 :
					bcd2bin(buf[2] & HYM8563_DAY_MASK);
	alm_tm->tm_wday = (buf[3] & HYM8563_ALM_BIT_DISABLE) ?
					-1 :
					bcd2bin(buf[3] & HYM8563_WEEKDAY_MASK);

	alm_tm->tm_mon = -1;
	alm_tm->tm_year = -1;

	alm->minEnabled = true;
	alm->hourEnabled = true;
	alm->mdayEnabled = true;
	alm->wdayEnabled = true;

	if(alm_tm->tm_min == -1)
		alm->minEnabled = false;
	if(alm_tm->tm_hour == -1)
		alm->hourEnabled = false;
	if(alm_tm->tm_mday == -1)
		alm->mdayEnabled = false;
	if(alm_tm->tm_wday == -1)
		alm->wdayEnabled = false;

	return 0;
}

/**
 * @brief 设置警报时间 
 * @param alm 报警时间
 * @return 设置成功返回0，失败返回-1
 */
int HYM8563::setAlarm(struct wkalrm *alm)
{
    struct tm *alm_tm = &alm->time;
	uint8_t buf[4];
	int ret;

	/*
	 * The alarm has no seconds so deal with it
	 */
	if (alm_tm->tm_sec) {
		alm_tm->tm_sec = 0;
		alm_tm->tm_min++;
		if (alm_tm->tm_min >= 60) {
			alm_tm->tm_min = 0;
			alm_tm->tm_hour++;
			if (alm_tm->tm_hour >= 24) {
				alm_tm->tm_hour = 0;
				alm_tm->tm_mday++;
				if (alm_tm->tm_mday > 31)
					alm_tm->tm_mday = 0;
			}
		}
	}

	ret = client->readByte(HYM8563_CTL2);
	if (ret < 0)
		return ret;

	ret &= ~HYM8563_CTL2_AIE;

	ret = client->writeByte(HYM8563_CTL2, ret);
	if (ret < 0)
		return ret;

	buf[0] = (alm_tm->tm_min < 60 && alm_tm->tm_min >= 0) ?
			bin2bcd(alm_tm->tm_min) : HYM8563_ALM_BIT_DISABLE;

	buf[1] = (alm_tm->tm_hour < 24 && alm_tm->tm_hour >= 0) ?
			bin2bcd(alm_tm->tm_hour) : HYM8563_ALM_BIT_DISABLE;

	buf[2] = (alm_tm->tm_mday <= 31 && alm_tm->tm_mday >= 1) ?
			bin2bcd(alm_tm->tm_mday) : HYM8563_ALM_BIT_DISABLE;

	buf[3] = (alm_tm->tm_wday < 7 && alm_tm->tm_wday >= 0) ?
			bin2bcd(alm_tm->tm_wday) : HYM8563_ALM_BIT_DISABLE;

	// 分钟报警无效
	if(!alm->minEnabled)
	{
		buf[0] = HYM8563_ALM_BIT_DISABLE;
	}

	// 小时报警无效
	if(!alm->hourEnabled)
	{
		buf[1] = HYM8563_ALM_BIT_DISABLE;
	}

	// 日报警无效
	if(!alm->mdayEnabled)
	{
		buf[2] = HYM8563_ALM_BIT_DISABLE;
	}

	// 周报警无效
	if(!alm->wdayEnabled)
	{
		buf[3] = HYM8563_ALM_BIT_DISABLE;
	}

	ret = client->writeBlockData(HYM8563_ALM_MIN, 4, buf);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * @brief 获取定时器中断状态位
 * @return 定时器中断状态
 */
bool HYM8563::isTimerIrqEnable()
{
	int ret = client->readByte(HYM8563_CTL2);
	return ret & HYM8563_CTL2_TIE;
}

/**
 * @brief 定时器控制
 * @param enabled true 定时器中断被使能；false 定时器中断被禁止
  * @param frequency 定时器频率 默认频率为1
 * @return 设置成功返回0，失败返回-1
 */
int HYM8563::timerIrqEnable(bool enabled, uint8_t frequency)
{
    int data;
	int ret;

	// 设置定时器频率
    data = client->readByte(HYM8563_TMR_CTL);
	if(data < 0)
		return data;

	data &= ~HYM8563_TMR_CTL_MASK; // 清零频率
	data |= frequency; // 设置频率
    ret = client->writeByte(HYM8563_TMR_CTL, data);
    if (ret < 0)
		return ret;
	
	// 设置定时器中断
	data = client->readByte(HYM8563_CTL2);
	if (data < 0)
		return data;

	if (enabled)
		data |= HYM8563_CTL2_TIE;
	else
		data &= ~HYM8563_CTL2_TIE;

	ret = client->writeByte(HYM8563_CTL2, data);
	
	return ret;
}

/**
 * @brief 获取定时器时间
 * @return 返回倒计时寄存器的值
 */
uint8_t HYM8563::readTimer()
{
    return client->readByte(HYM8563_TMR_CNT);
}

/**
 * @brief 设置定时器
 * @param t 倒计时时间
 * @return 设置成功返回0，失败返回-1
 */
int HYM8563::setTimer(uint8_t t)
{
    int ret;
    //读取状态寄存器2的值
    ret = client->readByte(HYM8563_CTL2);
	if (ret < 0)
		return ret;

    //清空定时器控制位
	ret &= ~HYM8563_CTL2_TIE;
    ret = client->writeByte(HYM8563_CTL2, ret);
	if (ret < 0)
		return ret;

    //设置倒计时寄存器
    ret = client->writeByte(HYM8563_TMR_CNT, t);
    if (ret < 0)
		return ret;

    return 0;
}