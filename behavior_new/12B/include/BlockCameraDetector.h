#ifndef block_camera_h
#define block_camera_h

#include <Detector.h>
#include <BehaviorConf.hpp>

class BlockCameraDetector : public Detector
{
public:
    
    BlockCameraDetector();

    ~BlockCameraDetector();

    /**
     * @brief 图像预处理
     */
    int imgPreprocess() override;

    /**
     * @brief opencv 检测推理
     * @details 使用opencv对图像进行二值化处理，判断计算遮挡像素点个数
     * @return 成功返回0，失败返回-1
     */
    int detect() override;

    /**
     * @brief 检测该部分是否遮挡
     * @details 使用Canny处理图像，检测分割后的图像，若未遮挡像素点小于thres阈值，则判定为遮挡
     * @param 截取出来的图片
     * @param 未遮挡像素点阈值
     * @return 遮挡返回1，未遮挡返回0
    */
    int isBlockCamera(cv::Mat img, int thres);

    /**
     * @brief 获取行为
     * @details 计算遮挡像素点个数占总像素点个数占比，若大于检测阈值则判定遮挡摄像头
     * @return 遮挡摄像头 返回1，否则返回0
     */
    int getBehavior();

private:
    cv::Mat preProcessImage;	///< 预处理后的图片
    float preProcessImageW;		///< 预处理后的图片宽度
    float preProcessImageH;		///< 预处理后的图片高度

    int block_num;              ///< 遮挡部分个数
    int piexs_sum;              ///< 图像像素点总个数

    float block_thres;          ///< 遮挡摄像头检测阈值

    int unblock_num_thres;      ///< 未遮挡像素点数量阈值
    int block_part_num_thres;   ///< 遮挡块数量阈值

};

#endif /* block_camera_h */