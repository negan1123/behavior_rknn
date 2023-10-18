/*
 * closeEye.h
 *
 *  Created on: Jan 10, 2022
 *      Author: wy
 */

#ifndef CLOSEEYE_H_
#define CLOSEEYE_H_

#include "behaviorClassify.h"

/**@class	CloseEye
 * @brief	频繁闭眼分类器
 */
class CloseEye: public BehaviorClassify
{
public:
	/**@brief	构造函数
	 * @param	None
	 * @return	None
	 */
	CloseEye();

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~CloseEye();

	/**@brief	推理函数
	 * @param1	time	推理时间
	 * @param2	img		待推理图片
	 * @return	None
	 */
	virtual void inference(const time_t& time, const cv::Mat& img);
private:
	std::vector<Record> records;	// 用一个vector来存储符合条件的闭眼图片
};




#endif /* CLOSEEYE_H_ */
