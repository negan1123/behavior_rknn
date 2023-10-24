#ifndef _DETECTOR_SERVICE_H_
#define _DETECTOR_SERVICE_H_

#include <functional>
#include <thread>
#include <unistd.h>
#include <chrono>
#include <atomic>
#include <behaviorStatus.h>
#include <DetectorDriver.h>
#include <UploadBehavior.h>
#include <localBus.h>
#include <classify/Yawn.h>
#include <classify/Call.h>
#include <classify/Smoke.h>
#include <classify/BlockCamera.h>
#include <classify/CloseEyes.h>
#include <classify/lookLeftRight.h>
#include <classify/lookStraight.h>
#include <BehaviorConf.hpp>

struct detectImgInfo
{
    cv::Mat img;
    string name;
    time_t enTime;
};

class DetectorService
{
public:
	/**
     *@brief 构造函数
	 */
    DetectorService();

    ~DetectorService();

    /**
     * @brief 开始行为检查
	 */
    void start();

    /**
     * @brief 图片入队函数
     * @param[in] 待检测图片
     */
    void enqueueMat(cv::Mat & detect);

    /**
     * @brief 遍历本地目录,将图片加入到待检测图片列表中,测试用
     * @param[in] 目录路径
     * @return 是否成功读取图片
     */
    bool openImageToList(string dirPath);

    string getRknnApiVer();

    string getRknnDrvVer();

    void test_start();

    /**@brief	设置回调函数
	 * @param	_cb	回调函数
	 * @return	None
	 */
	// void test_setOnDetect(onDetect _cb){test_cb = _cb;};

    string getRknnApiV();
    string getRknnDrvV();


private:
	mutex mtx;									///< detectImageList队列锁
    
    unique_ptr<LocalBus> localBus;	            ///< 内部总线实例
    unique_ptr<DetectorDriver> mDetectorDriver;	///< 行为检查实例
    unique_ptr<UploadBehavior> mUploadBehavior;	///< 驾驶行为上传服务器实例

    unique_ptr<Yawn> mYawn;	                    ///< 打呵欠分类实例
    unique_ptr<Call> mCall;	                    ///< 打电话分类实例
    unique_ptr<Smoke> mSmoke;	                ///< 抽烟分类实例
    unique_ptr<BlockCamera> mBlockCamera;	    ///< 遮挡摄像头分类实例
    unique_ptr<CloseEyes> mCloseEyes;	        ///< 闭眼分类实例
    unique_ptr<lookLeftRight> mLookLeftRight;   ///< 左顾右盼分类实例
    unique_ptr<lookStraight> mLookStraight;     ///< 长时间直视分类实例
    unique_ptr<BehaviorStatus> mstatus;	                ///< 行为检测程序状态实例


    queue<detectImgInfo> detectImageList; // 待检测图片列表

    int thresholdOne {0};							// 速度阈值1，大于此值才进行行为检测
    atomic<float> speed {0.0};									// 当前车速


    onDetect test_cb{nullptr};	// 回调函数指针
};

#endif /* _DETECTOR_SERVICE_H_ */