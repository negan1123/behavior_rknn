/*
 * PersonHeadDetector.h
 *
 *  Created on: 2020年12月22日
 *      Author: syukousen
 */

#ifndef _PERSONHEADDETECTOR_H_
#define _PERSONHEADDETECTOR_H_

#include <Detector.h>
#include <vector>
/**@struct ConfInfo
 * @brief 保存人像和头像的boxid和置信度，用于计算最优的box
 * */
typedef struct _confInfo_
{
	float value;			///< box的置信度
	int boxId;				///< box的ID
}ConfInfo;

/**@struct DetectBox
 * @brief 保存最优人像和头像的boxid和置信度和坐标信息
 * */
typedef struct _detectBox_
{
	float value;			///< box的置信度
	int boxId;				///< box的ID
	float x;				///< 先验框x坐标，模型直接出来的，需要根据公式计算出对应1280*736的真实框
	float y;				///< 先验框y坐标，模型直接出来的，需要根据公式计算出对应1280*736的真实框
	float w;				///< 先验框宽度，模型直接出来的，需要根据公式计算出对应1280*736的真实框
	float h;				///< 先验框高度，模型直接出来的，需要根据公式计算出对应1280*736的真实框
}DetectBox;

/**@class PersonHeadDetector
 * @brief 对驾驶员身体和头部进行识别的检测器，输入一个图像，输出两个图像(驾驶员身体部分和头部个一个图像)
 * */
class PersonHeadDetector : public Detector
{
public:
	/**@brief 构造函数，从配置文件config.ini中获取身体及头部检测模型的路径，调用Init实现初始化 \n
				调用loadPriors()将priors.bin的数据读取到类变量中 \n
				从config.ini中加载variance数据和confThresh数据
	* @param	None
	* @return	None
	*/
   PersonHeadDetector();

	/**@brief 析构函数，调用deInit()释放资源
	* @param	None
	* @return	None
	*/
   ~PersonHeadDetector();

	/**@brief 显示priors的头部和内容
	* @param	None
	* @return	None
	*/
   void showPriors(void);

	/**@brief 根据输入图像、推理结果创建身体部分及头部图像 \n
				需要调用createDetectBox()方法获得person和head的框以及置信度，置信度大于confThresh时，从原图中抠取出headBox或者personBox区域，小于则不生成对应图像
	* @param	None
	* @return	int 创建图像成功与否
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int createPersonHeadImage(void);

	/**@brief 获取检测到的身体部分的图像，如果没有符合条件的PersonBox，则返回空图
	* @param	None
	* @return	cv::Mat	获取的人像
	*/
   cv::Mat getPersonImage(void);

	/**@brief 获取检测到的头部图像，如果没有符合条件的HeadBox，则返回空图
	* @param	None
	* @return	cv::Mat	获取的头像
	*/
   cv::Mat getHeadImage(void);

	/**@brief 对输入的图片进行预处理，进行均值和缩放处理
	* @param	None
	* @return	int 预处理结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int imgPreprocess(void) override;

	/**@brief 加载后的rknn模型进行推理，并将推理结果保存到outData中
	* @param	None
	* @return	int 推理结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int detect(void) override;

	/**@brief 获取person区域的外框,这个坐标是输入图片里的坐标
	* @param	None
	* @return	cv::Rect person区域坐标
	*/
   cv::Rect getPersonBox(void);

	/**@brief 获取头部区域的外框,这个坐标是输入图片里的坐标
	* @param	None
	* @return	cv::Rect 头部区域坐标
	*/
   cv::Rect getHeadBox(void);

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

private:
	/**@brief 读取指定的位置信息文件，读取priors数据到类变量priorsBody中，前三个量分别写入priorsHead中
	* @param[in]	fileName  文件路径名
	* @return	int 加载结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int loadPriors(std::string fileName);

	/**@brief 根据outData、priors、variance，结合先验框转真实框的算法，计算出头部、person的box及可信度概率值
	* @param	None
	* @return	int 计算结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int createDetectBox(void);
private:
   cv::Mat personImage;						///< 保存检测出来的身体部分图像
   cv::Mat headImage;						///< 保存检测到的头部图像
   cv::Mat preProcessImage;					///< 预处理后的图片
   int preProcessImageW;					///< 预处理后的图片宽度
   int preProcessImageH;					///< 预处理后的图片高度
   int* priorsHead;							///< 从priors.bin读取出的头部数据
   float (*priors)[4];						///< 从priors.bin读取出的数据，定义为：float (*priors)[4]
   float variance[2];						///< 从配置文件中读取的variance数据[0.1, 0.2]
   std::vector<ConfInfo> headConfList;		///< 头部可信度数据列表
   std::vector<ConfInfo> personConfList;	///< person可信度数据列表
   float confThresh;						///< 可信度阈值，初始化时从config.ini中读取，创建好的DetectBox中，如果value大于confThresh，则用于生成头部或者person图像
   DetectBox headBox;						///< 保存置信度最高的那个头部框和置信度值
   DetectBox personBox;						///< 保存置信度最高的那个person的框和置信度
   char *priorsdata{nullptr};				///< 保存pirorsbin文件数据
   std::vector<int> mean;					///< 传入图像前需要处理的均值，<b,g,r>
   std::string apiVersion;					// rknnApi版本
   std::string drvVersion;					// rknnDrv版本
};
#endif /* _PERSONHEADDETECTOR_H_ */
