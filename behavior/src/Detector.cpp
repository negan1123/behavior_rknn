/*
 * Detector.cpp
 *
 *  Created on: 2020年12月22日
 *      Author: syukousen
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <BehaviorConf.hpp>
#include "Detector.h"
#include <spdlog/spdlog.h>

Detector::Detector()
{

}

Detector::~Detector()
{
	deInit();
}

int Detector::init(std::string rknnName)
{
	// Load model
	int ret = 0;
	int model_len = 0;
	// 打开模型文件，获取模型数据
	std::ifstream is (rknnName.c_str(), std::ios::in | std::ios::binary);
	if (is)
	{
		// get length of file:
		is.seekg (0, is.end);
		model_len = is.tellg();
		is.seekg (0, is.beg);

		if(nullptr != model)
		{
			delete [](char*)model;
			model = nullptr;
		}
		model = new char[model_len];
		is.read ((char*)model,model_len);
		if (!is)
		{
			spdlog::error("read rknn err");
			delete [](char*)model;
			model = nullptr;
			is.close();
			return -1;
		}
		is.close();
	}
	else
	{
		return -1;
	}

	// 初始化模型
	ret = rknn_init(&ctx,model,model_len,RKNN_FLAG_PRIOR_MEDIUM);

	delete [](char*)model;
	model = nullptr;
	if(ret < 0)
	{
		spdlog::error("rknn_init fail! ret={}", ret);
		return -1;
	}
	showModelInfo();
	return 0;
}

int Detector::deInit(void)
{
	// 释放模型的内存
	if(nullptr != model)
	{
		delete [](char*)model;
		model = nullptr;
	}

	// 打开模型文件，获取模型数据
	for(std::vector<void *>::const_iterator iter=outData.begin();iter!=outData.end();++iter)
	{
		char *ptr = (char*)*iter;
		delete []ptr;
	}
	outData.clear();
	return 0;
}

int Detector::input(cv::Mat &image)
{
	// 输入图片保存
	inImage = image;
	// 调用子类的预处理，将图片处理成模型需要的数据格式
	imgPreprocess();
	return 0;
}

void Detector::showModelInfo(void)
{
	rknn_input_output_num querynum;
	int ret = 0;
	rknn_tensor_attr inputs_attr[1];
	rknn_tensor_attr outputs_attr[10];

	if(BehaviorConf::Instance()->getLogLevel() >= SPDLOG_LEVEL_OFF)
	{
		return;
	}
	ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &querynum, sizeof(rknn_input_output_num));
	if(ret < 0) {
		spdlog::error("rknn_query num! ret={}", ret);
	    return;
	}
	printf("input num = %d, output num = %d\n",querynum.n_input, querynum.n_output);
	inputs_attr[0].index = 0;
	outputs_attr[0].index = 0;
	outputs_attr[1].index = 1;
	ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(inputs_attr[0]), sizeof(inputs_attr[0]));
	if(ret < 0) {
		spdlog::error("rknn_query fail! ret={}", ret);
	    return;
	}
	printf("intput ---------------start\n");
	printf("n_dims = %d\n",inputs_attr[0].n_dims);
	printf("dims array =");
	for(uint32_t j = 0; j < inputs_attr[0].n_dims; j++)
	{
		printf(" %d",inputs_attr[0].dims[j]);
	}
	printf("\n");
	printf("name = %s\n",inputs_attr[0].name);
	printf("n_elems = %d\n",inputs_attr[0].n_elems);
	printf("size = %d\n",inputs_attr[0].size);
	printf("fmt = %d\n",inputs_attr[0].fmt);
	printf("type = %d\n",inputs_attr[0].type);
	printf("qnt_type = %d\n",inputs_attr[0].qnt_type);
	printf("fl = %d\n",inputs_attr[0].fl);
	printf("zp = %d\n",inputs_attr[0].zp);
	printf("scale = %f\n",inputs_attr[0].scale);
	printf("intput ---------------end\n");

	// 本身是需要查每个output的，但是so不让查大于2以后的output
	uint32_t queryCnt = querynum.n_output > 2 ? 2 : querynum.n_output;
	//for(uint32_t i = 0; i < querynum.n_output; i++)
	for(uint32_t i = 0; i < queryCnt; i++)
	{
		ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[i]), sizeof(outputs_attr[i]));
		if(ret < 0) {
			spdlog::error("rknn_query fail! ret={}", ret);
		    return;
		}

		printf("output[%d] ---------------start\n",i);
		printf("n_dims = %d\n",outputs_attr[i].n_dims);
		printf("dims array =");
		for(uint32_t j = 0; j < outputs_attr[i].n_dims; j++)
		{
			printf(" %d",outputs_attr[i].dims[j]);
		}
		printf("\n");
		printf("name = %s\n",outputs_attr[i].name);
		printf("n_elems = %d\n",outputs_attr[i].n_elems);
		printf("size = %d\n",outputs_attr[i].size);
		printf("fmt = %d\n",outputs_attr[i].fmt);
		printf("type = %d\n",outputs_attr[i].type);
		printf("qnt_type = %d\n",outputs_attr[i].qnt_type);
		printf("fl = %d\n",outputs_attr[i].fl);
		printf("zp = %d\n",outputs_attr[i].zp);
		printf("scale = %f\n",outputs_attr[i].scale);

		printf("output[%d] ---------------end\n",i);
	}
}
