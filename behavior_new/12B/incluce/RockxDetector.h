#ifndef ROCKX_DETECTOR_H_
#define ROCKX_DETECTOR_H_

#include <iostream>
#include <modules/face.h>
#include <utils/rockx_config_util.h>
#include <rockx.h>
#include <string>
#include <util/log.h>
#include <BehaviorConf.hpp>

using namespace std;

struct landmark
{
    int x{0};
    int y{0};
};

struct faceBox
{
    int left{0};
    int top{0};
    int right{0};
    int bottom{0};
};

struct faceAngle
{
    float yaw{-1000};
    float pitch{-1000};
    float roll{-1000};
};

class RockxDetector
{
public:

    RockxDetector();

    ~RockxDetector();

    /**
     * @brief 加载rock人脸推理模型，包括人脸检测、人脸标志点、人脸欧拉角
     * @return 0 加载成功，其他 加载失败
    */
    int init();

    /**
     * @brief 初始化rockx输入
     * @param 本地图片路径
     * @return rockx_image_t rockx输入
    */
    int input(const std::string& img_path);

    /**
     * @brief rockx人脸信息检测
     * @param rockx_image_t rockx输入
     * @return 0 推理成功，其他 推理失败
    */
    int faceDetect();

    /**
     * @brief 返回结果
     * @param 需要返回的结果变量
    */
    void getResults(faceAngle& angle, vector<landmark>& landmarks, faceBox& box);

    /**
     * @brief 判断是否闭眼
     * @param 人脸关键点 68个
     * @return 闭眼返回1，否则返回0
    */
    int isCloseEye(vector<landmark> marks);

    int isYawn(vector<landmark> marks);

    /**
     * @brief 销毁模型句柄
    */
    int destoryDetector();

private:
    float closeEye_Thres{0.0};                   ///< 闭眼检测阈值
    float yawn_Thres{0.0};                       ///< 打呵欠检测阈值

    rockx_handle_t face_det_handle{NULL};         ///< 人脸检测模型上下文
	rockx_handle_t face_landmark68_handle{NULL};  ///< 人脸标志点模型上下文

    rockx_object_array_t face_array;        ///< 人脸检测输出结果
	rockx_face_angle_t out_angle;           ///< 人脸欧拉角输出结果
    rockx_face_landmark_t out_landmark68;   ///< 人脸标志点输出结果

    rockx_image_t input_image;              ///< rockx模型输入

};



#endif /* ROCKX_DETECTOR_H_ */