#ifndef CloseEyes_H_
#define CloseEyes_H_
#include <classify/BehaviorClassify.h>

class CloseEyes: public BehaviorClassify
{
public:
    
    CloseEyes();

    ~CloseEyes();

    virtual void inference(const time_t& time, const cv::Mat& img);

private:
    vector<Record> recordList;	// 用一个vector来存储符合条件的图片

    int count {0};
};

#endif /* CloseEyes_H_ */