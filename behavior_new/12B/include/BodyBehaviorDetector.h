#ifndef BodyBehaviorDetector_H_
#define BodyBehaviorDetector_H_

#include <Detector.h>
#include <list>
#include <RetinaDriverConf.hpp>
#include <BehaviorConf.hpp>

// GridInfo用于存储每个网格中的8条基本数据，xywh conf scores
struct BB_GridInfo
{
    float x;                // x值
    float y;                // y值
    float w;                // w值
    float h;                // h值
    float conf;             // box置信度
    float headScore;       // 手机得分
    float phoneScore;        // 头部得分
};



class BodyBehaviorDetector : public Detector
{
public:
    BodyBehaviorDetector();
    
    ~BodyBehaviorDetector();

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
     * @brief 获取最优解
     * @return 成功返回0，失败返回-1
     */
    int getBestResult();

    /**
     *  @brief 获取身体行为，目前仅支持检测使用手机
     *  @return 检测到手机---1，没检测到---0
     */
    int getBodyBehavior();

    /**
     * @brief 获取手机目标检测框
     * @return 目标检测框Rect
     */
    cv::Rect getPhoneBox();

    /**
     * @brief 获取 rknn api 版本
     * @return 返回 rknn api 版本
     */
    string getRknnApiVer();

    /**
     * @brief 获取 rknn drv 版本
     * @return 返回 rknn drv 版本
     */
    string getRknnDrvVer();


private:


    cv::Mat preProcessImage;	// 预处理后的图片
    float preProcessImageW;		// 预处理后的图片宽度
    float preProcessImageH;		// 预处理后的图片高度
    float left_right_padding;     // 左右填充像素
    float top_bottom_padding;     // 上下填充像素

    vector<float> anchors;	// 将读取的内容放入动态数组
    list<float> stride;     // 存储网格大小

    list<ClassInfo> PhoneList;           // 手机数据

    ClassInfo PhoneMax;                      // 手机最优解

    float PhoneThres;                    // 手机检测阈值
    string apiVersion;      // rknn api版本
    string drvVersion;      // rknn drv版本

    /**
     * @brief 加载anchors.txt文件
     * @param[in] 文件路径
     * @return 成功返回0，失败返回-1
     */
    int loadAnchors(string filePath);

    /**
     * @brief 设置ClassInfo内容
     * @param[in] ClassInfo的引用
     * @param[in] x
     * @param[in] y
     * @param[in] w
     * @param[in] h
     * @param[in] 对应class得分
     */
    void setClassInfo(ClassInfo &Info, float x, float y, float w, float h, float score)
    {
        Info.x = x;
        Info.y = y;
        Info.w = w;
        Info.h = h;
        Info.score = score;
    };

    /**
     * @brief 用于计算模型输出的三个尺度
     * @param[in] outData中每个尺度的指针
     * @param[in] 一个网格中的锚框数量a
     * @param[in] 当前网格 h 数量
     * @param[in] 当前网格 w 数量
     * @param[in] 当前网格对应的anchors指针
     * @param[in] 当前网格像素值 stride
     * @return 成功返回0
     */
    int calculateGridInfo(float *GridData, int a, int h, int w, float *anchors, float stride);

    /**
     * @brief 获取真实框
     * @param[in] xyxy类型的ClassInfo
     * @return 真实框ClassInfo
     */
    ClassInfo getTrueBox(ClassInfo Info);
};

#endif /* BodyBehaviorDetector_H_ */