#include <DetectorService.h>

// 图片路径
#define DIRPATH "../testimg"

DetectorService::DetectorService()
{	
	int flag = 0;	// behavior程序状态 0正在加载模型 1加载成功
	mstatus = std::make_unique<BehaviorStatus>();
    mDetectorDriver = std::make_unique<DetectorDriver>();
	mUploadBehavior = std::make_unique<UploadBehavior>(BehaviorConf::Instance()->getServerHost(),
													   BehaviorConf::Instance()->getServerPort(),
													   BehaviorConf::Instance()->getPicVideoPath());
	thresholdOne = BehaviorConf::Instance()->getThresholdOne();

	localBus = std::make_unique<LocalBus>();
	localBus -> setOnRecieveMsg([&](const json &js){
		std::string spd = js.at("speed");
		speed = std::stof(spd);
	});
	localBus->init();

	mCloseEyes = make_unique<CloseEyes>();
	mCall = make_unique<Call>();
	mSmoke = make_unique<Smoke>();
	mBlockCamera = make_unique<BlockCamera>();
	mYawn = make_unique<Yawn>();
	mLookLeftRight = make_unique<lookLeftRight>();
	mLookStraight = make_unique<lookStraight>();

	// 上传器回调函数，将报警图片传入上传器队列
	onDetect f = [&](const std::string& pic_path)
	{
		mUploadBehavior->queueBehavior(pic_path);
	};
	// 设置回调函数
	mCloseEyes->setOnDetect(f);
	mCall->setOnDetect(f);
	mBlockCamera->setOnDetect(f);
	mYawn->setOnDetect(f);
	mLookLeftRight->setOnDetect(f);
	mLookStraight->setOnDetect(f);

	flag = 1;
	mstatus->setBehaviorFlag(flag);		// 向临时文件写入程序状态
}

DetectorService::~DetectorService()
{

}

void DetectorService::enqueueMat(cv::Mat & detect)
{
	cv::Mat imgclone = detect.clone();
	auto ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	auto nowMs = ms.count();	// 获取当前毫秒级时间

	lock_guard<std::mutex> lck (mtx);
	if (detectImageList.size() < 2)
	{
		if (!detectImageList.empty())
		{
			//距离上一张图片要有200毫秒，这样1s大概放5张图
			if(nowMs - detectImageList.back().enTime  < 200)
			{
				spdlog::info("入队时间小于200毫秒,当前时间:{}",nowMs);
				// cout<<"入队时间小于200毫秒,当前时间:"<<nowMs<<endl;
				return;
			}
		}
		detectImgInfo imgInfo;				// 创建检测图片结构体
		imgInfo.enTime = nowMs;				// 初始化入队时间
		cv::Mat imgclone = detect.clone();	// 深拷贝图片
		imgInfo.name = "rtsp";				// 初始化图片名称
		imgInfo.img = imgclone;				// 初始化检测图片
		detectImageList.push(imgInfo);		// 结构体入队
		spdlog::debug("当前时间:{} 有图片入队",nowMs);
	}
}

void DetectorService::start()
{		
	mUploadBehavior->start();

	thread thread1([&]() {
		int isSavePic = BehaviorConf::Instance()->isSaveDetectPic();
		while(1)
		{
			int ret = 0;
			bool haveImage = false;
			time_t t;					// 初始化检测时间
			float spd = 0.0;
			spd = speed.load();		// 从mqtt获取速度
			string detectName;
			cv::Mat detectImg;
			detectImgInfo tmpInfo;

			// 把队列中的图片取出来
			mtx.lock();
			if(!detectImageList.empty())
			{
				haveImage = true;
				tmpInfo = detectImageList.front();
				detectImageList.pop();
				cout<<"当前图片数量："<<detectImageList.size()<<endl;
			}
			mtx.unlock();
			if (!tmpInfo.img.empty())
			{
				time_t nowTime = time(NULL);
				//出队时间减入队时间超过1s不做检测
				if(nowTime*1000 - tmpInfo.enTime > 1000)
					continue;
				detectImg = tmpInfo.img;
				detectName = tmpInfo.name;
			}

			// 成功取出图片后，开始进行检测
			if(haveImage && (spd > thresholdOne))
			{
				cout<<"---- img:"<<detectName<<" ----"<<endl;
				t = time(NULL);			// 获取当前检测时间
				int64_t start,end;
				start = util::curentTimeMillis();
				// 输入图像开始检测
				mDetectorDriver->input(detectImg);
				ret = mDetectorDriver->detect();
				// 成功推理后，开始作行为分类逻辑
				if(0 == ret && isSavePic)
				{
					int beh = mDetectorDriver->getBehaviorInt();	// 获取行为Int
					cout<<"当前行为："<<beh<<endl;
					cv::Mat classify;
					if (beh == 1)	// 闭眼
					{
						classify = mDetectorDriver->getLabelImage(true);
						mCloseEyes->inference(t, classify);
					}
					else if (beh >1 && beh < 4)	// 打呵欠
					{
						classify = mDetectorDriver->getLabelImage(true);
						mYawn->inference(t, classify);
					}
					else if (beh > 3 && beh < 8)	// 抽烟
					{
						classify = mDetectorDriver->getLabelImage(true);
						mSmoke->inference(t, classify);
					}
					else if (beh > 7 && beh < 16)	// 打电话
					{
						classify = mDetectorDriver->getLabelImage(true);
						mCall->inference(t, classify);
					}
					else if(beh > 15)	// 遮挡摄像头
					{
						classify = mDetectorDriver->getLabelImage(true);
						mBlockCamera->inference(t, classify);
					}
					else
					{
						// 人脸偏转角
						faceAngle angle;				// 人脸欧拉角
						vector<landmark> landmarks;		// 人脸关键点
						faceBox box;					// 人脸框
						mDetectorDriver->getFaceResult(angle, landmarks, box);	// 获取人脸信息推理结果
						if (angle.yaw != -1000)
						{
							float y,p,r;
							y = angle.yaw;
							p = angle.pitch;
							r = angle.roll;
							cout<<"y,p,r:"<<y<<","<<p<<","<<r<<endl;
							cv::Mat LLR = mDetectorDriver->getLookLeftRightLabelImage(true);
							mLookLeftRight->inference(t, LLR, y, p);
							cv::Mat LS = mDetectorDriver->getLookStraightLableImage(true);
							mLookStraight->inference(t, LS, y, p);
						}
					}
					// if (!classify.empty())
						// cv::imwrite("../img/classify.jpg",classify);
				}
				end = util::curentTimeMillis();
				spdlog::info("detect elapse = {}", end-start);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));		///< 推理，上传一次之后休息一下，减轻cpu负荷
			}
			else 
				std::this_thread::sleep_for(std::chrono::milliseconds(10));	///< 等待采集器抓取图片
		}
	});
	thread1.detach();
}


std::string DetectorService::getRknnApiV()
{
	return mDetectorDriver->getRknnApiVer();
}

std::string DetectorService::getRknnDrvV()
{
	return mDetectorDriver->getRknnDrvVer();
}