/*
 * LookRightLeftDetector.h
 *
 *  Created on: 2021年1月7日
 *      Author: syukousen
 */

#ifndef _LOOKRIGHTLEFTDETECTOR_H_
#define _LOOKRIGHTLEFTDETECTOR_H_
#include <Detector.h>
#include <vector>
#include <queue>

#define EULAR_QUEUE_SIZE	(3)
/**@class LookRightLeftDetector
 * @brief 对驾驶员左顾右盼做出检测
 * */
class LookRightLeftDetector : public Detector
{
public:
	struct EularAngle
	{
		time_t time;		///< 欧拉角产生的时间
		float angle;		///< 欧拉角大小
	};
	/**@brief 构造函数，从配置文件config.ini中获取左顾右盼检测模型的路径，调用Init实现初始化
	* @param	None
	* @return	None
	*/
	LookRightLeftDetector();

	/**@brief 析构函数，调用deInit()释放资源
	* @param	None
	* @return	None
	*/
	~LookRightLeftDetector();

	/**@brief 对输入的图片进行预处理，进行均值和缩放处理
	* @param	None
	* @return	int 预处理结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int imgPreprocess(void);

	/**@brief 加载后的rknn模型进行推理，并将推理结果保存到outData中
	* @param	None
	* @return	int 推理结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int detect(void);

	/**@brief 返回是否左顾右盼
	* @param	None
	* @return	bool 是否左顾右盼
	*/
	bool isLookRightLeft(void);

	void getAngle(float &y, float &p, float &r){y = yaw;p = pitch; r = roll;}
protected:
private:
   cv::Mat preProcessImage;		///< 预处理后的图片
   int preProcessImageW;		///< 预处理后的图片宽度
   int preProcessImageH;		///< 预处理后的图片高度
   float yaw;					///< 左右欧拉角
   float pitch;					///< 上下欧拉角
   float roll;					///< 旋转欧拉角
   /* 图片进行滤波用的均值，动态数组方式保存，均值通常是3个值，分别为RGB需要调整的值 */
   std::vector<float> mean;
   std::vector<float> std;		///< 图片进行预处理的参数
   std::queue<EularAngle> lrAngleQueue; ///< 左顾右盼欧拉角队列
};
#endif /* _LOOKRIGHTLEFTDETECTOR_H_ */
