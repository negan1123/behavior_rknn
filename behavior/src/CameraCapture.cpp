/*
 * CameraCapture.cpp
 *
 *  Created on: 2021年1月6日
 *      Author: syukousen
 */
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include "CameraCapture.h"
#include "MjpegDecoder.h"
#include <spdlog/spdlog.h>

extern long long what_time_is_it_now();
#define CLEAR(x) memset(&(x), 0, sizeof(x))

CameraCapture::CameraCapture(const std::string &devName, int width, int height, int fmt , int interval) {
	this->devName = devName;
	this->fd_cam = -1;
	this->buffers = NULL;
	this->n_buffers = 0;
	this->frameIndex = -1;
	this->capW=width;
	this->capH=height;
	this->interval=interval;

	//摄像头输出格式，目前暂时支持这两种
	if(0 == fmt)
	{
		this->fmt = V4L2_PIX_FMT_YUYV;
	}
	else
	{
		this->fmt = V4L2_PIX_FMT_MJPEG;
	}
}

CameraCapture::~CameraCapture() {
	// TODO Auto-generated destructor stub
	freeBuffers();
	closeDevice();
}

int CameraCapture::openDevice() {
	// 设备的打开
	fd_cam = open(devName.c_str(), O_RDWR);
	if (fd_cam < 0) {
		spdlog::error("Can't open video device");
		return -1;
	}
	return 0;
}

int CameraCapture::closeDevice() {
	if (fd_cam > 0) {
		int ret = 0;
		if ((ret = close(fd_cam)) < 0) {
			spdlog::error("Can't close video device");
			return -1;
		}
		fd_cam = -1;
		return 0;
	} else {
		return -1;
	}
}

int CameraCapture::init() {
	int ret;
	struct v4l2_capability cam_cap;		//显示设备信息
	struct v4l2_cropcap cam_cropcap;	//设置摄像头的捕捉能力
	struct v4l2_fmtdesc cam_fmtdesc;	//查询所有支持的格式：VIDIOC_ENUM_FMT
	struct v4l2_crop cam_crop;			//图像的缩放
	struct v4l2_format cam_format;		//设置摄像头的视频制式、帧格式等

	//init 之前先打开设备
	ret = openDevice();
	/* 使用IOCTL命令VIDIOC_QUERYCAP，获取摄像头的基本信息*/
	ret = ioctl(fd_cam, VIDIOC_QUERYCAP, &cam_cap);
	if (ret < 0) {
		spdlog::error("Can't get device information: VIDIOCGCAP");
	}
	spdlog::info(
			"Driver Name:{}\nCard Name:{}\nBus info:{}\nDriver Version:{}.{}.{}\n",
			cam_cap.driver, cam_cap.card, cam_cap.bus_info,
			(cam_cap.version >> 16) & 0XFF, (cam_cap.version >> 8) & 0XFF,
			cam_cap.version & 0XFF);

	/* 使用IOCTL命令VIDIOC_ENUM_FMT，获取摄像头所有支持的格式*/
	cam_fmtdesc.index = 0;
	cam_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	spdlog::info("Support format:");
	while (ioctl(fd_cam, VIDIOC_ENUM_FMT, &cam_fmtdesc) != -1) {
		spdlog::info("{}.{}", cam_fmtdesc.index + 1, cam_fmtdesc.description);
		cam_fmtdesc.index++;
	}

	/* 使用IOCTL命令VIDIOC_CROPCAP，获取摄像头的捕捉能力*/
	cam_cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(fd_cam, VIDIOC_CROPCAP, &cam_cropcap)) {
		spdlog::info("Default rec:left:{}top:{}width:{}height:{}",
				cam_cropcap.defrect.left, cam_cropcap.defrect.top,
				cam_cropcap.defrect.width, cam_cropcap.defrect.height);
		/* 使用IOCTL命令VIDIOC_S_CROP，获取摄像头的窗口取景参数*/
		cam_crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cam_crop.c = cam_cropcap.defrect;		//默认取景窗口大小
		if (-1 == ioctl(fd_cam, VIDIOC_S_CROP, &cam_crop)) {
			//printf("Can't set crop para\n");
		}
	} else {
		spdlog::error("Can't set cropcap para");
	}

	/* 使用IOCTL命令VIDIOC_S_FMT，设置摄像头帧信息*/
	cam_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	cam_format.fmt.pix.width = capW;
	cam_format.fmt.pix.height = capH;
	cam_format.fmt.pix.pixelformat = fmt;		//要和摄像头支持的类型对应 V4L2_PIX_FMT_YUYV,V4L2_PIX_FMT_MJPEG
	cam_format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ret = ioctl(fd_cam, VIDIOC_S_FMT, &cam_format);
	if (ret < 0) {
		spdlog::error("Can't set frame information");
	}
	/* 使用IOCTL命令VIDIOC_G_FMT，获取摄像头帧信息*/
	cam_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd_cam, VIDIOC_G_FMT, &cam_format);
	if (ret < 0) {
		spdlog::error("Can't get frame information");
	}
	spdlog::info("Current data format information: width:{}height:{}",
			cam_format.fmt.pix.width, cam_format.fmt.pix.height);
	ret = initBuffers();
	if (ret < 0) {
		spdlog::error("Buffers init error");
		return -1;
	}
	return 0;
}

int CameraCapture::initBuffers() {
	int ret;
	/* 使用IOCTL命令VIDIOC_REQBUFS，申请帧缓冲*/
	struct v4l2_requestbuffers req;
	CLEAR(req);
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fd_cam, VIDIOC_REQBUFS, &req);
	if (ret < 0) {
		spdlog::error("Request frame buffers failed");
	}
	if (req.count < 2) {
		spdlog::error("Request frame buffers while insufficient buffer memory");
	}
	buffers = (struct cam_buffer*) calloc(req.count, sizeof(*buffers));
	if (!buffers) {
		spdlog::error("Out of memory");
	}
	for (n_buffers = 0; n_buffers < req.count; n_buffers++) {
		struct v4l2_buffer buf;
		CLEAR(buf);
		// 查询序号为n_buffers 的缓冲区，得到其起始物理地址和大小
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;
		ret = ioctl(fd_cam, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			spdlog::error("VIDIOC_QUERYBUF %d failed\n", n_buffers);
			return -1;
		}
		buffers[n_buffers].length = buf.length;
		// 映射内存
		buffers[n_buffers].start = mmap(
				NULL, // start anywhere
				buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_cam,
				buf.m.offset);
		if (MAP_FAILED == buffers[n_buffers].start) {
			spdlog::error("mmap buffer%d failed\n", n_buffers);
			return -1;
		}
	}
	return 0;
}

int CameraCapture::startCapture(std::function<void(cv::Mat& pic)> f) {
	unsigned int i;
	for (i = 0; i < n_buffers; i++) {
		struct v4l2_buffer buf;
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (-1 == ioctl(fd_cam, VIDIOC_QBUF, &buf)) {
			spdlog::error("VIDIOC_QBUF buffer%d failed\n", i);
			return -1;
		}
	}
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd_cam, VIDIOC_STREAMON, &type)) {
		spdlog::error("VIDIOC_STREAMON error");
		return -1;
	}
	onCapFrame = f;
	// 开启一个线程进行图像抓取
	std::thread decode_thread([&]() {
		int ret;
		unsigned char *yuv422frame = NULL;
		unsigned long yuvframeSize = 0;
		cv::Mat rgbImage(capH, capW, CV_8UC3);
		MjpegDecoder jpegdecoder(true);

		jpegdecoder.init();
		capturing = true;
		threadrunning = true;
		while(capturing)
		{
			ret = getFrame((void **) &yuv422frame, (size_t *)&yuvframeSize);
			if(0 == ret)
			{
				// 摄像机输出的是yuyv的格式
				if(fmt == V4L2_PIX_FMT_YUYV)
				{
					cv::Mat yuvImg(capH, capW, CV_8UC2, yuv422frame);
					cv::cvtColor(yuvImg, rgbImage, cv::COLOR_YUV2BGR_YUYV);
					onCapFrame(rgbImage);
				}
				else		//摄像机输出mjpeg的格式
				{
					std::vector<char> jpeg;
					jpeg.resize(yuvframeSize);
					memcpy(&jpeg[0], yuv422frame, yuvframeSize);
					jpegdecoder.decode(jpeg,rgbImage);
					onCapFrame(rgbImage);
				}
			}
			backFrame();
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		}
		threadrunning = false;
	});
	decode_thread.detach();
	return 0;
}

int CameraCapture::stopCapture() {
	enum v4l2_buf_type type;

	capturing = false;
	// 等待抓取线程完全退出
	while(threadrunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd_cam, VIDIOC_STREAMOFF, &type)) {
		spdlog::error("VIDIOC_STREAMOFF error");
		return -1;
	}
	return 0;
}

int CameraCapture::freeBuffers() {
	unsigned int i;
	for (i = 0; i < n_buffers; ++i) {
		if (-1 == munmap(buffers[i].start, buffers[i].length)) {
			spdlog::error("munmap buffer{} failed", i);
			return -1;
		}
	}
	free(buffers);
	return 0;
}

int CameraCapture::getFrame(void **frame_buf, size_t* len) {
	struct v4l2_buffer queue_buf;
	CLEAR(queue_buf);
	queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	queue_buf.memory = V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd_cam, VIDIOC_DQBUF, &queue_buf)) {
		spdlog::error("VIDIOC_DQBUF error");
		return -1;
	}
	*frame_buf = buffers[queue_buf.index].start;
	*len = buffers[queue_buf.index].length;
	frameIndex = queue_buf.index;
	return 0;
}

int CameraCapture::backFrame() {
	if (frameIndex != -1) {
		struct v4l2_buffer queue_buf;
		CLEAR(queue_buf);
		queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		queue_buf.memory = V4L2_MEMORY_MMAP;
		queue_buf.index = frameIndex;
		if (-1 == ioctl(fd_cam, VIDIOC_QBUF, &queue_buf)) {
			spdlog::error("VIDIOC_QBUF error");
			return -1;
		}
		return 0;
	}
	return -1;
}


