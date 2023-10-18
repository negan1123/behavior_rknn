/*
 * DriverBehaviorService.h
 *
 *  Created on: 2021年3月15日
 *      Author: syukousen
 */

#ifndef INCLUDE_DRIVERBEHAVIORSERVICE_H_
#define INCLUDE_DRIVERBEHAVIORSERVICE_H_

#include "RetinaDriver.h"
#include "UploadBehavior.h"
#include "localBus.h"
#include <classifier/closeEye.h>
#include <classifier/lookLeftRight.h>
#include <classifier/lookStraight.h>
#include <atomic>

#define	BEHAVIOR_TYPE_MAX		(8)			///< 总共检测驾驶员行为的数量

/*
 * @brief 图片队列,记录入队时间
 */
struct picQueue
{
	time_t enTime;///< 入队时间
	cv::Mat img; ///< 待检测图片

	picQueue(){}

	/**
	 * @brief 深拷贝图片
	 */
	void setImg(cv::Mat& img)
	{
		this->img = img.clone();
	}

	/**
	* @brief 成员拷贝
	*/
	void set(picQueue* p1, picQueue* p2)
	{
		p1->enTime = p2->enTime;
		p1->img = p2->img.clone();
	}

	/**
	* @brief 复制构造函数
	*/
	picQueue(const picQueue& p)
	{
		//printf("picQueue 复制构造函数\n");
		*this = p;
	}

	/**
	* @beief 重载运算符
	*/
	picQueue& operator=(const picQueue& p)
	{
		//printf("picQueue 重载运算符\n");
		set(this, (picQueue*)&p);
		return (*this);
	}
};

/**@class DriverBehaviorService
  * @brief 驾驶员驾驶行为检测服务，主要实现输入要检测的图形进入检测队列，然后内部线程循环检测图像
  * */
class DriverBehaviorService
{
public:
	/**@brief 构造函数，初始化RetinaDriver，UploadBehavior两个对象
	* @param	None
	* @return	None
	*/
	DriverBehaviorService();

	/**@brief 输入一张要检查的图像到检查队列
	* @param	detect		检查的图形
	* @return	None
	*/
	void enqueueMat(cv::Mat & detect);

	/**@brief 开始驾驶行为检查
	* @param	None
	* @return	None
	*/
	void start();

	void testpic(std::string path);

	/**@brief 获取rknnApi版本
	* @param	None
	* @return	None
	*/
	std::string getRknnApiVer();

	/**@brief 获取rknnDrv版本
	* @param	None
	* @return	None
	*/
	std::string getRknnDrvVer();
private:
	std::queue<struct picQueue> detectMatQueue;				///< 等待rknn推理的图像队列
	std::mutex mtx;									///< detectMatQueue队列锁
	std::unique_ptr<RetinaDriver> mRetinaDriver;	///< 驾驶行为检查实例
	std::unique_ptr<UploadBehavior> mUploadBehavior;	///< 驾驶行为上传服务器实例
	std::unique_ptr<CloseEye> ce;					// 闭眼判断判断实例
	std::unique_ptr<LookLeftRight> llr;				// 左顾右盼判断实例
	std::unique_ptr<LookStraight> ls;				// 长时间直视判断实例
	std::unique_ptr<LocalBus> localBus;				// 内部总线实例
	std::atomic<float> speed {0.0};									// 当前车速
	int thresholdOne {0};							// 速度阈值1，大于此值才检测闭眼
	int thresholdTwo {0};							// 速度阈值2，大于此值才检测长时间直视与左顾右盼
};

#endif /* INCLUDE_DRIVERBEHAVIORSERVICE_H_ */
