#ifndef I2C_H
#define I2C_H

#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/types.h>
#include <unistd.h>
#include <iostream>

/**
 * @brief 使用此类实现设备驱动 需要的库 i2c-tools libi2c-dev
 * @section
 * 使用背景：
 * 在IIC总线系统中，寻址字节由七位地址位(它占据了D7-D1位)和一位方向位(为D0位)组成。
 * 方向位为0时表示主控器将数据写入被控器，为 1时表示主控器从被控器读取数据。
 * 以HYM8563为例进行介绍
 * HYM8563 I2C总线从地址：读，0A3H；写，0A2H
 * 使用的基本流程：
 * I2c * bus = new I2c("/dev/i2c-4");//打开设备
 * bus->addressSet(0x51, 1);//设置I2C设备的从机地址
 * 使用注意事项：
 * （1）使用时将地址右移一位才是真正的读取地址 0A3H >> 1 = 51H
 * （2）当某个i2c设备地址已经关联了某个内核driver时，再用I2C_SLAVE作为ioctl的flag就无法取得该设备的控制权了。
 *  这时，应该使用I2C_SLAVE_FORCE，即绑定地址时使用addressSet函数 需要将force设置成1
 * （3）读写前必须先设置I2C设备的从机地址
 * （4）编译时需要链接i2c 例: g++ main.cpp -li2c
 */
class I2c {
public:
		/**
		 * @brief 构造函数，打开设备
		 * @param deviceName 设备地址
		 */
		I2c(const char * deviceName);

		/**
		 * @brief 析构函数，关闭设备
		 */
		~I2c();

		/**
		 * @brief 打开设备
		 * @param deviceName 设备地址
		 */
		void setup(const char * deviceName);

		/**
		 * @brief 设置从机地址
		 * @param address 从机地址
		 * @param force 1 强制设置I2C设备的从机地址 默认为0不强制
		 * @return 成功返回1 失败返回-1
		 */
		int addressSet(uint8_t address,int force = 0);

		/**
		 *@brief 写入1字节数据，一般是写入寄存器的偏移地址以便后续的读写
		 *@param command 写入的数据
		 *@return 成功返回1 失败返回-1
		 */
		int write(uint8_t command);

		/**
		 * @brief 向寄存器写入数据 1字节
		 * @param command 寄存器的偏移地址
		 * @param data 写入的数据
		 * @return 成功返回1 失败返回-1
		 */
		int writeByte(uint8_t command, uint8_t data);

		/**
		 * @brief 向寄存器写入数据
		 * @param command 寄存器的偏移地址
		 * @param size 数据大小
		 * @param data 写入的数据数组
		 * @return 成功返回1 失败返回-1
		 */
		int writeBlockData(uint8_t command, uint8_t size, __u8 * data);

		/**
		 *@brief 读取1字节数据
		 *@param command 读取的数据
		 *@return 返回读取的数据 读取失败返回-1
		 */
		uint16_t readByte(uint8_t command);

		/**
		 * @brief 读取数据块
		 * @param command 寄存器的偏移地址
		 * @param size 读取数据的大小
		 * @param data 存放读取的数据
		 * @return 返回读取数据的大小 读取失败返回-1
		 */
		uint16_t readBlock(uint8_t command, uint8_t size, uint8_t * data);

		/**
		 * @brief 设置从机地址并进行数据读取
		 * @param address 从机地址
		 * @param command 寄存器的偏移地址
		 * @return 返回读取的数据 读取失败返回-1
		 */
		uint16_t tryReadByte(uint8_t address, uint8_t command);
private:
		int fd;				///< 文件描述符
};

#endif
