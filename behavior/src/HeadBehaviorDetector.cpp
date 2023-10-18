/*
 * HeadBehaviorDetector.cpp
 *
 *  Created on: Mar 31, 2022
 *      Author: wy
 */

#include <HeadBehaviorDetector.h>
#include <RetinaDriverConf.hpp>
#include <spdlog/spdlog.h>

HeadBehaviorDetector::HeadBehaviorDetector()
{
	imageWidth = 224;
	imageHeight = 224;
	init(RetinaDriverConf::Instance()->HEAD_BEHAVIOR_RKNN_PATH);
}

HeadBehaviorDetector::~HeadBehaviorDetector()
{

}

int HeadBehaviorDetector::imgPreprocess(void)
{
	cv::Mat temp;
	cv::Mat img_rgb;
	cv::resize(inImage, temp, cv::Size(imageWidth, imageHeight));
	cv::cvtColor(temp, preProcessImage, cv::COLOR_BGR2RGB);
	preProcessImage.convertTo(preProcessImage, CV_32FC3, 1/255.0);
}

int HeadBehaviorDetector::detect(void)
{
	int ret = 0;
	rknn_input inputs[1];
	rknn_output outputs[2];
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
	for(int i = 0; i < 2; i++)
	{
		outputs[i].want_float = true;
		outputs[i].is_prealloc = false;
		outputs[i].index = i;
	}
	// 获取rknn输出
	ret = rknn_outputs_get(ctx, 2, outputs, NULL);
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
	for(int i = 0; i < 2; i++)
	{
		outData.push_back(new char[outputs[i].size]);
		memcpy(outData[i], outputs[i].buf, outputs[i].size);
	}
	// 释放动态库里面申请的内存
	rknn_outputs_release(ctx, 2, outputs);
	return 0;
}

int HeadBehaviorDetector::classifyBehavior(void)
{
	if(outData.empty())
	{
		spdlog::error("head behavior rknn outData empty");
		return -1;
	}
	float conf_value[2] = {0.0};
	float expvalue[2] = {0.0};
	float sum = 1.0;
	for(int conf_index = 0; conf_index < 2; conf_index++)
	{
	/* 计算方法
	 *  x[i] = exp(x[i])
	 *  sum = x[0]+x[1]
	 *  x[i] = x[i]/sum
	 *  if(x[1] > x[0])
	 *  	行为检测正确
	 * */
		float *outpos = ((float *)outData[conf_index]);
		bool beh = false;
		conf_value[0] = *outpos;
		conf_value[1] = *(outpos + 1);
		expvalue[0] = exp(conf_value[0]);
		expvalue[1] = exp(conf_value[1]);
		sum = expvalue[0] + expvalue[1];
		conf_value[0] = expvalue[0] / sum;
		conf_value[1] = expvalue[1] / sum;
		if(conf_value[1] > conf_value[0])
			beh = true;
		// 将对应的行为赋值
		if(conf_index == 0)
		{
			behavior.close_eye = beh;
		}
		else
		{
			behavior.yawn = beh;
		}
	}
	return 0;
}

HeadBehavior HeadBehaviorDetector::getBehavior(void)
{
	return behavior;
}

