/*
 * Capture.h
 *
 *  Created on: 2021年1月12日
 *      Author: syukousen
 */

#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <functional>
#include <opencv2/core.hpp>

 /**@class Capture
  * @brief 抓取图片的基类
  * */
class Capture
{
public:
	/**@brief 构造函数
	* @param	None
	* @return	None
	*/
	Capture(){}

	/**@brief 析构函数，释放申请的资源
	* @param	None
	* @return	None
	*/
	virtual ~Capture(){}

	/**@brief 初始化
	* @param	None
	* @return	int 初始化结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	virtual  int init() = 0;

	/**@brief 开始采集图像
	* @param	f	采集好一帧图像后，回调函数
	* @return	int 开始采集结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	virtual int startCapture(std::function<void(cv::Mat& pic)> f) = 0;

	/**@brief 停止采集图像
	* @param	None
	* @return	int 停止结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	virtual int stopCapture() = 0;

protected:
	/**@brief 通过摄像头抓拍，得到一个图像的回调函数(事件)
	* @param[in]	pic  抓拍处理好的BGR图片，可供算法使用
	* @return	None
	*/
	std::function<void(cv::Mat& pic)> onCapFrame{nullptr};
};
#endif /* _CAPTURE_H_ */
