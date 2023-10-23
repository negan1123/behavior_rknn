/*
 * RtspCapture.h
 *
 *  Created on: 2021年1月12日
 *      Author: syukousen
 */

#ifndef _rtsp_capture_h_
#define _rtsp_capture_h_

#include <iostream>
#include <thread>
#include <atomic>
#include <queue>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
extern "C" 
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <Capture.h>
#include <H264FFmpegDecoder.h>

#include <unistd.h>
#include <RtspVideoCircleRecorder.h>


using namespace std;

/**@class RtspCapture
 * @brief 通过rtsp服务器获取图像，供算法判断行为
 * */
class RtspCapture : public Capture
{
public:
	/**@brief 构造函数，主要设置一下Rtsp路径的名字，采集图像的长，高
	* @param[in]	file		Rtsp路径的名字
	* @param[in]	width		采集图像长
	* @param[in]	height		采集图像高
	* @param[in]	interval	采集间隔
	* @return	None
	*/
	RtspCapture(const std::string &file, int width = 1280, int height = 720, int interval = 500);

	/**@brief 析构函数，释放申请的资源
	* @param	None
	* @return	None
	*/
	~RtspCapture();

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
	* @return	int 停止采集结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int stopCapture() override;
private:
	/**@brief 获取视频封装文件中视频流的序号
	* @param	None
	* @return	int 视频流在视频封装中的序号，-1表示没有发现视频流
	*/
	int getVideoIndex();

	/**@brief 从封装文件中读取一个压缩数据包，注意读取到的数据包需要调用freePacket进行资源释放
	* @param[out]	packet	视频读取视频帧
	* @return	bool 读取结果
	* 			true 表示成功
	* 			false 表示失败
	*/
	bool readPacket(AVPacket &packet);

	/**@brief 释放读取到的数据包关联资源，当使用完了AVPacket后必须调用freePacket()
	* @param[in]	packet	视频读取视频帧
	* @return	None
	*/
	void freePacket(AVPacket &packet);

	/**@brief 打开指定码流对应的解码器，打开成功后pCodecCtx指向对应的解码器上下文
	* @param[in]	streamIndex 码流序号
	* @param[out]	pCodecCtx AVCodecContext对象，解码器上下文
	* @return	bool 打开解码器结果
	* 			true 打开解码器成功
	* 			false 打开解码器失败
	*/
	bool openCodec(int streamIndex, AVCodecContext* &pCodecCtx);

	/**@brief 关闭解码器
	* @param	None
	* @return	None
	*/
	void closeCodec();

private:
	uint16_t width;			///< 解码输出的视频宽度
	uint16_t height;		///< 解码输出的视频高度
	AVPixelFormat pixFmt;	///< 解码输出的视频色彩空间
	int frameRate;			///< 视频帧率
	int videoIndex;			///< 视频流序号
	AVFormatContext *pFormatCtx{nullptr};	///< ffmpeg输入上下文

	AVCodecContext* pCodecCtx{nullptr};		///< 解码器指针
	AVFrame* pFrame{nullptr};				///< 视频帧图像
	AVFrame* pFrameOut{nullptr};			///< 转成RGB的帧图像
	int numBytes{0};						///< 图像转换使用的缓冲区的大小
	uint8_t* buffer{nullptr};				///< 图像转换使用的缓冲区
	struct SwsContext* sws_ctx{nullptr};	///< 图像转换的上下文
	std::string inAvFile;					///< 输入文件，可以是一个具体文件或者网络访问地址url
	std::atomic<bool> threadrunning{false};		///< 抓图线程是否结束
	std::atomic<bool> decoding{false};		///< 是否正在解码，不会发生线程冲突，线程安全
	bool isInited{false};
	Decoder *decoder{nullptr};						///< 解码器
	std::queue<cv::Mat>yuvQueue;			///< 解码后的YUV图像队列
	std::mutex yuvMtx;						///< YUV图像队列控制信号
	std::queue<AVPacket>packetQueue;		///< AVPacket缓冲队列
	std::mutex packetMtx;					///< AVPacket缓冲队列控制信号

    bool isDecodeIdr = true;
    bool isMppEnable = false;
};
#endif /* _rtsp_capture_h_ */
