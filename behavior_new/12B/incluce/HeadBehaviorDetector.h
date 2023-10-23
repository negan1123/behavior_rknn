#ifndef HeadBehaviorDetector_H_
#define HeadBehaviorDetector_H_

#include <Detector.h>
#include <list>
#include <RetinaDriverConf.hpp>
#include <BehaviorConf.hpp>

/**
 * @brief 用于存储分类结果的结构体
 * @param positive_score 正常行为得分
 * @param smoke_score 抽烟行为得分
*/
struct result_cls
{
    float positive_score {0};
    float smoke_score {0};
};

class HeadBehaviorDetector : public Detector
{
public:
    HeadBehaviorDetector();
    
    ~HeadBehaviorDetector();

    /**
     * @brief 图像预处理
     */
    int imgPreprocess() override;

    /**
     * @brief 模型推理
     * @return 成功返回0，失败返回-1
     */
    int detect() override;

    /**
     * @brief 推理结果处理
     * @return 成功返回0，失败返回-1
     */
    int getResults();

    /**
     * @brief 获取当前头部行为(目前仅支持抽烟检测)
     * @return 0---正常，1---抽烟
     */
    int getHeadBehavior();

private:
    cv::Mat preProcessImage;	// 预处理后的图片
    float preProcessImageW;		// 预处理后的图片宽度
    float preProcessImageH;		// 预处理后的图片高度

    result_cls res;         // 初始化分类结果结构体

    string apiVersion;      // rknn api版本
    string drvVersion;      // rknn drv版本
};

#endif /*HeadBehaviorDetector_H_*/