#ifndef TRANSMISSION_CAN_H_
#define TRANSMISSION_CAN_H_

#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <functional>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define CAN_EFF_FLAG 0x80000000U //扩展帧的标识
#define CAN_RTR_FLAG 0x40000000U //远程帧的标识
#define CAN_ERR_FLAG 0x20000000U //错误帧的标识，用于错误检查

//定义消息回调函数
typedef std::function<void(struct can_frame*)> canCallback;

class TransmissionCAN
{
public:
    /*
     * @brief 构造函数，传入CAN接口名
     * @param canName CAN接口名
     */
     TransmissionCAN(const std::string canName);
    
     ~TransmissionCAN();

    /*
     * @brief 绑定CAN总线
     * @return 成功返回0 失败返回其它
     */
     int bindCan();

    /*
     * @brief 向CAN总线发送消息
     * @param frame 发送帧
     * @return 成功返回0 失败返回其它
     */
     int send(struct can_frame* frame);

    /*
     * @brief 向CAN总线发送消息
     * @param canId CAN 标识符
     * @param data 发送数据
     * @param canDlc 数据场的长度
     * @return 成功返回0 失败返回其它
     */
	int send(uint32_t canId, uint8_t* data, uint8_t canDlc);

    /*
     * @brief 设置接收到消息后的回调函数，当收到消息的时候，回调函数将会被调用
     * @param callback 回调函数
     */
	void onRecieve(canCallback callback);

	/*
     * @brief 启动接收数据线程，开始接收数据
     * @param lo 接收自己发出去的数据
     */
	void startRecv(bool lo = false);

	/*
     * @brief 停止接收数据线程
     */
	void stopRecv();

private:
     std::string canName; ///< CAN接口名
     int socketCAN; ///< CAN套接字
     struct sockaddr_can addr; ///< 记录CAN通信的地址
     struct ifreq ifr; ///< 保存网络接口信息
     canCallback callback;
     std::atomic<bool> isRuning; ///< 接收数据线程的运行状态
};  

#endif