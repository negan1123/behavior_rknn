/*
 * H264ToMp4.h
 *
 *  Created on: 2021年1月19日
 *      Author: syukousen
 */

#ifndef H264TOMP4_H_
#define H264TOMP4_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>
extern "C"
{
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
class H264ToMp4
{
public:
	/**@brief 构造函数
	* @param	None
	* @return	None
	*/
	H264ToMp4(std::string name):mMp4Name(name){ptsInc=0;waitkey=1;};

	/**@brief 构造函数
	* @param	None
	* @return	None
	*/
	~H264ToMp4(){};

	/**@brief 初始化生成mp4的参数，初始化编码器
	* @param	None
	* @return	int		初始化结果
	*/
	int CreateMp4(AVStream *in_stream);

	/**@brief 向编码器写入H264的帧数据
	* @param[in]	data	H264的帧数据
	* @param[in]	nLen	H264的帧数据长度
	* @return	None
	*/
	void WriteVideo(void* data, int nLen);

	/**@brief 关闭编码器，写入mp4尾部
	* @param	None
	* @return	None
	*/
	void CloseMp4();

private:
	/**@brief 获取视频帧类型
	* @param[in]	p	H264的帧数据
	* @param[in]	len	H264的帧数据长度
	* @return	int  帧类型
	* 			 < 0 = error
	*			0 = I-Frame
	*			1 = P-Frame
	*			2 = B-Frame
	*/
	int getVopType( const void *p, int len );
private:
	AVFormatContext *m_pOc{nullptr};
    AVStream *m_pVideoSt{nullptr};
    AVRational inputTimebase;
    AVRational inputFramerate;
    bool mInited{false};
    std::string mMp4Name;
    int ptsInc;
    int waitkey;
};
#endif /* H264TOMP4_H_ */
