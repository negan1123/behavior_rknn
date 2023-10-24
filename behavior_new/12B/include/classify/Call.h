#ifndef CALL_H_
#define CALL_H_
#include <classify/BehaviorClassify.h>

class Call: public BehaviorClassify
{
public:
    
    Call();

    ~Call();

    virtual void inference(const time_t& time, const cv::Mat& img);

private:
    vector<Record> recordList;	// 用一个vector来存储符合条件的图片

    int count {0};


};

#endif /* CALL_H_ */