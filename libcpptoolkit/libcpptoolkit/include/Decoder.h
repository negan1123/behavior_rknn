#ifndef _DECODER_H_
#define _DECODER_H_

#include <iostream>
#include <functional>
#include <opencv2/core.hpp>
extern "C"{
    #include <libavcodec/avcodec.h>
}
/**
 * 使用方法
 * 1、 首先创建实例
 * （1）FFmpeg解码，传入ffmpeg上下文，视频索引，视频宽度，视频高度
 * Decoder *decoder = new H264FFmpegDecoder(ctx, videoIndex, width, height);
 * （2）MPP解码，视频宽度，视频高度
 * Decoder *decoder = new H264MPPDecoder(width, height);
 * 2、 进行初始化（必要）
 * decoder->init();
 * 3、 不断的喂数据
 * decoder->putAVPacket(packet);
 * 4、 设置回调取出解码后的图像，FFmpeg解码得到YUV420P格式的图像，MPP解码得到YUV420SP格式的图像
 * decoder->setOnFrameCallBack(std::function<void(cv::Mat &yuvImg)>)
 * 5、不需要解码时调用stopDecode
 * decoder->stopDecode();
 */
class Decoder
{
public:
    Decoder(){}
    virtual ~Decoder(){}

    /**
     * @brief 开始解码
     * @param packet h264包
     */
    virtual void putAVPacket(AVPacket* packet) = 0;

    /**
     * @brief 停止解码释放资源
     */
    virtual void stopDecode() = 0;

    /**
     * @brief 设置回调函数，当有一个帧（一幅图片或者一个音频）解码成功后回调，对解码成功的数据做后续处理
     * @param oncall 回调函数
     */
    virtual void setOnFrameCallBack(std::function<void(cv::Mat &yuvImg)> oncall) = 0;

    /**
     * @brief 初始化函数，解码前准备
     */
    virtual bool init() = 0;
};

#endif
