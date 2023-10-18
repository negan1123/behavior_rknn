/*
 * RetinaDriver.cpp
 *
 *  Created on: 2020年12月29日
 *      Author: syukousen
 */
#include <thread>
#include <spdlog/spdlog.h>
#include <RetinaDriverConf.hpp>
#include "RetinaDriver.h"
#include <BehaviorConf.hpp>
#include <stdio.h>

extern long long what_time_is_it_now();
std::string behaviorStr[] = {"closeEye","yawn","smoke","phone","drink","blockCamera","lookrightleft","other"};

RetinaDriver::RetinaDriver()
{
	// 必须先加载conf, 后面的实例构造的时候会用到配置参数
	loadConfig();
	phDetector = std::make_unique<PersonHeadDetector>();
//	dbDetector = std::make_unique<DriverBehaviorDetector>();
	lrlDetector = std::make_unique<LookLeftRightDetector>();
	hbDetector = std::make_unique<HeadBehaviorDetector>();
	bbDetector =  std::make_unique<BodyBehaviorDetector>();
//	lrlDetector = std::make_unique<LookRightLeftDetector>();
}

RetinaDriver::~RetinaDriver()
{

}

int RetinaDriver::input(cv::Mat &image)
{
	// 为了不影响输入的图片，这里克隆一份，cv::Mat是浅拷贝的
	inImage = image.clone();
	return 0;
}

int RetinaDriver::inference(void)
{
	int ret = 0;
	behavior = {false,false,false,false,false,false,false,false};
	long long start_time,end_time;
	start_time = what_time_is_it_now();
	ret = phDetector->input(inImage);
	if(0 != ret)
	{
		spdlog::debug("phDetector intput fail! ret={}", ret);
		return -1;
	}
	ret = phDetector->detect();
	if(0 != ret)
	{
		spdlog::debug("phDetector detect fail! ret={}", ret);
		return -1;
	}
	ret = phDetector->createPersonHeadImage();
	if(0 != ret)
	{
		spdlog::debug("phDetector createPersonHeadImage fail! ret={}", ret);
		return -1;
	}
	// 获取人像和头像，必须检测是否为空
	cv::Mat person = phDetector->getPersonImage();
	cv::Mat head = phDetector->getHeadImage();

	// 行为检测前置条件
	ret = preconditionCheck();
	if(0 != ret)
	{
		spdlog::debug("preconditionCheck fail! ret={}", ret);
		return -1;
	}

	if(!person.empty())
	{
		ret = bbDetector->input(person);
		ret = bbDetector->detect();
		ret = bbDetector->classifyBehavior();
		int temp;
		temp = bbDetector->getBehavior();
		if(temp == 1)
			behavior.drink = true;
		else if(temp == 2)
			behavior.phone = true;
		else if(temp == 3)
			behavior.smoke = true;
	}
	else
	{
		spdlog::debug("getPersonImage fail!");
		return -1;
	}

	if(!head.empty())
	{
		ret = hbDetector->input(head);
		ret = hbDetector->detect();
		ret = hbDetector->classifyBehavior();
		HeadBehavior temp;
		temp = hbDetector->getBehavior();
		behavior.closeEye = temp.close_eye;
		behavior.yawn = temp.yawn;

		std::string pic_path(BehaviorConf::Instance()->getPicVideoPath());
		pic_path = pic_path + "/forRockX.jpg";
		cv::imwrite(pic_path, inImage);
		ret = lrlDetector->input(pic_path);
		ret = lrlDetector->detect();
		remove(pic_path.data());
//		behavior.lookrightleft = lrlDetector->isLookRightLeft();

	}
	else
	{
		// 检测出来人身体，但是没有头像，判断为其它行为
		behavior.other = true;
	}

	end_time = what_time_is_it_now();
	spdlog::info("dbdetector elapse = {}", end_time-start_time);
	return 0;
}

int RetinaDriver::loadConfig(void)
{
	RetinaDriverConf::Instance();
	return 0;
}

cv::Mat &RetinaDriver::getLabelledImage(bool showPersonBox, bool showHeadBox)
{
	cv::Rect person = phDetector->getPersonBox();
	cv::Rect head = phDetector->getHeadBox();
	int drawTxtIndex = 0;
	int txtX,TxtY;
	if(!person.empty() && showPersonBox)
	{
		cv::rectangle(inImage, person, cv::Scalar(0,0,255));
	}
	if(!head.empty() && showHeadBox)
	{
		cv::rectangle(inImage, head, cv::Scalar(0,255,0));
	}
	txtX = person.x;
	TxtY = person.y + 30;
	if(behavior.closeEye)
	{
		cv::putText(inImage,behaviorStr[0], cv::Point(txtX,TxtY), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	if(behavior.yawn)
	{
		cv::putText(inImage,behaviorStr[1], cv::Point(txtX,TxtY+drawTxtIndex*30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	if(behavior.smoke)
	{
		cv::putText(inImage,behaviorStr[2], cv::Point(txtX,TxtY+drawTxtIndex*30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	if(behavior.phone)
	{
		cv::putText(inImage,behaviorStr[3], cv::Point(txtX,TxtY+drawTxtIndex*30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	if(behavior.drink)
	{
		cv::putText(inImage,behaviorStr[4], cv::Point(txtX,TxtY+drawTxtIndex*30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	if(behavior.blockCamera)
	{
		cv::putText(inImage,behaviorStr[5], cv::Point(txtX,TxtY+drawTxtIndex*30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	if(behavior.lookrightleft)
	{
		cv::putText(inImage,behaviorStr[6], cv::Point(txtX,TxtY+drawTxtIndex*30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	if(behavior.other)
	{
		cv::putText(inImage,behaviorStr[7], cv::Point(txtX,TxtY+drawTxtIndex*30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
		drawTxtIndex++;
	}
	return inImage;
}

cv::Mat RetinaDriver::getCloseEyeLabelledImage(bool showPersonBox, bool showHeadBox)
{
	cv::Rect person = phDetector->getPersonBox();
	cv::Rect head = phDetector->getHeadBox();
	cv::Mat img = inImage.clone();
	int txtX,TxtY;
	if(!person.empty() && showPersonBox)
	{
		cv::rectangle(img, person, cv::Scalar(0,0,255));
	}
	if(!head.empty() && showHeadBox)
	{
		cv::rectangle(img, head, cv::Scalar(0,255,0));
	}
	txtX = person.x;
	TxtY = person.y + 30;
	cv::putText(img,behaviorStr[0], cv::Point(txtX,TxtY), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
	return img;
}

cv::Mat RetinaDriver::getLookLeftRightLabelledImage(bool showPersonBox, bool showHeadBox)
{
	cv::Rect person = phDetector->getPersonBox();
	cv::Rect head = phDetector->getHeadBox();
	cv::Mat img = inImage.clone();
	int txtX,TxtY;
	if(!person.empty() && showPersonBox)
	{
		cv::rectangle(img, person, cv::Scalar(0,0,255));
	}
	if(!head.empty() && showHeadBox)
	{
		cv::rectangle(img, head, cv::Scalar(0,255,0));
	}
	txtX = person.x;
	TxtY = person.y + 30;
	cv::putText(img,behaviorStr[6], cv::Point(txtX,TxtY), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
	return img;
}

cv::Mat RetinaDriver::getLookStraightLabelledImage(bool showPersonBox, bool showHeadBox)
{
	cv::Rect person = phDetector->getPersonBox();
	cv::Rect head = phDetector->getHeadBox();
	cv::Mat img = inImage.clone();
	int txtX,TxtY;
	if(!person.empty() && showPersonBox)
	{
		cv::rectangle(img, person, cv::Scalar(0,0,255));
	}
	if(!head.empty() && showHeadBox)
	{
		cv::rectangle(img, head, cv::Scalar(0,255,0));
	}
	txtX = person.x;
	TxtY = person.y + 30;
	std::string ls("lookstraight");
	cv::putText(img, ls, cv::Point(txtX,TxtY), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0,0,255),2);
	return img;
}

int RetinaDriver::getBehaviorInt(void)
{
	int ret = 0;
	if(behavior.closeEye)
		ret = 1;
	if(behavior.yawn)
		ret = (1 << 1) | ret;
	if(behavior.smoke)
		ret = (1 << 2) | ret;
	if(behavior.phone)
		ret = (1 << 3) | ret;
	if(behavior.drink)
		ret = (1 << 4) | ret;
//	if(behavior.blockCamera)
//		ret = (1 << 5) | ret;
//	if(behavior.lookrightleft)
//		ret = (1 << 6) | ret;
//	if(behavior.other)
//		ret = (1 << 7) | ret;
	return ret;
}
#if 1	//测试用，将截取的图像保存出来
cv::Mat RetinaDriver::getPersonImage(void)
{
	return phDetector->getPersonImage();
}

cv::Mat RetinaDriver::getHeadImage(void)
{
	return phDetector->getHeadImage();
}
#endif

std::string RetinaDriver::getRknnApiVer()
{
	return phDetector->getRknnApiVer();
}
std::string RetinaDriver::getRknnDrvVer()
{
	return phDetector->getRknnDrvVer();
}

int RetinaDriver::preconditionCheck()
{
	cv::Mat person = phDetector->getPersonImage();
	cv::Mat head = phDetector->getHeadImage();

	// 行为检测前置条件
	if(person.empty())
		return -1;
	if(head.empty())
		return -2;
	if(head.cols*head.rows > person.cols*person.rows*0.3)
		return -3;

	cv::Rect personRect = phDetector->getPersonBox();
	cv::Rect headRect = phDetector->getHeadBox();
	int x1p, x2p, y1p, y2p;
	int x1h, x2h, y1h, y2h;
	x1p = personRect.x;
	y1p = personRect.y;
	x2p = x1p + personRect.width - 1;
	y2p = y1p + personRect.height - 1;
	x1h = headRect.x;
	y1h = headRect.y;
	x2h = x1h + headRect.width - 1;
	y2h = y1h + headRect.height - 1;
	int x1o, y1o, x2o, y2o;
	x1o = std::max(x1p, x1h);
	y1o = std::max(y1p, y1h);
	x2o = std::min(x2p, x2h);
	y2o = std::min(y2p, y2h);
	if(x1o >= x2o || y1o >= y2o)
		return -4;
	else if((x2o - x1o + 1)*(y2o - y1o + 1) < 0.9*headRect.width*headRect.height)
		return -5;

	return 0;
}
