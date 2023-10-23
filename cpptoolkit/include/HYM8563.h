#ifndef HYM8563_INCLUDE
#define HYM8563_INCLUDE

#include <i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/module.h>
#include <linux/rtc.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define HYM8563_CTL1		0x00
#define HYM8563_CTL1_TEST	0x80//BIT(7)
#define HYM8563_CTL1_STOP	0x20//BIT(5)
#define HYM8563_CTL1_TESTC	0x08//BIT(3)

#define HYM8563_CTL2		0x01
#define HYM8563_CTL2_TI_TP	0x10//BIT(4)
#define HYM8563_CTL2_AF		0x08//BIT(3)
#define HYM8563_CTL2_TF		0x04//BIT(2)
#define HYM8563_CTL2_AIE	0x02//BIT(1)
#define HYM8563_CTL2_TIE	0x01//BIT(0)

#define HYM8563_SEC		0x02
#define HYM8563_SEC_VL		0x80//BIT(7)
#define HYM8563_SEC_MASK	0x7f

#define HYM8563_MIN		0x03
#define HYM8563_MIN_MASK	0x7f

#define HYM8563_HOUR		0x04
#define HYM8563_HOUR_MASK	0x3f

#define HYM8563_DAY		0x05
#define HYM8563_DAY_MASK	0x3f

#define HYM8563_WEEKDAY		0x06
#define HYM8563_WEEKDAY_MASK	0x07

#define HYM8563_MONTH		0x07
#define HYM8563_MONTH_CENTURY	0x80//BIT(7)
#define HYM8563_MONTH_MASK	0x1f

#define HYM8563_YEAR		0x08

#define HYM8563_ALM_MIN		0x09
#define HYM8563_ALM_HOUR	0x0a
#define HYM8563_ALM_DAY		0x0b
#define HYM8563_ALM_WEEK	0x0c

/* Each alarm check can be disabled by setting this bit in the register */
#define HYM8563_ALM_BIT_DISABLE	0x80//BIT(7)

#define HYM8563_CLKOUT		0x0d
#define HYM8563_CLKOUT_ENABLE	0x80//BIT(7)
#define HYM8563_CLKOUT_32768	0
#define HYM8563_CLKOUT_1024	1
#define HYM8563_CLKOUT_32	2
#define HYM8563_CLKOUT_1	3
#define HYM8563_CLKOUT_MASK	3

#define HYM8563_TMR_CTL		0x0e
#define HYM8563_TMR_CTL_ENABLE	0x80//BIT(7)
#define HYM8563_TMR_CTL_4096	0
#define HYM8563_TMR_CTL_64	1
#define HYM8563_TMR_CTL_1	2
#define HYM8563_TMR_CTL_1_60	3
#define HYM8563_TMR_CTL_MASK	3

#define HYM8563_TMR_CNT		0x0f

/**
 * @brief BCD码转二进制
 * @param val BCD码
 * @return 转换后的二进制值
 */
static uint16_t bcd2bin(uint16_t val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

/**
 * @brief 二进制转BCD码
 * @param val 二进制
 * @return 转换后的BCD码
 */
static uint16_t bin2bcd(uint16_t val)
{
	return ((val / 10) << 4) + val % 10;
}


struct wkalrm {

    bool minEnabled; ///< 分钟报警 0 = 禁止alarm,1 = 使能alarm 

    bool hourEnabled; ///< 小时报警 0 = 禁止alarm,1 = 使能alarm 

    bool mdayEnabled; ///< 日报警 0 = 禁止alarm,1 = 使能alarm 

    bool wdayEnabled; ///< 星期报警 0 = 禁止alarm,1 = 使能alarm 

    struct tm time; // 设置的alarm中断发生的时刻 

};


/**
 * @brief 通过I2C总线操控HYM8563芯片
 */
class HYM8563
{
public:
    /**
     * @brief 构造函数，打开设备
     * @param deviceName 设备地址
     */
    HYM8563(const char * deviceName);

    /**
     * @brief 析构函数，关闭设备
     */
    ~HYM8563();

    /**
     * @brief 初始化芯片。清空状态寄存器1的值；状态寄存器2取消警报和定时器中断，清空状态位
     * @return 返回状态寄存器2的值，初始化失败返回-1 
     */
    int init();

    /**
     * @brief 获取rtc时间
     * @param tm rtc时间
     * @return 获取成功返回0，失败返回-1
     */
    int readTime(struct tm  *tm);

    /**
     * @brief 设置rtc时间
     * @param tm rtc时间
     * @return 设置成功返回0，失败返回-1
     */
    int setTime(struct tm  *tm);

    /**
     * @brief 获取报警中断状态位
     * @return 报警中断状态
     */
    bool isAlarmIrqEnable();
    /**
     * @brief 警报中断控制
     * @param enabled true 1 报警中断被使能；false 报警中断被禁止
     * @return 设置成功返回0，失败返回-1
     */
    int alarmIrqEnable(bool enabled);

    /**
     * @brief 获取警报时间 分钟、小时、日、星期有效
     * @param alm 报警时间
     * @return 获取成功返回0，失败返回-1
     */
    int readAlarm(struct wkalrm *alm);

    /**
     * @brief 设置警报时间 
     * @param alm 报警时间
     * @return 设置成功返回0，失败返回-1
     */
    int setAlarm(struct wkalrm *alm);

    /**
     * @brief 获取定时器中断状态位
     * @return 定时器中断状态
     */
    bool isTimerIrqEnable();

    /**
     * @brief 定时器控制
     * @param enabled true 定时器中断被使能；false 定时器中断被禁止
     * @param frequency 定时器频率 默认频率为1
     * @return 设置成功返回0，失败返回-1
     */
    int timerIrqEnable(bool enabled, uint8_t frequency = 0x82);

    /**
     * @brief 获取定时器时间
     * @return 返回倒计时寄存器的值
     */
    uint8_t readTimer();

    /**
     * @brief 设置定时器
     * @param t 倒计时时间
     * @return 设置成功返回0，失败返回-1
     */
    int setTimer(uint8_t t);

private:
    I2c* client;    ///< I2C设备
};

#endif