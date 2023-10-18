/*
 * RtspCapture.cpp
 *
 *  Created on: 2021年1月12日
 *      Author: syukousen
 */

#include "RtspCapture.h"
#include "RtspVideoCircleRecorder.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

RtspCapture::RtspCapture(const std::string &file, int width, int height, int interval)
{
	inAvFile = file;
	this->width = width;
	this->height = height;
	pixFmt = AV_PIX_FMT_BGR24;
	frameRate = interval;
	videoIndex = -1;

	//启动一个出队线程，将解码的数据送至检测程序
	std::thread popImgThread([&](){
		while(1)
		{
			yuvMtx.lock();
			//每5张返回一次解码数据
			if(yuvQueue.size() >= 5)
			{
				if(onCapFrame)
					onCapFrame(yuvQueue.front());
				//弹出5张图片
				for(int i = 0;i < 5;++i)
					yuvQueue.pop();
			}
			yuvMtx.unlock();

			usleep(5000);
		}
	});
	popImgThread.detach();
}

RtspCapture::~RtspCapture() {
	// 关闭输入文件，释放相应的资源
	stopCapture();
}

int RtspCapture::init()
{
	int ret;
	//环境注册，注册编解码器、各种通信协议
//	高版本已经弃用av_register_all和avcodec_register_all
//	av_register_all();
//	avcodec_register_all();
	avformat_network_init();

#if 0	// 一些Rtsp不能设置
	AVDictionary* options = NULL;
	av_dict_set(&options, "rtsp_transport", "tcp", 0);
	if ((ret = avformat_open_input(&pFormatCtx, inAvFile.c_str(), 0, &options)) < 0) {
		std::cout << "Could not open input file:" << inAvFile << std::endl;
		return -1;
	}
#else

	AVDictionary* options = NULL;
	av_dict_set(&options, "rtsp_transport", "tcp", 0);
	av_dict_set(&options, "stimeout", "10000000", 0);//设置超时10秒,一定要设置，不然后面的avformat_find_stream_info，readPacket可能不会退出
	if ((ret = avformat_open_input(&pFormatCtx, inAvFile.c_str(), 0, &options)) < 0) {
		std::cout << "Could not open input file:" << inAvFile << std::endl;
		av_dict_free(&options);
		return -1;
	}
	av_dict_free(&options);
#endif
	// 读取输入流的基本信息到pFormatCtx中
	if ((ret = avformat_find_stream_info(pFormatCtx, 0)) < 0) {
		if(pFormatCtx) {
			avformat_close_input(&pFormatCtx);
			pFormatCtx = nullptr;
		}
		std::cout << "Failed to retrieve input stream information" << std::endl;
		return -1;
	}
	// 得到视频流序号
	getVideoIndex();
	// 申请帧图像的内存空间
	pFrame = av_frame_alloc();

	//使用MPP解码
	if(BehaviorConf::Instance()->isMppEnable())
	{
		decoder = new H264MPPDecoder(width, height);
		decoder->init();
		decoder->setOnFrameCallBack([&](cv::Mat &yuvImg){
			yuvMtx.lock();
			//只解码关键帧不放入队列
			if(BehaviorConf::Instance()->isDecodeIdr())
			{
				if(onCapFrame)
					onCapFrame(yuvImg);
			}
			else
			{
				//将解码后的图片加入队列
				yuvQueue.push(yuvImg.clone());
			}
			yuvMtx.unlock();
		});

		std::thread decodeThread([&](){
			while(1)
			{
				packetMtx.lock();
				if(!packetQueue.empty())
				{
					if(decoder)
					{
						//取出packet，送至解码器
						AVPacket pkt = packetQueue.front();
						decoder->putAVPacket(&pkt);
						//释放资源
						freePacket(pkt);
						packetQueue.pop();
					}
				}
				packetMtx.unlock();

				usleep(5000);
			}
		});
		decodeThread.detach();

		isInited = true;
		return 0;
	}

	// 首先获取视频流序号，打开解码器
	openCodec(videoIndex, pCodecCtx);

	// 申请帧图像的内存空间
//	pFrame = av_frame_alloc();
	pFrameOut = av_frame_alloc();

	// 使用的缓冲区的大小


	// 开辟输出图像的缓存，并将缓存和帧进行关联
	numBytes = av_image_get_buffer_size(pixFmt,	// 色彩空间类型
			width,								// 图像的宽度
			height,								// 图像的高度
			1);									// 内存对齐方式，按照1个字节进行对齐，得到的结果就是与实际的内存大小一样
	buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
	// av_image_fill_arrays()函数自身不具备内存申请的功能，此函数类似于格式化已经申请的内存，即通过av_malloc()函数申请的内存空间。
	av_image_fill_arrays(pFrameOut->data,		// 输出图像帧的数据
			pFrameOut->linesize,				// 输出图像每个通道的行字节数
			buffer,								// 申请到的内存
			pixFmt,								// 色彩空间类型
			width,								// 图像的宽度
			height,								// 图像的高度
			1);									// 内存对齐方式，按照1个字节进行对齐，得到的结果就是与实际的内存大小一样
	pFrameOut->height = height;
	pFrameOut->width = width;

	// 创建图像转换的上下文，将YUV420P的视频帧转成RGB24,具体使用方法见：https://www.cnblogs.com/yongdaimi/p/10715830.html
	sws_ctx = sws_getContext(pCodecCtx->width, 	// 输入图像的宽度
			pCodecCtx->height, 					// 输入图像的高度
			pCodecCtx->pix_fmt,					// 输入图像的格式
			width, 								// 输出图像的宽度
			height, 							// 输出图像的高度
			pixFmt, 							// 输出图像的格式
			SWS_BILINEAR,						// 选择缩放算法(只有当输入输出图像大小不同时有效),一般选择SWS_FAST_BILINEAR
			nullptr,							// 输入图像的滤波器信息, 若不需要传nullptr
			nullptr,							// 输出图像的滤波器信息, 若不需要传nullptr
			nullptr);							// 特定缩放算法需要的参数，默认为nullptr

	isInited = true;

	return 0;
}

int RtspCapture::startCapture(std::function<void(cv::Mat& pic)> f)
{
	onCapFrame = f;
	// 从视频流中读取数据包进行解码
	std::thread decode_thread([&]() {
		threadrunning = true;
		decoding = true;
		cv::Mat rgbImage(height, width, CV_8UC3);
		int readerrcount = 0;
		while(decoding)
		{
			if(!isInited)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				init();
				continue;
			}
			AVPacket packet;
			if(readPacket(packet)) {
				// 只对视频流进行解码
				if(packet.stream_index == videoIndex) {
					// 保存录像，用于记录驾驶行为相关的视频
					RtspVideoCircleRecorder::Instance()->writeH264Frame((const char *)packet.data, packet.size);
					//判断是否包含关键帧，最低为1表示该数据是一个关键帧
					if((packet.flags & AV_PKT_FLAG_KEY) || !BehaviorConf::Instance()->isDecodeIdr())
					{
						if(BehaviorConf::Instance()->isMppEnable())
						{
							packetMtx.lock();
							AVPacket tmp;
							//引用计数+1
							av_packet_ref(&tmp, &packet);
							//加入缓存队列 等待解码
							packetQueue.push(tmp);
							packetMtx.unlock();
						}
						else
						{
							// 将码流包packet解码，存入到pFrame中
							// avcodec_send_packet的作用是将待解码的数据包“喂”给解码器，调用后必须释放packet
							avcodec_send_packet(pCodecCtx, &packet);
							// 判断解码是否成功，P帧和B帧，可能需要几个码流包才能解码成一个完整的图像帧
							if(avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
								// 进行图像变换，包括缩放及色彩空间变换
								sws_scale(sws_ctx,									// 转换格式的上下文
										(uint8_t const * const *)pFrame->data,		// 输入图像的每个通道的数据指针
										pFrame->linesize,							// 每个通道的行字节数
										0,											// 输入图像上处理区域：起始位置
										pCodecCtx->height,							// 输入图像上处理区域：处理的行数，这里表示处理整个图像
										pFrameOut->data,							// 输出图像信息
										pFrameOut->linesize);						// 输出图像每个通道字节数

								// 调用回调函数，让回调函数对解码后的数据进行处理
								if(onCapFrame) {
									rgbImage = cv::Mat(height, width, CV_8UC3, buffer);
									onCapFrame(rgbImage);
								}
							}
						}

					}

				}
				// 数据包使用完毕，解除和资源关联
				freePacket(packet);
				readerrcount = 0;
			}
			else
			{
				readerrcount++;
				if(readerrcount > 2)
				{
					if(pFormatCtx) {
						avformat_close_input(&pFormatCtx);
						pFormatCtx = nullptr;
					}
					closeCodec();
					isInited = false;
					readerrcount = 0;
				}
			}
		}
		// 最后关闭解码器
		closeCodec();
		threadrunning = false;
	});
	decode_thread.detach();
	return 0;
}

int  RtspCapture::stopCapture()
{
	decoding = false;
	// 等待抓取线程完全退出
	while(threadrunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// 关闭输入文件，释放相应的资源
	if(pFormatCtx) {
		avformat_close_input(&pFormatCtx);
		pFormatCtx = nullptr;
	}
	return 0;
}
int RtspCapture::getVideoIndex() {
	if(pFormatCtx) {
		//获取视频在码流中的序号：通过遍历每一个流，判断编码类型是否等于AVMEDIA_TYPE_VIDEO
		for(unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
			if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				videoIndex = i;
				frameRate = pFormatCtx->streams[i]->r_frame_rate.num;
				break;
			}
		}
	}
	return videoIndex;
}

bool RtspCapture::readPacket(AVPacket &packet) {
	if(av_read_frame(pFormatCtx, &packet) == 0) {
		return true;
	}
	else {
		return false;
	}
}

void RtspCapture::freePacket(AVPacket &packet) {
	av_packet_unref(&packet);
}

/**
 * 打开指定码流对应的解码器，打开成功后pCodecCtx指向对应的解码器上下文。解码器使用完毕要调用closeCodec()进行关闭
 * 参数：
 * 		streamIndex 码流序号
 * 		pCodecCtx AVCodecContext对象，解码器上下文
 * 返回值：
 * 		true 打开解码器成功；false 打开解码器失败
 */
bool RtspCapture::openCodec(int streamIndex, AVCodecContext* &pCodecCtx) {
	bool result{false};
	// 解码器指针
	AVCodec* pCodec{nullptr};

	// 创建码流对应的解码器上下文
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (pCodecCtx == NULL) {
		std::cout << "Could not allocate AVCodecContext" << std::endl;
		return false;
	}
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[streamIndex]->codecpar);

	// 找到码流对应的decoder
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(!pCodec) {
		std::cout << "Unsupported codec!" << std::endl;
		return false;
	}

	// 打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, nullptr) >= 0) {
		result = true;
	}

	return result;
}

/**
 * 关闭解码器
 * 参数：None
 */
void RtspCapture::closeCodec(){
	//释放解码器
	if(decoder)
	{
		delete decoder;
		decoder = NULL;
	}
	// 释放sws_scale
	if(sws_ctx)
	{
		sws_freeContext(sws_ctx);
		sws_ctx = nullptr;
	}
	// 释放图像帧资源和内存
	if(buffer)
	{
		av_free(buffer);
		buffer = nullptr;
	}
	if(pFrameOut)
	{
		av_frame_free(&pFrameOut);
		pFrameOut = nullptr;
	}
	if(pFrame)
	{
		av_frame_free(&pFrame);
		pFrame = nullptr;
	}
	if(pCodecCtx) {
		avcodec_close(pCodecCtx);
		pCodecCtx = nullptr;
	}
}

