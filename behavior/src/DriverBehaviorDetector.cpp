/*
 * DriverBehaviorDetector.cpp
 *
 *  Created on: 2020年12月29日
 *      Author: syukousen
 */
#include "DriverBehaviorDetector.h"
#include <RetinaDriverConf.hpp>
#include <spdlog/spdlog.h>

//#define  SIZE_200  true
DriverBehaviorDetector::DriverBehaviorDetector()
{
	std::string var1,var2,var3;
	std::stringstream istring;
#if SIZE_200
	preProcessImageW = 200;
	preProcessImageH = 200;
#else
	preProcessImageW = 416;
	preProcessImageH = 416;
#endif
	// 动态数组，先要初始化或者设在大小
	mean = {0.485,0.456,0.406};
	std = {0.229,0.224,0.225};
	// 读取减均值
	istring << RetinaDriverConf::Instance()->BOD_MEAN;
	std::getline(istring,var1,',');
	std::getline(istring,var2,',');
	std::getline(istring,var3,',');
	mean[0] = std::stof(var1);
	mean[1] = std::stof(var2);
	mean[2] = std::stof(var3);
	// 读取除均值
	istring.clear();
	istring.str("");
	istring << RetinaDriverConf::Instance()->BOD_STD;
	std::getline(istring,var1,',');
	std::getline(istring,var2,',');
	std::getline(istring,var3,',');
	std[0] = std::stof(var1);
	std[1] = std::stof(var2);
	std[2] = std::stof(var3);
	init(RetinaDriverConf::Instance()->BOD_RKNN_PATH);
}

DriverBehaviorDetector::~DriverBehaviorDetector()
{

}

int DriverBehaviorDetector::imgPreprocess(void)
{
	// 将输入的图片按照高，宽比例小的缩放，然后复制到416*416的图像中
#if 0
	cv::resize(inImage, preProcessImage, cv::Size(preProcessImageW,preProcessImageH));
#else	//转化成rgb的
	cv::Mat temp;
	cv::resize(inImage, temp, cv::Size(preProcessImageW,preProcessImageH));
	cv::cvtColor(temp, preProcessImage, cv::COLOR_BGR2RGB);
#endif
#if SIZE_200
	preProcessImage.convertTo(preProcessImage, CV_32FC3, 1/255.0);
#endif
	preProcessImage.convertTo(preProcessImage, CV_32FC3, 1/255.0);
#ifndef RKNN_HAVE_MEAN_DEAL
	// 检测是否需要减去均值
	if(mean.size() == 3)
	{
		cv::Scalar scalarmean = cv::Scalar(mean[0], mean[1], mean[2]);
		cv::Scalar scalarstd = cv::Scalar(std[0], std[1], std[2]);
		preProcessImage = preProcessImage-scalarmean;
		preProcessImage = preProcessImage/scalarstd;
	}
#endif
	return 0;
}

int DriverBehaviorDetector::detect(void)
{
	int ret = 0;
	rknn_input inputs[1];
	rknn_output outputs[DRIVER_RKNN_OUTPUT_CNT];

	//rknn 数据输入参数设置
	inputs[0].index = 0;
#if SIZE_200
	inputs[0].size = preProcessImage.cols *preProcessImage.rows * preProcessImage.channels()*4;
	inputs[0].pass_through = false;         //需要type和fmt
	inputs[0].type = RKNN_TENSOR_FLOAT32;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = preProcessImage.data;
#else
	inputs[0].size = preProcessImage.cols *preProcessImage.rows * preProcessImage.channels()*4;
	inputs[0].pass_through = false;         //需要type和fmt
	inputs[0].type = RKNN_TENSOR_FLOAT32;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = preProcessImage.data;
#endif
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
	for(int i = 0; i < DRIVER_RKNN_OUTPUT_CNT; i++)
	{
		outputs[i].want_float = true;
		outputs[i].is_prealloc = false;
		outputs[i].index = i;
	}
	/*	输出的6个output
		地址	长度(byte)	类型	内容
		outputs[0]
		0	4	float	CloseEye置信度相关(第一个)
		4	4	float
		outputs[1]
		0	4	float	Yawn置信度相关(第二个)
		4	4	float
		outputs[2]
		0	4	float	Smoke置信度相关(第三个)
		4	4	float
		outputs[3]
		0	4	float	Phone置信度相关(第四个)
		4	4	float
		outputs[4]
		0	4	float	Drink置信度相关(第五个)
		4	4	float
		outputs[5]
		0	4	float	BlockCamera置信度相关(第六个)
		4	4	float
	 */
	ret = rknn_outputs_get(ctx, DRIVER_RKNN_OUTPUT_CNT, outputs, NULL);
	if (ret < 0) {
		spdlog::error("rknn_outputs_get fail! ret={}", ret);
		return -1;
	}

	// 清除上次输出数据
	for(std::vector<void *>::const_iterator iter=outData.begin();iter!=outData.end();++iter)
	{
		char *ptr = (char*)*iter;
		delete []ptr;
	}
	outData.clear();

	// 保存输出，供classifyBehavior检测驾驶行为用
	for(int i = 0; i < DRIVER_RKNN_OUTPUT_CNT; i++)
	{
		outData.push_back(new char[outputs[i].size]);
		memcpy(outData[i], outputs[i].buf, outputs[i].size);
	}
	// 释放动态库里面申请的内存
	rknn_outputs_release(ctx, DRIVER_RKNN_OUTPUT_CNT, outputs);
	return 0;
}

int DriverBehaviorDetector::classifyBehavior(void)
{
	if(outData.empty())
	{
		spdlog::error("dirver behavior rknn outData empty");
		return -1;
	}
	float conf_value[2] = {0.0};
	float expvalue[2] = {0.0};
	float sum = 1.0;

	behavior = {false,false,false,false,false,false,false,false};
	/*	输出的6个output
		地址	长度(byte)	类型	内容
		outputs[0]
		0	4	float	CloseEye置信度相关(第一个)
		4	4	float
		outputs[1]
		0	4	float	Yawn置信度相关(第二个)
		4	4	float
		outputs[2]
		0	4	float	Smoke置信度相关(第三个)
		4	4	float
		outputs[3]
		0	4	float	Phone置信度相关(第四个)
		4	4	float
		outputs[4]
		0	4	float	Drink置信度相关(第五个)
		4	4	float
		outputs[5]
		0	4	float	BlockCamera置信度相关(第六个)
		4	4	float
	 */
	for(int conf_index = 0; conf_index < DRIVER_RKNN_OUTPUT_CNT; conf_index++)
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
		switch(conf_index)
		{
			// 第一行 CloseEye
			case 0:
			{
				behavior.closeEye = beh;
			}
			break;
			// 第二行 Yawn
			case 1:
			{
				behavior.yawn = beh;
			}
			break;
			// 第三行 Smoke
			case 2:
			{
				behavior.smoke = beh;
			}
			break;
			// 第四行 Phone
			case 3:
			{
				behavior.phone = beh;
			}
			break;
			// 第五行 Drink
			case 4:
			{
				behavior.drink = beh;
			}
			break;
			// 第六行 BlockCamera
			case 5:
			{
				behavior.blockCamera = beh;
			}
			break;
		}
	}
	return 0;
}


