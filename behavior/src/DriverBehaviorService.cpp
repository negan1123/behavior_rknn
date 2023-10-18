/*
 * DriverBehaviorService.cpp
 *
 *  Created on: 2021年3月15日
 *      Author: syukousen
 */
#include <thread>
#include <BehaviorConf.hpp>
#include "DriverBehaviorService.h"


#include<iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

DriverBehaviorService::DriverBehaviorService()
{
	mRetinaDriver = std::make_unique<RetinaDriver>();
	mUploadBehavior = std::make_unique<UploadBehavior>(BehaviorConf::Instance()->getServerHost(),BehaviorConf::Instance()->getServerPort(),BehaviorConf::Instance()->getPicVideoPath());
	thresholdOne = BehaviorConf::Instance()->getThresholdOne();
	thresholdTwo = BehaviorConf::Instance()->getThresholdTwo();
	localBus = std::make_unique<LocalBus>();
	localBus -> setOnRecieveMsg([&](const json &js){
		std::string spd = js.at("speed");
		speed = std::stof(spd);
	});
	localBus->init();
	ce = std::make_unique<CloseEye>();
	llr = std::make_unique<LookLeftRight>();
	ls = std::make_unique<LookStraight>();
	onDetect f = [&](const std::string& pic_path){
		mUploadBehavior->queueBehavior(pic_path);
	};
	ce->setOnDetect(f);
	llr->setOnDetect(f);
	ls->setOnDetect(f);
}

void DriverBehaviorService::enqueueMat(cv::Mat & detect)
{
	// 要克隆，不然一直是浅拷贝
	cv::Mat imgclone = detect.clone();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	auto nowMs = ms.count();
	std::lock_guard<std::mutex> lck (mtx);
	if(detectMatQueue.size() < 2)
	{
		if(!detectMatQueue.empty())
		{
			//std::cout<<"上一个图片入队时间:"<<detectMatQueue.back().enTime<<std::endl;
			//距离上一张图片要有200毫秒，这样1s大概放5张图
			if(nowMs - detectMatQueue.back().enTime  < 200)
			{
				spdlog::info("入队时间小于200毫秒,当前时间:{}",nowMs);
				return;
			}
		}
		struct picQueue pQueue;
		pQueue.enTime = nowMs;
		//硬解码，入队做色彩空间转换yuv420sp to rgb
		if(BehaviorConf::Instance()->isMppEnable())
		{
			cv::Mat rgbImg(imgclone.rows, imgclone.cols, CV_8UC3);
			cv::cvtColor(imgclone, rgbImg, CV_YUV420sp2RGB);
			pQueue.setImg(rgbImg);
		}
		else
		{
			pQueue.setImg(imgclone);
		}
		detectMatQueue.push(pQueue);
		spdlog::debug("当前时间:{} 有图片入队",nowMs);
	}
}

void DriverBehaviorService::start()
{
	mUploadBehavior->start();

	// 开启一个线程进行上传图像
	std::thread thread1([&]() {
		int isSavePic = BehaviorConf::Instance()->isSaveDetectPic();
		while(1)
		{
			int ret = 0;
			std::string timestr;
			bool haveImage = false;
			time_t t;
			cv::Mat detectimg;
			float spd = 0.0;
			spd = speed.load();

			struct picQueue tmpQue;

			// 从队列里面获取要发送的图片名称
			mtx.lock();
			if(!detectMatQueue.empty())
			{
				haveImage = true;
				tmpQue = detectMatQueue.front();
				detectMatQueue.pop();
			}
			mtx.unlock();
			if(!tmpQue.img.empty())
			{
				time_t nowTime = time(NULL);
				//出队时间减入队时间超过1s不做检测
				if(nowTime*1000 - tmpQue.enTime > 1000)
					continue;
				detectimg = tmpQue.img.clone();
				if(BehaviorConf::Instance()->isSaveDecodeImg())
					cv::imwrite(BehaviorConf::Instance()->getDecodeImgPath()+"/detectPre_"+std::to_string(nowTime)+".jpg", detectimg);
			}

			if(haveImage && (spd > thresholdOne))
			{
				t = time(NULL);
				timestr = std::to_string(t);
				std::string picPath = BehaviorConf::Instance()->getPicVideoPath();

				// 输入图像开始检测
				mRetinaDriver->input(detectimg);
				ret = mRetinaDriver->inference();
				if(0 == ret && isSavePic)
				{
					int behaviorInt = mRetinaDriver->getBehaviorInt();
					int close_eye_int = 0;
					close_eye_int = behaviorInt & 1;
//					behaviorInt &= 1;
					if(close_eye_int)
					{
						ce->inference(t, mRetinaDriver->getCloseEyeLabelledImage());
					}
					float y, p, r;
					mRetinaDriver->getAngle(y, p, r);
					if(spd >= thresholdTwo){
						llr->inference(t, mRetinaDriver->getLookLeftRightLabelledImage(), y, p);
					}
					ls->inference(t, mRetinaDriver->getLookStraightLabelledImage(), y, p);
					// 测试用，保存打哈欠，抽烟等行为图片
//					int yawn_int = 0;
//					int smoke_int = 0;
//					int phone_int = 0;
//					int drink_int = 0;
//					yawn_int = behaviorInt & 2;
//					smoke_int = behaviorInt & 4;
//					phone_int = behaviorInt & 8;
//					drink_int = behaviorInt & 16;
//					if(yawn_int)
//					{
//						cv::imwrite("../tempPics/" + timestr + "_yawn.jpg", detectimg);
//					}
//					if(smoke_int)
//					{
//						cv::imwrite("../tempPics/" + timestr + "_smoke.jpg", detectimg);
//					}
//					if(phone_int)
//					{
//						cv::imwrite("../tempPics/" + timestr + "_phone.jpg", detectimg);
//					}
//					if(drink_int)
//					{
//						cv::imwrite("../tempPics/" + timestr + "_drink.jpg", detectimg);
//					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(10));		///< 推理，上传一次之后休息一下，减轻cpu负荷
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(500));	///< 等待采集器抓取图片
			}
		}
	});
	thread1.detach();
}

void DriverBehaviorService::testpic(std::string path)
{
	std::string dir = path;
	std::vector<std::string> dirlist;
	struct dirent* ent(0);
	DIR* pDir(opendir(dir.c_str()));

	if(pDir == NULL)
	{
		printf("dir err\n");
		return;
	}
	// 查找设备目录下的所有天数文件加，保存在dirlist里
	while ((ent = readdir(pDir)) != 0)
	{
		/**在Linux文件系统中 .和..也是特殊的子目录*/
		if(strstr(ent->d_name,".jpg") != NULL)
		{
			dirlist.push_back(std::string(ent->d_name));
		}
	}
	closedir(pDir);

	for(auto it = dirlist.begin(); it != dirlist.end(); it++)
	{
		std::string filePath = path + "/" + *it;
		float y,p,r;
		cv::Mat detectimg = cv::imread(filePath, cv::IMREAD_COLOR);
		mRetinaDriver->input(detectimg);
		mRetinaDriver->inference();
		mRetinaDriver->getAngle(y,p,r);
		printf("%s\n",filePath.c_str());
		printf("[%f %f %f]\n",y,p,r);
	}
}

std::string DriverBehaviorService::getRknnApiVer()
{
	return mRetinaDriver->getRknnApiVer();
}

std::string DriverBehaviorService::getRknnDrvVer()
{
	return mRetinaDriver->getRknnDrvVer();
}
