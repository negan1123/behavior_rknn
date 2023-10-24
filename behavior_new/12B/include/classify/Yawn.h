#ifndef Yawn_H_
#define Yawn_H_
#include <classify/BehaviorClassify.h>

class Yawn: public BehaviorClassify
{
public:

    Yawn();

    ~Yawn();

    /**
     * @brief	推理函数
	 * @param[in]	time	推理时间
	 * @param[in]	img		待推理图片
	 * @return	None
	 */
    virtual void inference(const time_t& time, const cv::Mat& img);

private:
	vector<Record> recordList;	// 用一个vector来存储符合条件的图片
    int count {0};
};

#endif /* Yawn_H_ */