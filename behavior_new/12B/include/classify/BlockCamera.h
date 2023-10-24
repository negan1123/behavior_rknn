#ifndef BLOCK_CAMERA_H_
#define BLOCK_CAMERA_H_
#include <classify/BehaviorClassify.h>

class BlockCamera: public BehaviorClassify
{
public:
    
    BlockCamera();

    ~BlockCamera();

    virtual void inference(const time_t& time, const cv::Mat& img);

private:
    vector<Record> recordList;	// 用一个vector来存储符合条件的图片

};

#endif /* BLOCK_CAMERA_H_ */