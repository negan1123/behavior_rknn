/*
 * lookLeftRight.h
 *
 *  Created on: Jan 12, 2022
 *      Author: wy
 */

#ifndef LOOKLEFTRIGHT_H_
#define LOOKLEFTRIGHT_H_

#include <classify/BehaviorClassify.h>

#define INTERVAL 15			// 检测间隔

/**@class	lookLeftRight
 * @brief	左顾右盼分类器
 */
class lookLeftRight: public BehaviorClassify
{
public:
	/**@brief	构造函数
	 * @param	None
	 * @return	None
	 */
	lookLeftRight();

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~lookLeftRight();

	/**@brief	推理函数
	 * @param1	time	推理时间
	 * @param2	img		待推理图片
	 * @param3	yaw		偏转角
	 * @param4	pitch	俯仰角
	 * @return	None
	 */
	virtual void inference(const time_t& time, const cv::Mat& img, const float& yaw, const float& pitch);
private:
	std::vector<Record> records;	// 用一个vector来存储符合条件的图片
	float yaw_sum{0};				// 偏转角总和
	int count{0};					// 检测出偏转角的图片数
//	time_t start{0};				// 第一次检测出偏转角的时间
	int l_interval{0};				// 检测间隔计数器
	float yawAngle{0};				// 偏转角界限，差值超过该值认定为一次左顾右盼
};



#endif /* LOOKLEFTRIGHT_H_ */
