#ifndef LOCAL_VOICE_H_
#define LOCAL_VOICE_H_


#include <iostream>
#include <mqttBus.h>
#include <ftp.h>

using namespace std;

/// @brief 用于存储ftp相关信息的结构体
struct ftpInfo
{
    string host;            ///< ftp地址
    string user;            ///< ftp用户名
    string password;        ///< ftp密码
    string download_file;   ///< 需要下载的文件名
    string file_url;        ///< 文件所在ftp服务器路径
    int port;               ///< 端口号
};


class localVoice
{
public:
    /**
     * @brief Construct a new local Voice object
     * 
     */
    localVoice();

    /**
     * @brief Destroy the local Voice object
     * 
     */
    ~localVoice();

    /**
     * @brief 执行函数
     * 
     * @return int 
     */
    int start();
    
    /**
     * @brief mqtt发送消息函数
     * 
     * @return int 
     */
    int sendVoiceVersion();


private:
    unique_ptr<mqttBus> mq;     ///< 外部mqtt服务总线实例
    int isClear;                ///< 是否清除当前本地文件，0--不清空 1--清空
    string version;             /// 当前语言文件版本


};

#endif /*LOCAL_VOICE_H_*/