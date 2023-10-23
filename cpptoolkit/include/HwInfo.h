/*
 * HwInfo.h
 *
 *  Created on: Sep 23, 2021
 *      Author: wy
 */

#ifndef HWINFO_H_
#define HWINFO_H_

#include <string>
#include <config.h>

/**@namespace	util
 * @brief		管理器空间
 */
namespace util
{

#ifndef SALT1
#define SALT1 "bjbn@bjbn"	///< 第一份盐
#endif	//SALT1
#ifndef SALT2
#define SALT2 "VAI"		///< 第二份盐
#endif	//SALT2

	/**@class	HwInfo
	 * @brief	硬件指纹管理器
	 * @details	获取cpu串口号，网卡MAC地址，使用MD5加密算法，base64编码加密，生成硬件指纹，并且能够判断输入的指纹是否正确
	 */
class HwInfo
{
public:

	/**@brief 	构造函数
	* @param	None
	* @return	None
	*/
	HwInfo();

	/**@brief 	析构函数
	* @param	None
	* @return	None
	*/
	~HwInfo();

	/**@brief 	初始化函数
	*@details	读取设备的CPUID和MAC地址，分别对CPUID和MAC地址用MD5算法加密，然后分别加盐，
	* 			再组成一个新的字符串，再一次进行MD5加密，最后对加密的结果进行base64编码，生成指纹
	* @param	None
	* @return	bool	初始化结果
	* 	@retval	true	初始化成功
	* 	@retval	false	初始化失败
	*/
	bool init();

	/**@brief 	获取CUPID
	* @param	None
	* @return	string 	返回CUPID
	*/
	std::string getCPUID();

	/**@brief 	获取网卡MAC地址
	* @param	None
	* @return	string	返回MAC地址
	*/
	std::string getMac();

	/**@brief 	获取硬件指纹
	* @param	None
	* @return	string	返回硬件指纹
	*/
	std::string getFinger();

	/**@brief 	生成的硬件指纹
	* @param	id		输入的cpuid
	* 			mac		输入的MAC地址
	* @return	string	返回生成的硬件指纹
	*/
	std::string genarateFinger(const std::string &id, const std::string &mac);

	/**@brief 	检测输入的指纹是否正确
	* @param	check	输入的待检测的指纹
	* @return	bool	检测结果
	* 	@retval	true	指纹正确
	* 	@retval	false	指纹错误或者获取CPUID，Mac失败
	*/
	bool verifyFinger(const std::string &check);

private:
	std::string cpuID;		///<CPU的串口号
	std::string mac;		///<网卡的MAC地址
	std::string finger;		///<硬件指纹
	std::string salt1;		///<加密算法使用的盐
	std::string salt2;		///<加密算法使用的盐
};
}

#endif /* HWINFO_H_ */
