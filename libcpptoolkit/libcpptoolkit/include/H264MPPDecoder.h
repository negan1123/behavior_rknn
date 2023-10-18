#ifndef _H264MPPDECODER_H_
#define _H264MPPDECODER_H_

#include "Decoder.h"
#include <rockchip/rk_type.h>
#include <rockchip/mpp_frame.h>
#include <rockchip/mpp_packet.h>
#include <rockchip/rk_mpi.h>

class H264MPPDecoder : public Decoder
{
private:
    MppCtx ctx{nullptr};		///< 解码器句柄
    MppApi *mpi{nullptr};		///< 解码器实例化对象指针
    int width;		                    	            ///< 视频宽度
    int height;		                                    ///< 视频高度
    std::function<void(cv::Mat &yuvImg)> yuvCallBack;  ///<解码回調
public:
    H264MPPDecoder(int width, int height);
    ~H264MPPDecoder();
    bool init();
    void putAVPacket(AVPacket*);
    void stopDecode();
    void setOnFrameCallBack(std::function<void(cv::Mat &yuvImg)> oncall){yuvCallBack = oncall;}
};

#endif
