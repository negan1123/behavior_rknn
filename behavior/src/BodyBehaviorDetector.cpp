/*
 * BodyBehaviorDetector.cpp
 *
 *  Created on: Apr 1, 2022
 *      Author: wy
 */

#include <BodyBehaviorDetector.h>
#include <RetinaDriverConf.hpp>
#include <spdlog/spdlog.h>

BodyBehaviorDetector::BodyBehaviorDetector()
{
	imageWidth = 224;
	imageHeight = 224;
	init(RetinaDriverConf::Instance()->BODY_BEHAVIOR_RKNN_PATH);
}

BodyBehaviorDetector::~BodyBehaviorDetector()
{

}

int BodyBehaviorDetector::imgPreprocess(void)
{
	cv::Mat temp;
	cv::Mat img_rgb;
	cv::resize(inImage, temp, cv::Size(imageWidth, imageHeight));
	cv::cvtColor(temp, preProcessImage, cv::COLOR_BGR2RGB);
	preProcessImage.convertTo(preProcessImage, CV_32FC3, 1/255.0);
}

int BodyBehaviorDetector::detect(void)
{
	int ret = 0;
	rknn_input inputs[1];
	rknn_output outputs[1];
	// rknn 输入格式设置
	inputs[0].index = 0;
	inputs[0].size = preProcessImage.cols * preProcessImage.rows * preProcessImage.channels() * 4;
	inputs[0].pass_through = false;         //需要type和fmt
	inputs[0].type = RKNN_TENSOR_FLOAT32;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = preProcessImage.data;
	// 数据输入
	ret = rknn_inputs_set(ctx, 1, inputs);
	if (ret < 0) {
		printf("rknn_input_set fail! ret=%d", ret);
		return -1;
	}
	// 检测处理
	ret = rknn_run(ctx, nullptr);
	if (ret < 0) {
		printf("rknn_run fail! ret=%d", ret);
		return -1;
	}
	// rknn 输出数据格式设置
	outputs[0].want_float = true;
	outputs[0].is_prealloc = false;
	outputs[0].index = 0;
	// 获取rknn输出
	ret = rknn_outputs_get(ctx, 1, outputs, NULL);
	if (ret < 0) {
		printf("rknn_outputs_get fail! ret=%d", ret);
		return -1;
	}
	// 清除上次输出数据
	for(std::vector<void *>::const_iterator iter=outData.begin();iter!=outData.end();++iter)
	{
		char *ptr = (char*)*iter;
		delete []ptr;
	}
	outData.clear();
	// 保存输出
	outData.push_back(new char[outputs[0].size]);
	memcpy(outData[0], outputs[0].buf, outputs[0].size);
	// 释放动态库里面申请的内存
	rknn_outputs_release(ctx, 1, outputs);
	return 0;
}

int BodyBehaviorDetector::classifyBehavior(void)
{
	if(outData.empty())
	{
		spdlog::error("body behavior rknn outData empty");
		return -1;
	}
	float conf_value[4] = {0.0};
	float expvalue[4] = {0.0};
	float sum = 0.0;

	/* 计算方法
	 *  x[i] = exp(x[i])
	 *  sum = x[0]+x[1]
	 *  x[i] = x[i]/sum
	 *  if(x[1] > x[0])
	 *  	行为检测正确
	 * */
	float *outpos = ((float *)outData[0]);
	int i = 0;
	for(i = 0; i < 4; ++i)
	{
		conf_value[i] = *(outpos + i);
		expvalue[i] = exp(conf_value[i]);
		sum += expvalue[i];
	}
	for(i = 0; i < 4; ++i)
	{
		conf_value[i] = expvalue[i] / sum;
	}
	float max = conf_value[0];
	int index = 0;
	for(i = 1; i < 4; ++i)
	{
		if(conf_value[i] > max)
		{
			index = i;
			max = conf_value[i];
		}
	}
	behavior = index;
	return 0;
}

int BodyBehaviorDetector::getBehavior(void)
{
	return behavior;
}


