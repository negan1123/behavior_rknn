#ifndef _DETECTOR_H_
#define _DETECTOR_H_

#include <string>
#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <memory>
#include <fstream>
#include <cstdint>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <rknn_api.h>

using namespace std;

// ClassInfo用于存储每条GridInfo中对应目标的score和xywh, 后续创建3个list用于存储不同类型的ClassInfo
struct ClassInfo
{
    float x;
    float y;
    float w;
    float h;
    float score;            // 目标得分
};

class Detector
{
public:

    Detector();

    int initModel(string rknnPath);

    void showModelInfo();

    int deInitModel();

    int input(cv::Mat &image);

    virtual int imgPreprocess()=0;

    virtual int detect()=0;

    virtual ~Detector();

protected:
    rknn_context ctx{0};           // rknn句柄
    vector<void*> outData;	    // 运行模型后输出结果
    cv::Mat inputImg;           // 输入的图片
    float rescale_ratio{1.0};	///< 预处理图片压缩比例

private:
    unsigned char *model;       // rknn模型数据指针

};

#endif /* _DETECTOR_H_ */