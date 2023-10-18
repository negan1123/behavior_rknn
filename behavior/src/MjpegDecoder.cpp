/*
 * MjpegDecoder.cpp
 *
 *  Created on: 2021年1月7日
 *      Author: syukousen
 */
#include "MjpegDecoder.h"
#include <spdlog/spdlog.h>
extern long long what_time_is_it_now();

#define SZ_4M (4*1024*1024)						///<	mpp解码器输入输出buf大小

MjpegDecoder::MjpegDecoder(bool mppdecode)
{
	this->mppdecode = mppdecode;
}

MjpegDecoder::~MjpegDecoder()
{
	if (packet) {
		mpp_packet_deinit(&packet);
		packet = nullptr;
	}
	if (frame) {
		mpp_frame_deinit(&frame);
		frame = nullptr;
	}
	if (ctx) {
		mpp_destroy(ctx);
		ctx = nullptr;
	}
	if (pkt_buf) {
		mpp_buffer_put(pkt_buf);
		pkt_buf = nullptr;
	}
	if (frm_buf) {
		mpp_buffer_put(frm_buf);
		frm_buf = nullptr;
	}
	if (pkt_grp) {
		mpp_buffer_group_put(pkt_grp);
		pkt_grp = nullptr;
	}
	if (frm_grp) {
		mpp_buffer_group_put(frm_grp);
		frm_grp = nullptr;
	}
}

int MjpegDecoder::init()
{
	MPP_RET ret = MPP_OK;
	if(mppdecode)
	{
	    MppParam param      = NULL;
	    RK_U32 need_split   = 1;
	    MppFrameFormat  format = MPP_FMT_YUV420SP;

	    // 获取mpp解码器输出帧数组
		ret = mpp_buffer_group_get_internal(&frm_grp, MPP_BUFFER_TYPE_ION);
		if (ret) {
			spdlog::error("failed to get buffer group for input frame ret {}", ret);
			goto MPP_TEST_OUT;
		}

		// 获取mpp解码器输入帧数组
		ret = mpp_buffer_group_get_internal(&pkt_grp, MPP_BUFFER_TYPE_ION);
		if (ret) {
			spdlog::error("failed to get buffer group for output packet ret {}", ret);
			goto MPP_TEST_OUT;
		}

		// 初始化解码器输出帧
		ret = mpp_frame_init(&frame); /* output frame */
		if (MPP_OK != ret) {
			spdlog::error("mpp_frame_init failed");
			goto MPP_TEST_OUT;
		}
		// 获取解码器输出buf
		ret = mpp_buffer_get(frm_grp, &frm_buf, SZ_4M);
		if (ret) {
			spdlog::error("failed to get buffer for input frame ret {}", ret);
			goto MPP_TEST_OUT;
		}
		// 获取解码器输入buf
		ret = mpp_buffer_get(pkt_grp, &pkt_buf, SZ_4M);
		if (ret) {
			spdlog::error("failed to get buffer for input frame ret {}", ret);
			goto MPP_TEST_OUT;
		}
		// 将输入包结构和输入buf绑定
		mpp_packet_init_with_buffer(&packet, pkt_buf);

		// 将输出帧结构和输出buf绑定
		mpp_frame_set_buffer(frame, frm_buf);

	    // 创建mpp解码实例
	    ret = mpp_create(&ctx, &mpi);

	    if (MPP_OK != ret) {
	    	spdlog::error("mpp_create failed");
	        goto MPP_TEST_OUT;
	    }

	    // 设置解码器是否整帧的输入
	    param = &need_split;
	    ret = mpi->control(ctx, MPP_DEC_SET_PARSER_SPLIT_MODE, param);
	    if (MPP_OK != ret) {
	    	spdlog::error("mpi->control failed");
	        goto MPP_TEST_OUT;
	    }
	    // 设置解码器解码的类型
	    ret = mpp_init(ctx, MPP_CTX_DEC, MPP_VIDEO_CodingMJPEG);
	    if (MPP_OK != ret) {
	    	spdlog::error("mpp_init failed");
	        goto MPP_TEST_OUT;
	    }
	    // 设置解码器输出的解码类型，测试发现解码器直接输出BGR数据不正确，所以改成yuv420sp格式，让opencv转化为BGR
	    //mpi->control(ctx, MPP_DEC_SET_OUTPUT_FORMAT, &format);
	    return 0;
MPP_TEST_OUT:
		if (packet) {
			mpp_packet_deinit(&packet);
			packet = nullptr;
		}
		if (frame) {
			mpp_frame_deinit(&frame);
			frame = nullptr;
		}
		if (ctx) {
			mpp_destroy(ctx);
			ctx = nullptr;
		}
		if (pkt_buf) {
			mpp_buffer_put(pkt_buf);
			pkt_buf = nullptr;
		}
		if (frm_buf) {
			mpp_buffer_put(frm_buf);
			frm_buf = nullptr;
		}
		if (pkt_grp) {
			mpp_buffer_group_put(pkt_grp);
			pkt_grp = nullptr;
		}
		if (frm_grp) {
			mpp_buffer_group_put(frm_grp);
			frm_grp = nullptr;
		}
		return -1;
	}
	return 0;
}

int MjpegDecoder::decode(const std::vector<char>& jpeg, cv::Mat& bgr)
{
	MPP_RET ret = MPP_OK;
	int funcret = -1;
	if(mppdecode)
	{
		MppTask task = NULL;
		char *buf = (char *)mpp_buffer_get_ptr(pkt_buf);

		// 将jpeg数据输入解码器的输入buf
		memcpy(buf, jpeg.data(), jpeg.size());
		// 设置解码输入包的位置和长度
		mpp_packet_set_pos(packet, buf);
		mpp_packet_set_length(packet, jpeg.size());
		// 设置输入包是否完成，由于我们是整帧的解码，所以一次全部输入完成
		mpp_packet_set_eos(packet);

		// 等待输入
		ret = mpi->poll(ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
		if (ret) {
			spdlog::error("mpp input poll failed");
			return -1;
		}
		// 获取输入
		ret = mpi->dequeue(ctx, MPP_PORT_INPUT, &task);  /* input queue */
		if (ret) {
			spdlog::error("mpp task input dequeue failed");
			return -1;
		}
		// 设置输入输出
		mpp_task_meta_set_packet(task, KEY_INPUT_PACKET, packet);
		mpp_task_meta_set_frame (task, KEY_OUTPUT_FRAME, frame);
		// 释放输入
		ret = mpi->enqueue(ctx, MPP_PORT_INPUT, task);  /* input queue */
		if (ret) {
			spdlog::error("mpp task input enqueue failed");
			return -1;
		}
		// 等待输出
		ret = mpi->poll(ctx, MPP_PORT_OUTPUT, MPP_POLL_BLOCK);
		if (ret) {
			spdlog::error("mpp output poll failed");
			return -1;
		}
		// 获取输出
		ret = mpi->dequeue(ctx, MPP_PORT_OUTPUT, &task); /* output queue */
		if (ret) {
			spdlog::error("mpp task output dequeue failed");
			return -1;
		}
		if (task) {
			MppBuffer buffer = NULL;
			RK_U8 *base = NULL;
			RK_U32 width    = 0;
			RK_U32 height   = 0;

			// 获取解码后的图像长宽，数据
			width    = mpp_frame_get_width(frame);
			height   = mpp_frame_get_height(frame);
			buffer   = mpp_frame_get_buffer(frame);

			if (NULL == buffer)
				return -1;

			base = (RK_U8 *)mpp_buffer_get_ptr(buffer);
			// 将yuv420sp的数据转化为BGR的数据
			cv::Mat yuvImg(height*3/2, width, CV_8UC1, (void*)base);
			cv::cvtColor(yuvImg, bgr, cv::COLOR_YUV2BGR_NV12);
			funcret = 0;
			// 释放输出
			ret = mpi->enqueue(ctx, MPP_PORT_OUTPUT, task);
			if (ret)
				spdlog::error("mpp task output enqueue failed");
		}
		return funcret;
	}
	else
	{
		// opencv直接软解mjpeg
		bgr=cv::imdecode(jpeg,cv::IMREAD_COLOR);
		return 0;
	}
}

