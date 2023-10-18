/*
 * PersonHeadDetector.cpp
 *
 *  Created on: 2020年12月22日
 *      Author: syukousen
 */
#include <fstream>
#include <iostream>
#include <cstdint>
#include <memory>
#include <RetinaDriverConf.hpp>
#include <spdlog/spdlog.h>
#include "PersonHeadDetector.h"

// 供std::max_elemen函数用的比对函数，求出人像列表和头像列表的置信度最高的box
bool cmp(ConfInfo a,ConfInfo b)
{
      return a.value < b.value;
}

struct SeetaRect{
    int x;
    int y;
    int width;
    int height;
};

template <typename T>
T CLAMP(T value, T min_value, T max_value)
{
    T result = std::max(min_value, std::min(value, max_value));

    return result;
}

static SeetaRect adjust_rect(const int &limit_width, const int &limit_height,
const SeetaRect &face)
{
    const float width_scale = 1.0;
    const float height_scale = 1.2;

    float x_centor = face.x + face.width / 2;
    float y_centor = face.y +face.height / 2;
    float width = std::sqrt((float)face.width * face.height);
    float height = 1.2 * width;

    float x_min = x_centor - width / 2;
    x_min = CLAMP((float)x_min, (float)0, float(limit_width - 1));

    float y_min = y_centor - height / 2;
    y_min = CLAMP((float)y_min, (float)0, float(limit_height -1));

    float x_max = x_centor + width / 2;
    x_max = CLAMP((float)x_max, (float)0, float(limit_width -1));

    float y_max = y_centor + height / 2;
    y_max = CLAMP((float)y_max, (float)0, (float)limit_height -1);

    SeetaRect result;
    result.x = x_min;
    result.y = y_min;
    result.width = x_max - x_min;
    result.height = y_max - y_min;

    return result;
}

//转换系数：
//[scale_w, scale_h, scale_x, scale_y]: [0.923038, 1.14125, -0.0108451, -0.0945746]

static SeetaRect convert_to_target(const int &limit_width, const int &limit_height,
                        const SeetaRect &face,
                        float x_scale, float y_scale, float w_scale, float h_scale)
{

    SeetaRect rect = face;

    rect.x += int(x_scale * rect.width);
    rect.y += int(y_scale * rect.height);
    rect.width = int(rect.width * w_scale);
    rect.height = int(rect.height * h_scale);
    int x1 = CLAMP(rect.x, 0, limit_width - 1);
    int y1 = CLAMP(rect.y, 0, limit_height - 1);
    int x2 = CLAMP(rect.x + rect.width - 1, 0, limit_width - 1);
    int y2 = CLAMP(rect.y + rect.height - 1, 0, limit_height - 1);
    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;

    rect.x = x1;
    rect.y = y1;
    rect.width = w;
    rect.height = h;

    return rect;
}

PersonHeadDetector::PersonHeadDetector()
{
	std::string var1,var2,var3;
	std::stringstream istring;

	preProcessImageW = 1280;
	preProcessImageH = 736;
	mean = {104,117,123};
	// 读取variance
	istring << RetinaDriverConf::Instance()->RETINATK_VARIANCE;
	std::getline(istring,var1,',');
	std::getline(istring,var2,',');
	variance[0] = std::stof(var1);
	variance[1] = std::stof(var2);

	// 读取均值
	istring.clear();
	istring.str("");
	istring << RetinaDriverConf::Instance()->RETINATK_MEAN;
	std::getline(istring,var1,',');
	std::getline(istring,var2,',');
	std::getline(istring,var3,',');
	mean[0] = std::stoi(var1);
	mean[1] = std::stoi(var2);
	mean[2] = std::stoi(var3);

	confThresh = std::stof(RetinaDriverConf::Instance()->RETINATK_CONF_THRESH);
	init(RetinaDriverConf::Instance()->RETINATK_RKNN_PATH);
	loadPriors(RetinaDriverConf::Instance()->RETINATK_PRIORS_PATH);

	rknn_sdk_version version;
	int ret =0;
	ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
	if(ret != 0){
		std::cout << "获取版本失败" << std::endl;
	}
	else
	{
		for(int i=0; i<5; ++i){
			apiVersion.push_back(version.api_version[i]);
			drvVersion.push_back(version.drv_version[i]);
		}
//		std::cout << apiVersion <<std::endl;
//		std::cout << drvVersion <<std::endl;
	}
}

PersonHeadDetector::~PersonHeadDetector()
{
	if(nullptr != priorsdata)
	{
		delete []priorsdata;
	}
}

void PersonHeadDetector::showPriors(void)
{
	if(nullptr != priorsHead)
	{
		spdlog::info("dimenson = {}",priorsHead[0]);
		spdlog::info("frist dimenson len = {}",priorsHead[1]);
		spdlog::info("second dimenson len = {}",priorsHead[2]);
	}
}

int PersonHeadDetector::createPersonHeadImage(void)
{
	if(createDetectBox() != 0)
		return -1;
	//将头像和人像清空，外部获取头像和人像检测成功与否，是判断图像是否为空
	headImage = cv::Mat();
	personImage = cv::Mat();
	// 算出满足要求置信度的body和head，并再原始输入图像中抓取图像
	if(headBox.value > confThresh)
	{
		//headImage = inImage(getHeadBox());
		// 新的左顾右盼算法，图像坐标需要做如下转换
		cv::Rect preRect = getHeadBox();
		SeetaRect convertRect{preRect.x, preRect.y, preRect.width, preRect.height};
		SeetaRect current_to_target = convert_to_target(inImage.cols, inImage.rows, convertRect, -0.0108451, -0.0945746, 0.923038, 1.14125);
		current_to_target = adjust_rect(inImage.cols, inImage.rows, current_to_target);
		preRect = cv::Rect(current_to_target.x, current_to_target.y, current_to_target.width, current_to_target.height);
		headImage = inImage(preRect);
	}

	if(personBox.value > confThresh)
	{
		personImage = inImage(getPersonBox());
	}
	return 0;
}

cv::Mat PersonHeadDetector::getPersonImage(void)
{
	return personImage;
}

cv::Mat PersonHeadDetector::getHeadImage(void)
{
	return headImage;
}

int PersonHeadDetector::loadPriors(std::string fileName)
{

	size_t len = 0;
	std::ifstream is (fileName.c_str(), std::ios::in | std::ios::binary);
	if (is)
	{
		// 获取文件大小
		is.seekg (0, is.end);
		len = is.tellg();
		is.seekg (0, is.beg);

		if(nullptr != priorsdata)
		{
			delete[]priorsdata;
			priorsdata = nullptr;
		}
		priorsdata = new char[len];
		is.read ((char*)priorsdata,len);
		if (!is)
		{
			spdlog::error("read priors err");
			delete[]priorsdata;
			priorsdata = nullptr;
			is.close();
			return -1;
		}
		is.close();
		/*
		地址	长度(byte)	类型	内容	描述
		0	4	int	2	priors文件体及位置数据的维数，二维数据
		4	4	int	38640	第一个维度的长度
		8	4	int	4	第二个维度的长度，两个维度表示38640*4
		地址	长度(byte)	类型	内容	描述
		12	4	float	priors[0][0]	第一行数据
		16	4	float	priors[0][1]
		20	4	float	priors[0][2]
		24	4	float	priors[0][3]
		......
		*/
		priorsHead = (int *)priorsdata;
		priors = (float (*)[4])(priorsHead+3);
	}
	else
	{
		spdlog::error("open priors.bin err");
		return -1;
	}

	return 0;
}

int PersonHeadDetector::createDetectBox(void)
{
	if(outData.empty())
	{
		spdlog::error("person head rknn outData empty");
		return -1;
	}
	float conf_value[3] = {0.0};
	float *outpos = ((float *)outData[0]);
	float *outprior = ((float *)outData[1]);
	headConfList.clear();
	personConfList.clear();
	headBox.value = 0;
	personBox.value = 0;
#if 0
	std::ofstream outconf,outloc;
	outconf.open("outconf.csv");
	outloc.open("outloc.csv");
	if(!outconf.is_open() || !outconf.is_open()) {
	    // error and exit here
	}
	outconf.precision(8);
	outloc.precision(8);
	for(int i= 0; i < 38640; i++)
	{
		outconf << *outprior++ << "," << *outprior++ << "," << *outprior++  << std::endl;
		outloc << *outpos++ << "," << *outpos++ << "," << *outpos++ << "," << *outpos++ << std::endl;
	}
	outconf.close();
	outloc.close();
	outpos = ((float *)outData[0]);
	outprior = ((float *)outData[1]);
#endif
	// 归类body和head
	for(int conf_index = 0; conf_index < 38640; conf_index++)
	{
		ConfInfo info;
		info.boxId = conf_index;
		conf_value[0] = *(outprior+conf_index*3);
		conf_value[1] = *(outprior+conf_index*3+1);
		conf_value[2] = *(outprior+conf_index*3+2);
		if(conf_value[1] > conf_value[2])
		{
			info.value = conf_value[1];
			personConfList.push_back(info);
		}
		else
		{
			info.value = conf_value[2];
			headConfList.push_back(info);
		}
	}

	// 求出得分最好的body和head
	ConfInfo maxhead = *std::max_element(headConfList.begin(),headConfList.end(),cmp);
	ConfInfo maxperson = *std::max_element(personConfList.begin(),personConfList.end(),cmp);
	float (*headpriors)[4] = priors+maxhead.boxId;
	float (*personpriors)[4] = priors+maxperson.boxId;
	float *headposition = outpos+maxhead.boxId*4;
	float *personposition = outpos+maxperson.boxId*4;
	/*	计算公式
		先验框位置d=(dx, dy, dw, dh)
		真实框位置g=(gx, gy, gw, gh)
		priors行数据=(priors[0] , priors[1], priors[2], priors[3])
		variance=(variance[0], variance[1])=(0.1, 0.2)

		gx = priors[0] + dx * variance[0] * priors[2]
		gy = priors[1] + dy * variance[0] * priors[3]
		gw = priors[2]*exp(dw * variance[1])
		gh = priors[3]*exp(dh * variance[1])
	 */
	// 计算body和head的位置信息
	headBox.x = (*headpriors)[0] + (*headposition) * variance[0] * (*headpriors)[2];
	headBox.y = (*headpriors)[1] + (*(headposition+1)) * variance[0] * (*headpriors)[3];
	headBox.w = (*headpriors)[2] * exp((*(headposition+2)) * variance[1]);
	headBox.h = (*headpriors)[3] * exp((*(headposition+3)) * variance[1]);
	headBox.value = maxhead.value;
	headBox.boxId = maxhead.boxId;
	spdlog::info("headBoxInfo-------------------------------->");
	spdlog::info("headBox.x = {}",headBox.x);
	spdlog::info("headBox.y = {}",headBox.y);
	spdlog::info("headBox.w = {}",headBox.w);
	spdlog::info("headBox.h = {}",headBox.h);
	spdlog::info("headBox.value = {}",headBox.value);
	spdlog::info("headBox.boxId = {}",headBox.boxId);
	spdlog::info("headBoxInfo--------------------------------<");

	personBox.x = (*personpriors)[0] + (*personposition) * variance[0] * (*personpriors)[2];
	personBox.y = (*personpriors)[1] + (*(personposition+1)) * variance[0] * (*personpriors)[3];
	personBox.w = (*personpriors)[2] * exp((*(personposition+2)) * variance[1]);
	personBox.h = (*personpriors)[3] * exp((*(personposition+3)) * variance[1]);
	personBox.value = maxperson.value;
	personBox.boxId = maxperson.boxId;

	spdlog::info("personBoxInfo-------------------------------->");
	spdlog::info("personBox.x = {}",personBox.x);
	spdlog::info("personBox.y = {}",personBox.y);
	spdlog::info("personBox.w = {}",personBox.w);
	spdlog::info("personBox.h = {}",personBox.h);
	spdlog::info("personBox.value = {}",personBox.value);
	spdlog::info("personBox.boxId = {}",personBox.boxId);
	spdlog::info("personBoxInfo--------------------------------<");
	return 0;
}

int PersonHeadDetector::imgPreprocess(void)
{
	cv::Mat resimg;
	// 将输入的图片按照高，宽比例小的缩放，然后复制到1280*736的图像中
	rescale_ratio = std::min((float)preProcessImageW/inImage.cols, (float)preProcessImageH/inImage.rows);
	cv::resize(inImage, resimg, cv::Size(), rescale_ratio, rescale_ratio);
	preProcessImage = cv::Mat(preProcessImageH,preProcessImageW,CV_8UC3,cv::Scalar(0,0,0));
	resimg.copyTo(preProcessImage(cv::Rect(0,0,resimg.cols,resimg.rows)));
//	cv::imwrite("./person_preprocess.jpg",preProcessImage);
#ifndef RKNN_HAVE_MEAN_DEAL
	// 检测是否需要减去均值
	if(mean.size() == 3)
	{
		cv::Scalar scalar = cv::Scalar(mean[0], mean[1], mean[2]);
		preProcessImage = preProcessImage-scalar;
	}
#endif
	return 0;
}

int PersonHeadDetector::detect(void)
{
	int ret = 0;
	rknn_input inputs[1];
	rknn_output outputs[2];

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
	outputs[1].want_float = true;
	outputs[1].is_prealloc = false;
	outputs[1].index = 1;
	ret = rknn_outputs_get(ctx, 2, outputs, NULL);
	if (ret < 0) {
		spdlog::error("rknn_outputs_get fail! ret={}", ret);
		return -1;
	}
	/* outputs[0]
	地址		长度(byte)	类型	内容	描述
	0		4	float	dx	第一个位置数据
	4		4	float	dy
	8		4	float	dw
	12		4	float	dh
	...		4	float		共38640个位置数据
	outputs[1]
	地址		长度(byte)	类型	内容	描述
	0		4	float	背景得分	第一个位置对应得分
	4		4	float	person得分
	8		4	float	head得分
	...		4	float	共38640个置信度数据
	*/
	// 清除上次输出数据
	for(std::vector<void *>::const_iterator iter=outData.begin();iter!=outData.end();++iter)
	{
		char *ptr = (char*)*iter;
		delete []ptr;
	}
	outData.clear();

	// 保存输出，供createDetectBox检测人像和头像
	outData.push_back(new char[outputs[0].size]);
	memcpy(outData[0], outputs[0].buf, outputs[0].size);
	outData.push_back(new char[outputs[1].size]);
	memcpy(outData[1], outputs[1].buf, outputs[1].size);
	// 释放动态库里面申请的内存
	rknn_outputs_release(ctx, 2, outputs);
	return 0;
}

cv::Rect PersonHeadDetector::getPersonBox(void)
{
	cv::Rect ret(0,0,0,0);
	if(personBox.value > confThresh)
	{
		int x1 =(personBox.x-personBox.w/2.)*preProcessImageW/rescale_ratio;
		int x2=(personBox.x+personBox.w/2.)*preProcessImageW/rescale_ratio;
		int y1=(personBox.y-personBox.h/2.)*preProcessImageH/rescale_ratio;
		int y2=(personBox.y+personBox.h/2.)*preProcessImageH/rescale_ratio;
		// 坐标限定，检查越界
		if(x1 < 0)
			x1 = 0;
		if(x2 < 0)
			x2 = 0;
		if(y1 < 0)
			y1 = 0;
		if(y2 < 0)
			y2 = 0;

		if(x1 > inImage.cols)
			x1 = inImage.cols;
		if(x2 > inImage.cols)
			x2 = inImage.cols;
		if(y1 > inImage.rows)
			y1 = inImage.rows;
		if(y2 > inImage.rows)
			y2 = inImage.rows;
		ret = cv::Rect(x1,y1,x2-x1,y2-y1);
		spdlog::info("personImage rect = {},{},{},{}",x1,y1,x2-x1,y2-y1);
	}
	return ret;
}

cv::Rect PersonHeadDetector::getHeadBox(void)
{
	cv::Rect ret(0,0,0,0);
	if(headBox.value > confThresh)
	{
		int x1 =(headBox.x-headBox.w/2.)*preProcessImageW/rescale_ratio;
		int x2=(headBox.x+headBox.w/2.)*preProcessImageW/rescale_ratio;
		int y1=(headBox.y-headBox.h/2.)*preProcessImageH/rescale_ratio;
		int y2=(headBox.y+headBox.h/2.)*preProcessImageH/rescale_ratio;

		// 坐标限定，检查越界
		if(x1 < 0)
			x1 = 0;
		if(x2 < 0)
			x2 = 0;
		if(y1 < 0)
			y1 = 0;
		if(y2 < 0)
			y2 = 0;

		if(x1 > inImage.cols)
			x1 = inImage.cols;
		if(x2 > inImage.cols)
			x2 = inImage.cols;
		if(y1 > inImage.rows)
			y1 = inImage.rows;
		if(y2 > inImage.rows)
			y2 = inImage.rows;
		ret = cv::Rect(x1,y1,x2-x1,y2-y1);
		spdlog::info("headImage rect = {},{},{},{}",x1,y1,x2-x1,y2-y1);
	}
	return ret;
}

std::string PersonHeadDetector::getRknnApiVer()
{
	return apiVersion;
}

std::string PersonHeadDetector::getRknnDrvVer()
{
	return drvVersion;
}
