#include <fcntl.h>
extern"C" {
	#include <linux/i2c.h>
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
}
#include "i2c.h"

/**
 * @brief 构造函数，打开设备
 * @param deviceName 设备地址
 */
I2c::I2c(const char * deviceName)
{
	fd = open(deviceName, O_RDWR);
	if (fd == -1)
	{
		std::cout<<"Failed to open I2c device!"<<std::endl;
		
	}
}

/**
 * @brief 析构函数，关闭设备
 */
I2c::~I2c()
{
	close(fd);
}

/**
 * @brief 打开设备
 * @param deviceName 设备地址
 */
void I2c::setup(const char * deviceName)
{
	fd = open(deviceName, O_RDWR);
	if (fd == -1)
	{
		std::cout<<"Failed to open I2c device!"<<std::endl;
	}
}

/**
 * @brief 设置从机地址
 * @param address 从机地址
 * @param force 1 强制设置I2C设备的从机地址 默认为0不强制
 * @return 成功返回1 失败返回-1
 */
int I2c::addressSet(uint8_t address,int force)
{
	int result = force ? ioctl(fd, I2C_SLAVE_FORCE, address) : ioctl(fd, I2C_SLAVE, address);
	if (result == -1)
	{
		std::cout<<"Failed to set address."<<std::endl;
		return -1;
	}
	return 1;
}

/**
 *@brief 写入1字节数据，一般是写入寄存器的偏移地址以便后续的读写
 *@param command 写入的数据
 *@return 成功返回1 失败返回-1
 */
int I2c::write(uint8_t command)
{
	int result = i2c_smbus_write_byte(fd, command);
	if (result == -1)
	{
        std::cout<<"Failed to write byte to I2c."<<std::endl;
		return -1;
	}
	return 1;
}

/**
 * @brief 向寄存器写入数据 1字节
 * @param command 寄存器的偏移地址
 * @param data 写入的数据
 * @return 成功返回1 失败返回-1
 */
int I2c::writeByte(uint8_t command, uint8_t data)
{
	int result = i2c_smbus_write_byte_data(fd, command, data);
	if (result == -1)
	{
        std::cout<<"Failed to write byte to I2c."<<std::endl;
		return -1;
	}
	return 1;
}

/**
 * @brief 向寄存器写入数据
 * @param command 寄存器的偏移地址
 * @param size 数据大小
 * @param data 写入的数据数组
 * @return 成功返回1 失败返回-1
 */
int I2c::writeBlockData(uint8_t command, uint8_t size, __u8 * data)
{
	int result = i2c_smbus_write_i2c_block_data(fd, command, size, data);
	if (result == -1)
	{
        std::cout<<"Failed to write block data byte to I2c."<<std::endl;
		return -1;
	}
	return 1;
}

/**
 *@brief 读取1字节数据
 *@param command 读取的数据
 *@return 返回读取的数据 读取失败返回-1
 */
uint16_t I2c::readByte(uint8_t command)
{
	int result = i2c_smbus_read_byte_data(fd, command);
	if (result == -1)
	{
        std::cout<<"Failed to read byte from I2c."<<std::endl;
	}
	return result;
}

/**
 * @brief 读取数据块
 * @param command 寄存器的偏移地址
 * @param size 读取数据的大小
 * @param data 存放读取的数据
 * @return 返回读取数据的大小 读取失败返回-1
 */
uint16_t I2c::readBlock(uint8_t command, uint8_t size, uint8_t * data)
{
	int result = i2c_smbus_read_i2c_block_data(fd, command, size, data);
	if (result != size)
	{
        std::cout<<"Failed to read block from I2c."<<std::endl;
	}
	return result;
}

/**
 * @brief 设置从机地址并进行数据读取
 * @param address 从机地址
 * @param command 寄存器的偏移地址
 * @return 返回读取的数据 读取失败返回-1
 */
uint16_t I2c::tryReadByte(uint8_t address, uint8_t command) {
	addressSet(address);
	return readByte(command);
}
