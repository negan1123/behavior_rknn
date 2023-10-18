/*
 * LookRightLeftDetector.cpp
 *
 *  Created on: 2021年1月7日
 *      Author: syukousen
 */
#include <spdlog/spdlog.h>
#include "LookRightLeftDetector.h"
#include <RetinaDriverConf.hpp>

#include<iostream>

LookRightLeftDetector::LookRightLeftDetector()
{
	std::string var1,var2,var3;
	std::stringstream istring;

	preProcessImageW = 80;
	preProcessImageH = 80;

	// 动态数组，先要初始化或者设在大小
	mean = {127.5,127.5,127.5};
	std = {128.0,128.0,128.0};

	// 读取减均值
	istring << RetinaDriverConf::Instance()->POSE_MEAN;
	std::getline(istring,var1,',');
	std::getline(istring,var2,',');
	std::getline(istring,var3,',');
	mean[0] = std::stoi(var1);
	mean[1] = std::stoi(var2);
	mean[2] = std::stoi(var3);
	// 读取除均值
	istring.clear();
	istring.str("");
	istring << RetinaDriverConf::Instance()->POSE_STD;
	std::getline(istring,var1,',');
	std::getline(istring,var2,',');
	std::getline(istring,var3,',');
	std[0] = std::stof(var1);
	std[1] = std::stof(var2);
	std[2] = std::stof(var3);

	init(RetinaDriverConf::Instance()->POSE_RKNN_PATH);
}

LookRightLeftDetector::~LookRightLeftDetector()
{

}

int LookRightLeftDetector::imgPreprocess(void)
{
	// 将输入的图片按照高，宽比例小的缩放，然后复制到64*64的图像中
	cv::Mat resizeImage;
	cv::resize(inImage, resizeImage, cv::Size(90,90));
	cv::Rect cutRect(5,5,preProcessImageW,preProcessImageH);
	//这个裁减不会更新内存。所以内存数据还是90*90的
	resizeImage = resizeImage(cutRect);
	preProcessImage = resizeImage.clone();
#if 0
	testfilename = std::to_string(time(NULL)) + ".jpg";
	cv::imwrite(testfilename, preProcessImage);
#endif
	return 0;
}

int LookRightLeftDetector::detect(void)
{
	int ret = 0;
	rknn_input inputs[1];
	rknn_output outputs[1];

	yaw = 0;
	pitch = 0;
	roll = 0;

	//rknn 数据输入参数设置
	inputs[0].index = 0;
	inputs[0].size = preProcessImage.cols *preProcessImage.rows * preProcessImage.channels();
	inputs[0].pass_through = false;         //需要type和fmt
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = preProcessImage.data;

	// 数据输入
	ret = rknn_inputs_set(ctx, 1, inputs);
	if (ret < 0) {
		spdlog::error("rknn_input_set fail! ret={}", ret);
		return -1;
	}
	// 检测处理
	ret = rknn_run(ctx, nullptr);
	if (ret < 0) {
		spdlog::error("rknn_run fail! ret={}", ret);
		return -1;
	}

	// rknn 输出数据格式设置
	outputs[0].want_float = true;
	outputs[0].is_prealloc = false;
	outputs[0].index = 0;

	ret = rknn_outputs_get(ctx, 1, outputs, NULL);
	if (ret < 0) {
		spdlog::error("rknn_outputs_get fail! ret={}", ret);
		return -1;
	}

	float *outpos = ((float *)outputs[0].buf);
	yaw = float((1.0 / (1.0 + std::exp(-*outpos)))*180.0 - 90.0);
	pitch = float((1.0 / (1.0 + std::exp(-*(outpos+1))))*180.0 - 90.0);
	roll = float((1.0 / (1.0 + std::exp(-*(outpos+2))))*180.0 - 90.0);
	// 释放动态库里面申请的内存
	rknn_outputs_release(ctx, 1, outputs);

	time_t timenow = time(NULL);
	EularAngle lrAngle{timenow,yaw};
	if(lrAngleQueue.size() >= EULAR_QUEUE_SIZE)
	{
		lrAngleQueue.pop();
	}
	lrAngleQueue.push(lrAngle);
	return 0;
}

bool LookRightLeftDetector::isLookRightLeft(void)
{
	// 队列小于需要判断队列时
	if(lrAngleQueue.size() < EULAR_QUEUE_SIZE)
	{
		return false;
	}

	// 队尾的检测时间与队首的时间相差过大时
	if(lrAngleQueue.back().time - lrAngleQueue.front().time > 10)
	{
		lrAngleQueue.pop();
		return false;
	}

	// 判断角度相差大于30度时，判定为左顾右盼
	if(abs(lrAngleQueue.back().angle) > 20.0 && abs(lrAngleQueue.back().angle - lrAngleQueue.front().angle) > 30.0)
	{
		return true;
	}
	else
		return false;
}
