/*
 * led.h
 *  设备的指示灯管理，主要管理WORK_LED、DIY_LED，控制相应的LED闪烁
 *  Created on: 2022年9月1日
 *      Author: william
 */

#ifndef _LED_H_
#define _LED_H_

/**
 * ALARM_LED指示灯的IO口编号(最下红灯)
 */
#define ALARM_LED 122

/**
 * WORK_LED指示灯的IO口编号(最下绿灯)
 */
#define WORK_LED 154

/**
 * GNSS_LED指示灯的IO口编号(中间绿灯，与定位模组共用一个led)
 */
#define GNSS_LED 149

#include <iostream>
#include <thread>
#include <gpio.h>

class Led {
public:
	/**
	 * @brief 构造函数
	 */
	Led() {
		init(false);		
	};

	~Led()=default;

	/**
	 * @brief 对LED进行初始化
	 * @param initFlag true初始化LED对应的GPIO，false只设置gpio编号，不错初始化
	 */
	void init(bool flag=true) {
		if(flag) {
			// 需要进行初始化，GPIO是共享的，整个程序只需要做一次初始化
			alarmLed.setup(ALARM_LED, ofx::OUT, ofx::LOW);
			gnssLed.setup(GNSS_LED, ofx::OUT, ofx::LOW);
			workLed.setup(WORK_LED, ofx::OUT, ofx::LOW);
		}
		else {
			// 仅设置恒gnssLed对应管脚号
			alarmLed.setup(ALARM_LED);
			gnssLed.setup(GNSS_LED);
			workLed.setup(WORK_LED);
		}
	};

	/**
	 * @brief WORK_LED闪烁(绿灯)
	 */
	void workLedBlink()  {
		std::thread blink([&]{
			// 先灭100ms
			workLed.set(ofx::LOW);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			// 亮300ms
			workLed.set(ofx::HIGH);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			// 熄灭
			workLed.set(ofx::LOW);
		});
		blink.detach();
	};

	/**
	 * @brief 点亮WORK_LED
	 */
	inline void workLedOn() {
		workLed.set(ofx::HIGH);
	};

	/**
	 * @brief 关闭WORK_LED
	 */
	inline void workLedOff() {
		workLed.set(ofx::LOW);
	};

	/**
	 * @brief DIY_LED闪烁
	 */
	void diyLedBlink() {
		std::thread blink([&]{
			// 先灭100ms
			alarmLed.set(ofx::LOW);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			// 亮300ms
			alarmLed.set(ofx::HIGH);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			// 熄灭
			alarmLed.set(ofx::LOW);
		});
		blink.detach();
	};

	/**
	 * @brief 点亮DIY_LED
	 */
	inline void diyLedOn() {
		alarmLed.set(ofx::HIGH);
	};

	/**
	 * @brief 关闭DIY_LED
	 */
	inline void diyLedOff() {
		alarmLed.set(ofx::LOW);
	};

	/**
	 * @brief GNSS_LED闪烁
	 */
	void gnssBlink() {
		std::thread blink([&]{
			// 先灭100ms
			gnssLed.set(ofx::LOW);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			// 亮300ms
			gnssLed.set(ofx::HIGH);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			// 熄灭
			gnssLed.set(ofx::LOW);
		});
		blink.detach();
	};

	/**
	 * @brief 点亮GNSS_LED
	 */
	inline void gnssLedOn() {
		gnssLed.set(ofx::HIGH);
	};

	/**
	 * @brief 关闭GNSS_LED
	 */
	inline void gnssLedOff() {
		gnssLed.set(ofx::LOW);
	};

private:
	ofx::GPIO alarmLed;
	ofx::GPIO gnssLed;
	ofx::GPIO workLed;
};

#endif /* _LED_H_ */

