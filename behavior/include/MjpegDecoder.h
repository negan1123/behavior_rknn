/*
 * MjpegDecoder.h
 *
 *  Created on: 2021年1月7日
 *      Author: syukousen
 */

#ifndef _MJPEGDECODER_H_
#define _MJPEGDECODER_H_
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <rockchip/rk_mpi.h>
#include <rockchip/rk_type.h>
#include <rockchip/mpp_err.h>

 /**@class MjpegDecoder
  * @brief Mjpeg解码为BGR，支持opencv软解码,和mpp硬解码
  * */
class MjpegDecoder
{
public:
	/**@brief 构造函数
	* @param[in]	mppdecode	是否使用mpp解码
	* @return	None
	*/
	MjpegDecoder(bool mppdecode = false);

	/**@brief 析构函数，释放申请的资源
	* @param	None
	* @return	None
	*/
	~MjpegDecoder();

	/**@brief 初始化mpp解码器
	* @param	None
	* @return	int 初始化结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int init();

	/**@brief mjpeg解码
	* @param[in]	jpeg	jpeg数据
	* @param[out]	bgr		解码后的数据
	* @return	int 解码结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int decode(const std::vector<char>& jpeg, cv::Mat& bgr);

private:
    MppCtx ctx{nullptr};		///< 解码器句柄
    MppApi *mpi{nullptr};		///< 解码器实例化对象指针
    MppPacket packet{nullptr};	///< 解码器输入的包结构
    MppFrame  frame{nullptr};	///< 解码器输出的帧结构
    MppBufferGroup  frm_grp{nullptr};	///<解码器输出帧数组
    MppBufferGroup  pkt_grp{nullptr};	///<解码器输入包数组
    MppBuffer pkt_buf{nullptr};			///<解码器输入数据buf
    MppBuffer frm_buf{nullptr};			///<解码器输出数据buf

    bool			mppdecode;			///< 是否使用mpp解码
};

#endif /* _MJPEGDECODER_H_ */
