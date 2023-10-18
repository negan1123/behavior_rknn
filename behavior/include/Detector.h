/*
 * Detector.h
 *
 *  Created on: 2020年12月22日
 *      Author: syukousen
 */

#ifndef _DETECTOR_H_
#define _DETECTOR_H_

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn/dnn.hpp>
#include "rknn_api.h"

class Detector
{
public:
	/**@brief 构造函数
	* @param	None
	* @return	None
	*/
   Detector();

	/**@brief 初始化相关的变量、加载RKNN，RKNN的路径需要指定
	* @param[in]	rknnName  rknn路径名
	* @return	int 初始化结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int init(std::string rknnName);

	/**@brief 去初始化，释放初始化中加载的资源
	* @param	None
	* @return	int 去初始化结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int deInit(void);

	/**@brief 输入需要进行检测的图片，输入格式采用opencv的mat，函数会根据不同的模型进行预处理
	* @param	image	检测的图片
	* @return	int 输入结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   int input(cv::Mat &image);

	/**@brief 对输入的图片进行预处理，这是一个纯虚函数，在子类中去实现
	* @param	None
	* @return	int 预处理结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   virtual int imgPreprocess(void)=0;

	/**@brief 加载后的rknn模型进行推理，并将推理结果保存到outData中，这是一个纯虚函数，在子类中去实现
	* @param	None
	* @return	int 推理结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
   virtual int detect(void)=0;

	/**@brief 通过控制台显示RKNN的元数据信息
	* @param	None
	* @return	None
	*/
   void showModelInfo(void);

	/**@brief 析构函数，调用deInit()释放资源
	* @param	None
	* @return	None
	*/
   virtual ~Detector();

protected:
   cv::Mat inImage;				///< 输入的图片
   std::vector<void*> outData;	///< 运行模型后输出结果
   float rescale_ratio{1.0};	///< 预处理图片压缩比例
   rknn_context ctx{0};			///< rknn句柄
private:
   void *model{nullptr};					///< rknn模型数据指针
};

#endif /* _DETECTOR_H_ */
