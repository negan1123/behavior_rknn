#ifndef SMOKE_H_
#define SMOKE_H_
#include <classify/BehaviorClassify.h>

class Smoke: public BehaviorClassify
{
public:
    
    Smoke();

    ~Smoke();

    virtual void inference(const time_t& time, const cv::Mat& img);

private:
    vector<Record> recordList;	// 用一个vector来存储符合条件的图片
    int count {0};
};

#endif /* SMOKE_H_ */