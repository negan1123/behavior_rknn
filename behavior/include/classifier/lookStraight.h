/*
 * lookStraight.h
 *
 *  Created on: Jan 12, 2022
 *      Author: wy
 */

#ifndef LOOKSTRAIGHT_H_
#define LOOKSTRAIGHT_H_

#include "behaviorClassify.h"

/**@class	LookLeftRight
 * @brief	长时间直视分类器
 */
class LookStraight: public BehaviorClassify
{
public:
	/**@brief	构造函数
	 * @param	None
	 * @return	None
	 */
	LookStraight();

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~LookStraight();

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
	float pitch_sum{0};				// 俯仰角总和
	int count{0};					// 检测出偏转角,俯仰角的图片数
//	time_t start{0};				// 第一次检测出偏转角,俯仰角的时间
	float yawAngle{0};				// 偏转角界限
	float pitchAngle{0};			// 俯仰角界限
};



#endif /* LOOKSTRAIGHT_H_ */
