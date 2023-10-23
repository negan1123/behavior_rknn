#include "transmission/transmission_can.h"

/*
 * @brief 构造函数，传入CAN接口名
 * @param canName CAN接口名
 */
TransmissionCAN::TransmissionCAN(const std::string canName)
{
    this->canName = canName;
    isRuning = false;
}

TransmissionCAN::~TransmissionCAN()
{
    startRecv();
    if(socketCAN > 0)
        close(socketCAN);
}

/*
 * @brief 绑定CAN总线
 * @return 成功返回0 失败返回其它
 */
int TransmissionCAN::bindCan()
{
    int ret = -1;
    socketCAN = socket(PF_CAN, SOCK_RAW, CAN_RAW);//创建套接字，设置为原始套接字，原始CAN协议
    strcpy(ifr.ifr_name, canName.c_str()); //指定设备名称
    ioctl(socketCAN, SIOCGIFINDEX, &ifr); //指定CAN设备，获取接口标志
    addr.can_family = AF_CAN; //采用AF_CAN协议
    addr.can_ifindex = ifr.ifr_ifindex;
    if(socketCAN < 0)
        return -1;
    //加长传输队列长度
    ifr.ifr_qlen = 10000;
    if (-1 == ioctl(socketCAN, SIOCSIFTXQLEN, &ifr))
        std::cout<<"设置CAN队列长度失败"<<std::endl;   
    ret = bind(socketCAN, (struct sockaddr *)&addr, sizeof(addr));//将套接字与CAN总线绑定

    if(ret < 0)
    {
        std::cout<<"绑定CAN总线失败"<<std::endl;
        close(socketCAN);
        return ret;
    }

    return 0;
}

/*
 * @brief 向CAN总线发送消息
 * @param frame 发送帧
 * @return 成功返回0 失败返回其它
 */
int TransmissionCAN::send(struct can_frame* frame)
{
    int ret = -1;
    if(socketCAN > 0)
    {
        //ret = sendto(socketCAN, frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
        ret = write(socketCAN, frame, sizeof(struct can_frame));
        if(ret < 0)
        {
            std::cout<<"消息发送失败"<<std::endl;
        }
    }
    
    return ret > 0? 0 : ret;
}

/*
 * @brief 向CAN总线发送消息
 * @param canId CAN 标识符
 * @param data 发送数据
 * @param canDlc 数据场的长度
 * @return 成功返回0 失败返回其它
*/
int TransmissionCAN::send(uint32_t canId, uint8_t* data, uint8_t canDlc)
{
    struct can_frame frame;
    frame.can_id = canId;
    frame.can_dlc = canDlc;

    //远程帧没有数据场
    if(canDlc > 0)
    {
        memcpy(frame.data, data, sizeof(data));
    }

    return send(&frame);
}

/*
 * @brief 设置接收到消息后的回调函数，当收到消息的时候，回调函数将会被调用
 * @param callback 回调函数
 */
void TransmissionCAN::onRecieve(canCallback callback)
{
    this->callback = callback;
}

/*
 * @brief 启动接收数据线程，开始接收数据
 * @param lo 接收自己发出去的数据
 */
void TransmissionCAN::startRecv(bool lo)
{
    if(lo && (socketCAN > 0))
    {
        int ro = 1; // 0 表示关闭( 默认), 1 表示开启
        //接收当前进程发送的报文
        setsockopt(socketCAN, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &ro, sizeof(ro));
    }
    if(!isRuning) {
		isRuning = true;
		std::thread t_recv([=](){
			while(isRuning && (socketCAN > 0))
            {
				try 
                {
                    struct can_frame frame;
                    int nbytes = read(socketCAN, &frame, sizeof(frame));
                    printf("ID=0x%X DLC=%d\n", frame.can_id, frame.can_dlc);
                    if(CAN_EFF_FLAG & frame.can_id)
                    {
                        std::cout<<"读取到扩展帧"<<std::endl;
                    }
                    if(CAN_RTR_FLAG & frame.can_id)
                    {
                        std::cout<<"读取到远程帧"<<std::endl;
                    }
                    if(CAN_ERR_FLAG & frame.can_id)
                    {
                        std::cout<<"读取到错误帧"<<std::endl;
                    }
                    
                    if(callback)
                        callback(&frame);
				}
				catch(...) 
                {
					std::cout<<"读取CAN数据失败"<<std::endl;
				}
			}
		});

		t_recv.detach();
	}
}

/*
 * @brief 停止接收数据线程
 */
void TransmissionCAN::stopRecv()
{
    isRuning = false;
}
