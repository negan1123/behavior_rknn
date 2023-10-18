/*
 * DriverBehaviorDetector.h
 *
 *  Created on: 2020年12月29日
 *      Author: syukousen
 */

#ifndef _DRIVERBEHAVIORDETECTOR_H_
#define _DRIVERBEHAVIORDETECTOR_H_

#include <vector>
#include <Detector.h>

#define  DRIVER_RKNN_OUTPUT_CNT		(6)				///<	驾驶行为分析模型输出个数

/**@struct DriverBehavior
 * @brief 驾驶检测的7种行为
 * */
typedef struct _DriverBehavior_
{
   bool closeEye;		///< 闭眼
   bool yawn;			///< 打哈欠
   bool smoke;			///< 抽烟
   bool phone;			///< 打电话
   bool drink;			///< 喝水
   bool blockCamera;	///< 遮挡镜头
   bool lookrightleft;	///< 左顾右盼
   bool other;			///< 其它行为，检测出人像，但是没有检测出头像
}DriverBehavior;

 /**@class DriverBehaviorDetector
  * @brief 驾驶员驾驶行为检测器，输入的是PersonHeadDetector类检测出来的person部分的图像，输出的是6种驾驶行为的分类
  * */
class DriverBehaviorDetector : public Detector
{
public:
	/**@brief 构造函数，从配置文件config.ini中获取身体及头部检测模型的路径，调用Init实现初始化 \n
			  从RetinaDriverConfig类中加载variance、confThresh、mean并保存到类变量中
	* @param	None
	* @return	None
	*/
	DriverBehaviorDetector();

	/**@brief 析构函数
	* @param	None
	* @return	None
	*/
	~DriverBehaviorDetector();

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

	/**@brief 根据推理结果对驾驶行为进行分类
	* @param	None
	* @return	int 分类结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int classifyBehavior(void);

	/**@brief 获取驾驶员驾驶行为分类结果
	* @param	None
	* @return	DriverBehavior 驾驶行为分类结果
	* 			0  成功返回
	* 			1  失败返回
	*/
	DriverBehavior getBehavior(void){return behavior;}
protected:
private:
   cv::Mat preProcessImage;		///< 预处理后的图片
   int preProcessImageW;		///< 预处理后的图片宽度
   int preProcessImageH;		///< 预处理后的图片高度
   std::vector<float> mean; 	///< 图片进行滤波用的均值，动态数组方式保存，均值通常是3个值，分别为RGB需要调整的值
   std::vector<float> std;		///< 图片进行预处理的参数
   DriverBehavior behavior;		///< 驾驶员驾驶行为分类，如果某一项为true，表示具有这种驾驶行为，false表示没有这种驾驶行为
};

#endif /* _DRIVERBEHAVIORDETECTOR_H_ */
