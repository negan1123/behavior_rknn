/*
 * RetinaDriver.h
 *
 *  Created on: 2020年12月29日
 *      Author: syukousen
 */

#ifndef _RETINADRIVER_H_
#define _RETINADRIVER_H_
#include <memory>
#include "PersonHeadDetector.h"
//#include "DriverBehaviorDetector.h"
#include "lookLeftRightDetector.h"
#include "HeadBehaviorDetector.h"
#include "BodyBehaviorDetector.h"
//#include "LookRightLeftDetector.h"

typedef struct _DriverBehavior_
{
   bool closeEye{false};		///< 闭眼
   bool yawn{false};			///< 打哈欠
   bool smoke{false};			///< 抽烟
   bool phone{false};			///< 打电话
   bool drink{false};			///< 喝水
   bool blockCamera{false};		///< 遮挡镜头
   bool lookrightleft{false};	///< 左顾右盼
   bool other{false};			///< 其它行为，检测出人像，但是没有检测出头像
}DriverBehavior;

/**@class RetinaDriver
 * @brief 驾驶员行为分析类，对驾驶员的8类行为进行分析。通过PersonHeadDetector、DriverBehaviorDetector、LookRightLeftDetector调用，\n
 * 			实现对驾驶员行为分析，分析记过通过DriverBehavior对象返回
 * */
class RetinaDriver
{
public:
	/**@brief 构造函数。调用loadConfig()加载配置信息，创建三种检测器的实例，并进行各自初始化
	* @param	None
	* @return	None
	*/
	RetinaDriver();

	/**@brief 析构函数，释放资源
	* @param	None
	* @return	None
	*/
	~RetinaDriver();

	/**@brief 输入需要进行检测的图片，输入格式采用opencv的mat
	* @param	image	检测的图片
	* @return	int 输入结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int input(cv::Mat &image);

	/**@brief 进行驾驶员行为推理 \n
	* 			首先通过PersonHeadDetector对输入图片进行识别，得到驾驶员person及head部分图片 \n
	*			将person图片作为输入，通过DriverBehaviorDetector进行推理，判断是否属于6中驾驶行为 \n
	* 			将head图片作为输入，通过LookRightLeft进行推理，判断是狗左顾右盼，并将这种行为整合到DriverBehavior中
	* @param	None
	* @return	int 推理结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int inference(void);

	/**@brief 获取驾驶员驾驶行为分类结果
	* @param	None
	* @return	DriverBehavior 驾驶行为分类结果
	*/
	DriverBehavior getBehavior(void){return behavior;}

	/**@brief 获取驾驶员驾驶行为分类结果,用int的形式表示，8bit每bit表示一个驾驶员行为
	* @details 从低比特位开始说明，第1bit代表有无闭眼行为，第2bit代表有无打哈欠行为，第3bit代表有无抽烟行为，第4bit代表有无打电话行为，
	* 第5bit代表有无喝水行为，第6bit代表有无遮挡摄像头行为，第7bit代表有无左顾右盼行为，第8bit代表有无其他行为
	* @param	None
	* @return	int 驾驶行为分类结果
	*/
	int getBehaviorInt(void);

	/**@brief 获取标记过的图片，标记包括一个或者多个行为的文字描述、头部和身体部分的红色外框
	* @param[in]	showPersonBox	画人像
	* @param[in]	showHeadBox		画头像
	* @return	cv::Mat 画人像头像标记后的图像
	*/
	cv::Mat &getLabelledImage(bool showPersonBox = true, bool showHeadBox = true);

	cv::Mat getCloseEyeLabelledImage(bool showPersonBox = true, bool showHeadBox = true);

	cv::Mat getLookLeftRightLabelledImage(bool showPersonBox = true, bool showHeadBox = true);

	cv::Mat getLookStraightLabelledImage(bool showPersonBox = true, bool showHeadBox = true);

	int preconditionCheck();

	/**@brief 获取rknnApi版本
	* @param	None
	* @return	None
	*/
	std::string getRknnApiVer();

	/**@brief 获取rknnDrv版本
	* @param	None
	* @return	None
	*/
	std::string getRknnDrvVer();

//	void getAngle(float &y, float &p, float &r){lrlDetector->getAngle(y,p,r);}

#if 1	//测试用，将截取的图像保存出来
	cv::Mat getPersonImage(void);
	cv::Mat getHeadImage(void);
	void getAngle(float &y, float &p, float &r){lrlDetector->getAngle(y,p,r);}
#endif
protected:
private:
	/**@brief 调用RetinaDriverConfig.load()从配置文件中加载配置信息
	* @param	None
	* @return	int		加载配置文件结果
	* 			0  成功返回
	* 			-1  失败返回
	*/
	/* 调用RetinaDriverConfig.load()从配置文件中加载配置信息
	* 成功返回0，失败返回-1 */
	int loadConfig(void);

private:
   cv::Mat inImage;				///<  输入的驾驶员图片
   std::unique_ptr<PersonHeadDetector> phDetector;		///< 身体和头部检测器
//   std::unique_ptr<DriverBehaviorDetector> dbDetector; 	///< 驾驶行为检测器
   std::unique_ptr<LookLeftRightDetector>  lrlDetector;		///< 左顾右盼检测器
   std::unique_ptr<HeadBehaviorDetector> hbDetector;		///< 头部行为检测器
   std::unique_ptr<BodyBehaviorDetector> bbDetector;		///< 身体行为检测器
//   std::unique_ptr<LookRightLeftDetector> lrlDetector; ///< 左顾右盼检测器
   DriverBehavior behavior;				///< 驾驶员驾驶行为分类，如果某一项为true，表示具有这种驾驶行为，false表示没有这种驾驶行为
};

#endif /* _RETINADRIVER_H_ */
