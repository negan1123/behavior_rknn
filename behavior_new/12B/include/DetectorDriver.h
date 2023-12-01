#ifndef DetectorDriver_H_
#define DetectorDriver_H_

#include <list>
#include <queue>
#include <dirent.h>

#include <util/commons.h>
#include <HeadBodyDetector.h>
#include <RockxDetector.h>
#include <BlockCameraDetector.h>
#include <HeadBehaviorDetector.h>


typedef struct _DriverBehavior_
{
    bool phone {false};			// 检测出电话
    bool smoke {false};         // 抽烟行为
    bool blockCamera {false};   // 检测出遮挡摄像头
    bool lookLeftRight {false}; // 左顾右盼行为
    bool closeEyes {false};     // 闭眼行为
    bool yawn {false};          // 打呵欠行为
}DriverBehavior;

class DetectorDriver
{
public:

    DetectorDriver();

    ~DetectorDriver();

    int input(cv::Mat &image);
    
    /**
     * @brief 推理测试函数,遍历本地目录图片进行检测
     */
    int detect();

	/**
    * @brief 获取行为分类结果,用int的形式表示.
	* @return	int 驾驶行为分类结果
	*/
    int getBehaviorInt();

    /**
     * @brief 绘制获取标签图片
     * @param[in] showLabelImage 是否绘制展示标签图片
     * @return 绘制好的标签图片Mat
     */
    cv::Mat getLabelImage(bool showLabelImage);

    /**
     * @brief 绘制获取长时间直视标签图片
     * @param[in] showLabelImage 是否绘制展示标签图片
     * @return 绘制好的标签图片Mat
     */
    cv::Mat getLookStraightLableImage(bool showLabelImage);

    /**
     * @brief 绘制获取左顾右盼标签图片
     * @param[in] showLabelImage 是否绘制展示标签图片
     * @return 绘制好的标签图片Mat
     */
    cv::Mat getLookLeftRightLabelImage(bool showLabelImage);

    /**
     * @brief 获取检测结果---人脸偏转角
     * @param[in] angle 人脸偏转角
     * @param[in] landmarks 人脸关键点
     * @param[in] box 人脸框
     */
    void getFaceResult(faceAngle& angle, vector<landmark>& landmarks, faceBox& box) { FBDetector->getResults(angle, landmarks, box); }


    string getRknnApiVer();

    string getRknnDrvVer();

    int preconditionCheck();

private:
    unique_ptr<HeadBodyDetector> HBDetector;  // 头部身体分割
    unique_ptr<BlockCameraDetector> BlockCamera;  // 指向遮挡摄像头检测器的指针
    unique_ptr<RockxDetector> FBDetector;           // 头部行为检测
    unique_ptr<HeadBehaviorDetector> SmokeDetector;           // 头部行为检测

    cv::Mat detectImg;              // 待检测图片

    DriverBehavior beh;        // 行为结果

    /**
     * @brief 调用RetinaDriverConfig.load()从配置文件中加载配置信息,
	 * @param	None
	 * @return	int		加载配置文件结果
	 * 			0  成功返回
	 * 			-1  失败返回
	 */
	int loadConfig(void);

};

#endif /* DetectorDriver_H_ */