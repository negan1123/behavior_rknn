/*
 * RtspVideoCircleRecorder.cpp
 *
 *  Created on: 2021年5月27日
 *      Author: syukousen
 */
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
#include <spdlog/spdlog.h>
#include <BehaviorConf.hpp>
#include <RtspVideoCircleRecorder.h>

RtspVideoCircleRecorder::RtspVideoCircleRecorder()
{
	mFrameRate = 25;
	firstH264.startTime = 0;
	secondH264.startTime = 0;
	mVideoDuration = BehaviorConf::Instance()->getVideoDuration()*2;		//时长*2
	videoPath = BehaviorConf::Instance()->getCircleVideoPath();
	mSaveVideo = BehaviorConf::Instance()->isSaveDetectVideo();
}

int RtspVideoCircleRecorder::writeH264Frame(const char *h264data, int size)
{
	if(!mSaveVideo)
		return 0;
	if(!h264file.is_open())
	{
		return writeFirstH264Frame(h264data, size);
	}
	else
	{
		time_t nowtime;
		time( &nowtime );
		if(nowtime - nowH264.startTime < mVideoDuration)
		{
			h264file.write(h264data, size);
		}
		else
		{
			if(getVopType(h264data, size) != 0)
			{
				h264file.write(h264data, size);
			}
			else
			{
				h264file.close();
				if(0 == firstH264.startTime)
				{
					firstH264 = nowH264;
				}
				else if (0 == secondH264.startTime)
				{
					secondH264 = nowH264;
				}
				else
				{
					// 删除firstH264;
					remove(firstH264.fileName.c_str());
					firstH264 = secondH264;
					secondH264 = nowH264;
				}
				writeFirstH264Frame(h264data, size);
			}
		}
		return 0;
	}
	return 0;
}

int RtspVideoCircleRecorder::writeFirstH264Frame(const char *h264data, int size)
{
	if(getVopType(h264data, size) != 0)
		return -1;

	time_t starttime;
	std::string filename;

	time( &starttime );

	if(h264file.is_open())
	{
		h264file.close();
	}
	filename = videoPath + "/" + std::to_string(starttime) + ".H264";
	h264file.open(filename.c_str());
	// 判断一下文件打开没有
	if(h264file.is_open())
	{
		h264file.write(h264data, size);
		nowH264.startTime = starttime;
		nowH264.fileName = filename;
		return 0;
	}
	return -1;
}

int RtspVideoCircleRecorder::cutBehaviorVideo(time_t time, std::string savePath)
{
	int cutTime;
	int startCutTime;
	int duration = mVideoDuration/2;
	int ret = -1;
	H264ToMp4 mp4file(savePath);
	if(firstH264.startTime == 0 || secondH264.startTime == 0)
		return 1;
	if(time < firstH264.startTime)
	{
		// 驾驶员行为不在这个时间段内，不截取图像
		return ret;
	}
	else if(time - duration < firstH264.startTime)
	{
		cutTime = time+duration-firstH264.startTime;
		cutVideo(0, cutTime, firstH264.fileName, mp4file);
		mp4file.CloseMp4();
		ret = 0;
	}
	else if(time < secondH264.startTime)
	{
		cutTime = duration + secondH264.startTime - time;
		startCutTime = time - firstH264.startTime - duration;
		cutVideo(startCutTime, cutTime, firstH264.fileName, mp4file);

		cutTime = time + duration - secondH264.startTime;
		cutVideo(0, cutTime, secondH264.fileName, mp4file);
		mp4file.CloseMp4();
		ret = 0;
	}
	else if(time <  secondH264.startTime+duration)
	{
		cutTime = duration - (time - secondH264.startTime);
		startCutTime = time - firstH264.startTime - duration;
		cutVideo(startCutTime, cutTime, firstH264.fileName, mp4file);

		cutTime = time + duration - secondH264.startTime;
		cutVideo(0, cutTime, secondH264.fileName, mp4file);
		mp4file.CloseMp4();

		ret = 0;
	}
	else
	{
		//等待录像产生
		ret = 1;
	}
	return ret;
}

int RtspVideoCircleRecorder::cutVideo(int starttime, int timelen, std::string src, H264ToMp4 &outFile)
{
	AVDictionary* options = NULL;
	AVFormatContext *pInFormatCtx{nullptr};	///< ffmpeg输入上下文
	int videoIndex;							///< 视频流序号
	int frameRate;
	AVPacket pkt;
	int frame_index = 0;
	int startFrames;
	int saveFramesTotal;
	int saveFrames = 0;
	bool findIFrame = false;
	unsigned char *data;
	AVStream *in_stream;

	av_dict_set(&options, "stimeout", "10000000", 0);//设置超时10秒,一定要设置，不然后面的avformat_find_stream_info，readPacket可能不会退出
	if (avformat_open_input(&pInFormatCtx, src.c_str(), 0, &options) < 0) {
		spdlog::error("Could not open input file:{}", src.c_str());
		av_dict_free(&options);
		goto end;
	}
	av_dict_free(&options);
	// 读取输入流的基本信息到pFormatCtx中
	if (avformat_find_stream_info(pInFormatCtx, 0) < 0) {
		spdlog::error("Failed to retrieve input stream information");
		goto end;
	}
	// 得到视频流序号
	videoIndex = -1;
	//获取视频在码流中的序号：通过遍历每一个流，判断编码类型是否等于AVMEDIA_TYPE_VIDEO
	for(unsigned int i = 0; i < pInFormatCtx->nb_streams; i++) {
		if (pInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = i;
			break;
		}
	}
	// 没有找到视频流
	if(videoIndex == -1)
		goto end;
	in_stream = pInFormatCtx->streams[videoIndex];

	outFile.CreateMp4(in_stream);

	frameRate = in_stream->r_frame_rate.num/in_stream->r_frame_rate.den;
	startFrames = starttime*frameRate;
	saveFramesTotal = timelen*frameRate;

	while(av_read_frame(pInFormatCtx, &pkt) >= 0){
		//主要针对H264 raw data的文件，设置pts，dts,并且延迟读取文件的下一帧数据
		// 只对视频流进行推送
		if(pkt.stream_index == videoIndex) {
			frame_index++;
			if(frame_index >= startFrames)
			{
				if(!findIFrame)
				{
					data = (unsigned char *)pkt.data;
					if((*(data+4) & 31) == 7)
					{
						findIFrame = true;
					}
				}

				if(findIFrame)
				{
					if(saveFrames < saveFramesTotal)
					{
						outFile.WriteVideo(pkt.data, pkt.size);
						saveFrames++;
					}
					else
					{
						av_packet_unref(&pkt);
						break;
					}
				}
			}
		}
		// 数据包使用完毕，解除和资源关联
		av_packet_unref(&pkt);
	}
	if(pInFormatCtx) {
		avformat_close_input(&pInFormatCtx);
		pInFormatCtx = nullptr;
	}
	return 0;
end:
	if(pInFormatCtx) {
		avformat_close_input(&pInFormatCtx);
		pInFormatCtx = nullptr;
	}
	return -1;
}

int RtspVideoCircleRecorder::getVopType( const void *p, int len )
{
    if ( !p || 6 >= len )
        return -1;

    unsigned char *b = (unsigned char*)p;
    // Verify NAL marker
    if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
    {   b++;
    if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
        return -1;
    } // end if

    b += 3;

    // Verify VOP id
    if ( 0xb6 == *b )
    {   b++;
    return ( *b & 0xc0 ) >> 6;
    } // end if

    // I帧或者sps
    //if((*b & 31) == 5 || (*b & 31) == 7)
    if((*b & 31) == 7)
    	return 0;

    return -1;
}

