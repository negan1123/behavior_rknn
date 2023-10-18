/*
 * V4L2Capture.h
 *
 *  Created on: 2021年1月6日
 *      Author: syukousen
 */

#ifndef _V4L2CAPTURE_H_
#define _V4L2CAPTURE_H_

#include <iostream>
#include <thread>
#include <atomic>
#include "Capture.h"

 /**@class CameraCapture
  * @brief 摄像头捕获数据，进行解析解压处理，输出算法可用的BGR图片
  * */
class CameraCapture : public Capture
{
public:
	/**@brief 构造函数，主要设置一下摄像头的名字，采集图像的长，高
	* @param[in]	devName		摄像头设备名
	* @param[in]	width		采集图像长
	* @param[in]	height		采集图像高
	* @param[in]	fmt			采集格式  0 V4L2_PIX_FMT_YUYV   1 V4L2_PIX_FMT_MJPEG
	* @return	None
	*/
	CameraCapture(const std::string &devName, int width = 1280, int height = 720, int fmt = 0, int interval = 500);

	/**@brief 析构函数，释放申请的资源
	* @param	None
	* @return	None
	*/
	virtual ~CameraCapture();

	/**@brief 初始化摄像头，设定摄像头的参数，输出格式，长高等参数
	* @param	None
	* @return	int 开始结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int init() override;

	/**@brief 开始采集图像
	* @param	f	采集好一帧图像后，回调函数
	* @return	int 停止结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int startCapture(std::function<void(cv::Mat& pic)> f) override;

	/**@brief 停止采集图像
	* @param	None
	* @return	int 打开摄像头结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int stopCapture() override;
private:
	/**@brief 打开摄像头
	* @param	None
	* @return	int 打开摄像头结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int openDevice();

	/**@brief 关闭摄像头
	* @param	None
	* @return	int 关闭摄像头结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int closeDevice();

	/**@brief 初始化摄像头输出buf
	* @param	None
	* @return	int 初始化摄像头输出buf结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int initBuffers();

	/**@brief 释放摄像头输出buf
	* @param	None
	* @return	int 释放摄像头输出buf结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int freeBuffers();

	/**@brief 获取一帧数据
	* @param[out]	data	数据起始地址
	* @param[out]	len		数据长度
	* @return	int 获取数据结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int getFrame(void **data,size_t *len);

	/**@brief 回退帧，让内核可以接着往map推送数据
	* @param	None
	* @return	int 回退帧结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int backFrame();
private:
	struct cam_buffer
	{
		void* start;					///< buf数据起始地址
		unsigned int length;			///< buf数据长度
	};
	std::string devName;				///< 摄像头设备名
	int capW;							///< 摄像头抓拍的长度
	int capH;							///< 摄像头抓拍的高度
	int fmt;							///< 摄像头抓拍输出图像格式 V4L2_PIX_FMT_YUYV或V4L2_PIX_FMT_MJPEG
	int interval;						///< 抓拍间隔
	int fd_cam;							///< 摄像机设备句柄
	cam_buffer *buffers;				///< 摄像机输出buf
	unsigned int n_buffers;				///< 摄像机输出buf个数
	int frameIndex;						///< 摄像机输出buf索引
	std::atomic<bool> capturing{false};			///< 是否正在抓图，不会发生线程冲突，线程安全
	std::atomic<bool> threadrunning{false};		///< 抓图线程是否结束
};


#endif /* _V4L2CAPTURE_H_ */
