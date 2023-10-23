#ifndef _rtsp_video_circle_recorder_h_
#define _rtsp_video_circle_recorder_h_

#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <H264ToMp4.h>

using namespace std;

typedef struct _CirCleH264Info_
{
	time_t startTime;
	std::string fileName;
}CirCleH264Info;

/**@class RtspVideoCircleRecorder
 * @brief 视频循环存储的封装类，主要是在保存驾驶行为发生是前后时间段视频时使用
 * */
class RtspVideoCircleRecorder
{
public:
	/**@brief 视频循环存储单列
	* @param	None
	* @return	RtspVideoCircleRecorder * 视频循环存储单列指针
	*/
	static RtspVideoCircleRecorder* Instance()
	{
		static RtspVideoCircleRecorder* _instance = nullptr;
		if(nullptr == _instance)
		{
			_instance = new RtspVideoCircleRecorder;
		}
		return _instance;
	}

	/**@brief 设置rtsp帧率
	* @param[in]	rt			帧率
	* @return	None
	*/
	void setRtspFrameRate(int rt) {mFrameRate = rt;}

	/**@brief 循环存储视频数据，写入一帧H264数据源
	* @param[in]	h264data		H264帧数据
	* @param[in]	size			帧数据长度
	* @return	int			返回成功与否
	*/
	int writeH264Frame(const char *h264data, int size);

	/**@brief 截取驾驶行为发生时间段视频
	* @param[in]	t				驾驶行为发生时间
	* @param[in]	savePath		视频保存路径
	* @return	int			返回成功与否
	*/
	int cutBehaviorVideo(time_t t, std::string savePath);
private:
	/**@brief 构造函数。
	* @param	None
	* @return	None
	*/
	RtspVideoCircleRecorder();

	/**@brief 输入H264视频帧进行存储
	* @param[in]	h264data	H264视频帧数据
	* @param[in]	size		视频数据大小
	* @return	int			返回成功与否
	*/
	int writeFirstH264Frame(const char *h264data, int size);

	/**@brief 检测是否I帧
	* @param[in]	p	H264视频帧数据
	* @param[in]	len	视频数据大小
	* @return	int		H264帧类型
	* 			< 0		error
	* 			0 		I-Frame
	* 			1		P-Frame
	* 			2		B-Frame
	*/
	int getVopType( const void *p, int len );

	/**@brief 截取驾驶行为发生的前后视频，保存成mp4文件
	* @param[in]	starttime	开始时间
	* @param[in]	timelen		截取时间长度
	* @param[in]	src			循环存储视频源
	* @param[out]	outFile		输出的mp4文件对象
	* @return	int			返回成功与否
	*/
	int cutVideo(int starttime, int timelen, std::string src, H264ToMp4 &outFile);
private:
	int mFrameRate;				///< 抓取的rtsp流的帧率
	int mVideoDuration;			///< 驾驶员行为保存的视频长度
	int mSaveVideo;				///< 是否保存录像
	std::ofstream h264file;		///< 保存文件的输出流
	CirCleH264Info	firstH264;		///< 第一段H264码流
	CirCleH264Info	secondH264;		///< 第二段H264码流
	CirCleH264Info	nowH264;		///< 正在保存的H264码流
	std::string videoPath;			///< 循环码流保存的内存路径
};

#endif /* _rtsp_video_circle_recorder_h_ */
