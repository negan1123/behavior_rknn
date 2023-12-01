#include <DetectorDriver.h>

string behaviorStr[] = {"callPhone","closeEyes","yawn","lookLeftRight","lookStraight","blockCamera","smoke"};

DetectorDriver::DetectorDriver()
{
	// 必须先加载conf, 后面的实例构造的时候会用到配置参数
	loadConfig();
    HBDetector = make_unique<HeadBodyDetector>();
	FBDetector = make_unique<RockxDetector>();
	BlockCamera = make_unique<BlockCameraDetector>();
	SmokeDetector = make_unique<HeadBehaviorDetector>();
}

DetectorDriver::~DetectorDriver()
{

}

int DetectorDriver::loadConfig(void)
{
	RetinaDriverConf::Instance();
	return 0;
}

int DetectorDriver::input(cv::Mat &image)
{
	// 为了不影响输入的图片，这里克隆一份，cv::Mat是浅拷贝的
	detectImg = image.clone();
	cout<<"DetectorDriver input"<<endl;
	return 0;
}

int DetectorDriver::detect()
{   
	// 初始化beh
	beh = {false,false,false,false,false,false};

	int ret = 0;
	// blockCamera opencv检测
	cout<<"blockCamera opencv检测"<<endl;
	ret = BlockCamera->input(detectImg);
	ret = BlockCamera->detect();
	int block = BlockCamera->getBehavior();
	beh.blockCamera = bool(block);
	// 若没有遮挡摄像头，则进行目标检测
	if (block != 1) 
	{
		// 人脸身体图像分割
		cout<<"--> 人脸身体图像分割"<<endl;
		ret = HBDetector->input(detectImg);
		if(0 != ret)
		{
			cout<<"HeadBodyDetector intput fail! ret="<<ret<<endl;
			return ret;
		}
		ret = HBDetector->detect();
		if(0 != ret)
		{
			cout<<"HeadBodyDetector detect fail! ret="<<ret<<endl;
			return ret;
		}
		ret = HBDetector->createHeadBodyImage();
		if(0 != ret)
		{
			cout<<"HeadBodyDetector createHeadBodyImage fail! ret="<<ret<<endl;
			return ret;
		}
		// 头部身体图片
		cv::Mat HeadImg = HBDetector->getHeadImage();
		cv::Mat BodyImg = HBDetector->getBodyImage();

		// 行为检测前置条件
		ret = preconditionCheck();
		if(0 != ret)
		{
			spdlog::debug("preconditionCheck fail! ret={}", ret);
			return -1;
		}
		if (!HeadImg.empty())
		{	
			cout<<"--> 头部行为检测"<<endl;
			// smoke 检测
			ret = SmokeDetector->input(HeadImg);
			if(0 != ret)
			{
				cout<<"SmokeDetector intput fail! ret="<<ret<<endl;
				return ret;
			}
			ret = SmokeDetector->detect();
			if(0 != ret)
			{
				cout<<"SmokeDetector detect fail! ret="<<ret<<endl;
				return ret;
			}
			ret = SmokeDetector->getHeadBehavior();
			// 判断是否抽烟
			if(ret == 1)
			{
				beh.smoke = true;
			}
			
		}

		if (!BodyImg.empty())
		{
			cout<<"--> 身体行为检测"<<endl;
			string rockx_input = BehaviorConf::Instance()->getPicVideoPath();
			rockx_input = rockx_input + "/forRockX.jpg";
			cv::imwrite(rockx_input, BodyImg);
			// 进行头部行为识别，包括闭眼，打呵欠，左顾右盼，使用rockx人脸关键点
			ret = FBDetector->input(rockx_input);	// rockx图片输入
			if(0 != ret)
			{
				cout<<"FBDetector input fail! ret="<<ret<<endl;
				return ret;
			}
			ret = FBDetector->faceDetect();		// rockx推理
			remove(rockx_input.data());
			
			if(0 != ret)
			{
				cout<<"FBDetector detect fail! ret="<<ret<<endl;
				return ret;
			}

			faceAngle angle;				// 人脸欧拉角
    		vector<landmark> landmarks;		// 人脸关键点
			faceBox box;					// 人脸框
    		FBDetector->getResults(angle, landmarks, box);	// 获取人脸信息推理结果

			// 判断是否闭眼
			ret = FBDetector->isCloseEye(landmarks);
			if (ret == 1)
			{
				beh.closeEyes = true;
			}

			// 判断是否打呵欠
			ret = FBDetector->isYawn(landmarks);
			if (ret == 1)
			{
				beh.yawn = true;
			}

			// 进行身体识别，检测手机
			ret = HBDetector->getBodyBehavior();
			if(ret == 1)
			{
				beh.phone = true;
			}
		}

	}

	return 0;
}

int DetectorDriver::getBehaviorInt()
{
	/*
	* 00000 正常驾驶 0
	* 00001 闭眼 1
	* 00010 打呵欠 2
	* 00100 抽烟 4
	* 01000 打电话 8
	* 10000 遮挡摄像头 16
	*/
	int ret = 0;
	if(beh.closeEyes)
	{
		ret = 1;
	}
	if(beh.yawn)
	{
		ret += (1 << 1);
	}
	// if(beh.smoke)
	// {
	// 	ret += (1 << 2);
	// }
	if(beh.phone)
	{
		ret += (1 << 3);
	}
	if(beh.blockCamera)
	{
		ret += (1 << 4);
	}
	return ret;
}

cv::Mat DetectorDriver::getLabelImage(bool showLabelImage)
{
	cv::Mat img = detectImg.clone();	// 拷贝输入原图
	
	cv::Rect head = HBDetector->getHeadBox();// 获取head区域的外框
	cv::Rect body = HBDetector->getBodyBox();// 获取body区域的外框

	int drawTxtIndex = 0;
	int txtX,txtY;
	txtX = 0;
	txtY = 0 + 30;
	if (showLabelImage)
	{	
		if (beh.blockCamera)
		{
			cv::putText(img,behaviorStr[5], 
						cv::Point(txtX,txtY), 
						cv::FONT_HERSHEY_PLAIN, 
						2.0, 
						cv::Scalar(0,0,255),
						2);
			return img;
		}

		// 绘制图像
		cv::rectangle(img, head, cv::Scalar(0,0,255), 1, 1, 0); // 绘制头像框
		cv::rectangle(img, body, cv::Scalar(0,255,0), 1, 1, 0); // 绘制身体框
		if (beh.closeEyes)
		{
			//在图像上绘制文字
			cv::putText(img,behaviorStr[1], 
						cv::Point(txtX,txtY), 
						cv::FONT_HERSHEY_PLAIN, 
						2.0, 
						cv::Scalar(0,0,255),
						2);
			drawTxtIndex++;
		}
		if (beh.yawn)
		{
			cv::putText(img,behaviorStr[2], 
						cv::Point(txtX,txtY+drawTxtIndex*30), 
						cv::FONT_HERSHEY_PLAIN, 
						2.0, 
						cv::Scalar(0,0,255),
						2);
			drawTxtIndex++;
		}
		if (beh.phone)
		{
			cv::Rect phone = HBDetector->getPhoneBox();// 获取body区域的外框
			cv::rectangle(img, phone, cv::Scalar(0,255,0), 1, 1, 0); // 绘制身体框
			cv::putText(img,behaviorStr[0], 
						cv::Point(txtX,txtY+drawTxtIndex*30), 
						cv::FONT_HERSHEY_PLAIN, 
						2.0, 
						cv::Scalar(0,0,255),
						2);
			drawTxtIndex++;
		}
		if (beh.smoke)
		{
			cv::putText(img,behaviorStr[6], 
						cv::Point(txtX,txtY+drawTxtIndex*30), 
						cv::FONT_HERSHEY_PLAIN, 
						2.0, 
						cv::Scalar(0,0,255),
						2);
			drawTxtIndex++;
		}
	}
	return img;
}



cv::Mat DetectorDriver::getLookLeftRightLabelImage(bool showLabelImage)
{
	cv::Mat img = detectImg.clone();	// 拷贝输入原图
	cv::Rect head = HBDetector->getHeadBox();// 获取head区域的外框
	cv::Rect body = HBDetector->getBodyBox();// 获取body区域的外框

	int txtX,txtY;	
	if (showLabelImage)
	{
		cv::rectangle(img, head, cv::Scalar(0,0,255), 1, 1, 0); // 绘制头像框
		cv::rectangle(img, body, cv::Scalar(0,255,0), 1, 1, 0); // 绘制身体框
		txtX = 0;
		txtY = 0 + 30;
		cv::putText(img,behaviorStr[3], 
					cv::Point(txtX,txtY), 
					cv::FONT_HERSHEY_PLAIN, 
					2.0, 
					cv::Scalar(0,0,255),
					2);
	}
	return img;
}

cv::Mat DetectorDriver::getLookStraightLableImage(bool showLabelImage)
{
	cv::Mat img = detectImg.clone();	// 拷贝输入原图
	cv::Rect head = HBDetector->getHeadBox();// 获取head区域的外框
	cv::Rect body = HBDetector->getBodyBox();// 获取body区域的外框

	int txtX,txtY;	
	if (showLabelImage)
	{
		cv::rectangle(img, head, cv::Scalar(0,0,255), 1, 1, 0); // 绘制头像框
		cv::rectangle(img, body, cv::Scalar(0,255,0), 1, 1, 0); // 绘制身体框
		txtX = 0;
		txtY = 0 + 30;
		cv::putText(img,behaviorStr[4], 
					cv::Point(txtX,txtY), 
					cv::FONT_HERSHEY_PLAIN, 
					2.0, 
					cv::Scalar(0,0,255),
					2);
	}
	return img;
}

std::string DetectorDriver::getRknnApiVer()
{
	return HBDetector->getRknnApiVer();
}
std::string DetectorDriver::getRknnDrvVer()
{
	return HBDetector->getRknnDrvVer();
}

int DetectorDriver::preconditionCheck()
{
	cv::Mat HeadImg = HBDetector->getHeadImage();
	cv::Mat BodyImg = HBDetector->getBodyImage();

	// 行为检测前置条件
	if(BodyImg.empty())
		return -1;
	if(HeadImg.empty())
		return -2;
	if(HeadImg.cols*HeadImg.rows > BodyImg.cols*BodyImg.rows*0.7)
		return -3;

	// cv::Rect head = HBDetector->getHeadBox();// 获取head区域的外框
	// cv::Rect body = HBDetector->getBodyBox();// 获取body区域的外框
	// int x1p, x2p, y1p, y2p;
	// int x1h, x2h, y1h, y2h;
	// x1p = body.x;
	// y1p = body.y;
	// x2p = x1p + body.width - 1;
	// y2p = y1p + body.height - 1;
	// x1h = head.x;
	// y1h = head.y;
	// x2h = x1h + head.width - 1;
	// y2h = y1h + head.height - 1;
	// int x1o, y1o, x2o, y2o;
	// x1o = std::max(x1p, x1h);
	// y1o = std::max(y1p, y1h);
	// x2o = std::min(x2p, x2h);
	// y2o = std::min(y2p, y2h);
	// if(x1o >= x2o || y1o >= y2o)
	// 	return -4;
	// else if((x2o - x1o + 1)*(y2o - y1o + 1) < 0.9*head.width*head.height)
	// 	return -5;

	return 0;
}