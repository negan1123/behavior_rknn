/*
 * behaviorClassify.h
 *
 *  Created on: Jan 11, 2022
 *      Author: wy
 */

#ifndef BEHAVIORCLASSIFY_H_
#define BEHAVIORCLASSIFY_H_

#include <functional>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <BehaviorConf.hpp>

using onDetect = std::function<void(const std::string& pic_path)>;

typedef struct
{
	time_t time;
	cv::Mat img;
}Record;

/**@class	BehaviorClassify
 * @brief	行为分类基类
 * @details	目前派生出三种分类器，频繁眨眼，左顾右盼，和长时间直视
 */
class BehaviorClassify
{
public:
	/**@brief	构造函数
	 * @param	None
	 * @return	None
	 */
	BehaviorClassify(){};

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~BehaviorClassify(){};

	/**@brief	推理函数1
	 * @param1	time	推理时间
	 * @param2	img		待推理图片
	 * @return	None
	 */
	virtual void inference(const time_t& time, const cv::Mat& img){};

	/**@brief	推理函数2
	 * @param1	time	推理时间
	 * @param2	img		待推理图片
	 * @param3	yaw		偏转角
	 * @param4	pitch	俯仰角
	 * @return	None
	 */
	virtual void inference(const time_t& time, const cv::Mat& img, const float& yaw, const float& pitch){};

	/**@brief	设置回调函数
	 * @param	_cb	回调函数
	 * @return	None
	 */
	void setOnDetect(onDetect _cb){cb = _cb;};
protected:
	onDetect cb{nullptr};	// 回调函数指针
	int duration{0};		// 单次推理持续时长
	int frequency{0};		// 单次预警需要检测出违规行为的次数
};



#endif /* BEHAVIORCLASSIFY_H_ */
