/*
 * H264ToMp4.cpp
 *
 *  Created on: 2021年1月19日
 *      Author: syukousen
 */

#include "H264ToMp4.h"

#define STREAM_FRAME_RATE 30
#define STREAM_PIX_FMT    AV_PIX_FMT_NV12 /* default pix_fmt */
//#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

// < 0 = error
// 0 = I-Frame
// 1 = P-Frame
// 2 = B-Frame
// 3 = S-Frame
int H264ToMp4::getVopType( const void *p, int len )
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

    if((*b & 31) == 7)
    	return 0;
    return -1;
}

int H264ToMp4::CreateMp4(AVStream *in_stream)
{
    int ret; // 成功返回0，失败返回1
    AVOutputFormat *fmt;
    AVCodec *video_codec;

    if(mInited)
    	return 0;
    avformat_alloc_output_context2(&m_pOc, NULL, NULL, mMp4Name.c_str());
    if (!m_pOc)
    {
        printf("Could not deduce output format from file extension: using MPEG. \n");
        avformat_alloc_output_context2(&m_pOc, NULL, "mpeg", mMp4Name.c_str());
    }
    if (!m_pOc)
    {
        return -1;
    }

    fmt = m_pOc->oformat;

    if (fmt->video_codec != AV_CODEC_ID_NONE)
    {

		m_pVideoSt = avformat_new_stream(m_pOc, NULL);
		if (!m_pVideoSt)
		{
			printf("could not allocate stream \n");
			return -1;
		}
		m_pVideoSt->id = m_pOc->nb_streams-1;
    }

    if (m_pVideoSt)
    {
    	 ret = avcodec_parameters_copy(m_pVideoSt->codecpar, in_stream->codecpar);
    }

    av_dump_format(m_pOc, 0, mMp4Name.c_str(), 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&m_pOc->pb, mMp4Name.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("could not open %s\n", mMp4Name.c_str());
            return -1;
        }
    }

    /* Write the stream header, if any */
    ret = avformat_write_header(m_pOc, NULL);
    if (ret < 0)
    {
        printf("Error occurred when opening output file\n");
        return -1;
    }
    inputTimebase = in_stream->time_base;
    inputFramerate = in_stream->r_frame_rate;
    mInited = true;
    return 0;
}


/* write h264 data to mp4 file
 * 创建mp4文件返回2；写入数据帧返回0 */

void H264ToMp4::WriteVideo(void* data, int nLen)
{
    int ret;

    // Init packet
    AVPacket pkt;
    av_init_packet( &pkt );
    pkt.flags |= ( 0 == getVopType( data, nLen ) ) ? AV_PKT_FLAG_KEY : 0;

    pkt.stream_index = m_pVideoSt->index;
    pkt.data = (uint8_t*)data;
    pkt.size = nLen;

    // 检测是否是I帧，不是则丢弃,第一帧一定是关键帧，不然会花屏
    if ( waitkey )
        if ( 0 == ( pkt.flags & AV_PKT_FLAG_KEY ) )
            return ;
        else
            waitkey = 0;

	//Duration between 2 frames (¦Ìs)
	int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(inputFramerate);
	//Parameters
	pkt.pts = (double)(ptsInc*calc_duration) / (double)(av_q2d(inputTimebase)*AV_TIME_BASE);
	pkt.dts = pkt.pts;
	pkt.duration = (double)calc_duration / (double)(av_q2d(inputTimebase)*AV_TIME_BASE);
	ptsInc++;

	AVRounding rnd = (AVRounding)((int)AV_ROUND_NEAR_INF | (int)AV_ROUND_PASS_MINMAX);
	pkt.pts = av_rescale_q_rnd(pkt.pts, inputTimebase, m_pVideoSt->time_base, rnd);
	pkt.dts = av_rescale_q_rnd(pkt.dts, inputTimebase, m_pVideoSt->time_base, rnd);
	pkt.duration = av_rescale_q(pkt.duration, inputTimebase, m_pVideoSt->time_base);
	pkt.pos = -1;

    ret = av_interleaved_write_frame( m_pOc, &pkt );
    if (ret < 0)
    {
        printf("cannot write frame");
    }
}

void H264ToMp4::CloseMp4()
{
    waitkey = -1;

    // 写入mp4的尾部，一定要写，不然打不开
    if (m_pOc)
        av_write_trailer(m_pOc);

    if (m_pOc && !(m_pOc->oformat->flags & AVFMT_NOFILE))
        avio_close(m_pOc->pb);

    if (m_pOc)
    {
        avformat_free_context(m_pOc);
        m_pOc = nullptr;
    }
}


