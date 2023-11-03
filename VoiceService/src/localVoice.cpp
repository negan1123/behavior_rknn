#include <localVoice.h>

localVoice::localVoice()
{
    version = VoiceServiceConf::Instance()->getVoiceVer();

    mq = make_unique<mqttBus>();
    mq->init();

    mq->msgCallBack([&](const json &js)
    {   
        ftpInfo ftp;  // 创建ftp信息结构体，用于将mqtt消息中的信息读入其中
        // 处理收到的消息
        try
        {
            ftp.host = js.at("ip");
            ftp.download_file = js.at("fileName");
            ftp.user = js.at("userName");
            ftp.password = js.at("passWord");
            ftp.file_url = js.at("fileUrl");
            ftp.port = js.at("port");
            string ic = js.at("isClear");
            isClear = std::stoi(ic);
            string ver = js.at("versionNumber");
            VoiceServiceConf::Instance()->setVoiceVer(ver);
            VoiceServiceConf::Instance()->saveConfig();
        }
        catch (...)
        {
            spdlog::error("mqtt信令解析错误!");
        } 
        
        // 判断是否清空当前文件
        if (isClear == 1)
        {
            string rmcmd = "rm -rf " + VoiceServiceConf::Instance()->getVoicePath() + "*";
            system(rmcmd.c_str());
        }

        // ftp下载文件
        string ftpURL = "ftp://" + ftp.host + ":" + to_string(ftp.port) + ftp.file_url + ftp.download_file;
        string saveURL = VoiceServiceConf::Instance()->getVoicePath() + ftp.download_file;
        string ftpUSER = ftp.user;
        string ftpPW = ftp.password;

        FTP_OPT ftp_opt;    // 构建curl框架 ftp操作对象
        ftp_opt.url = new char[ftpURL.length() + 1];    // 分配空间
        strcpy(ftp_opt.url, ftpURL.c_str());    // 设置ftp url
        ftp_opt.user_key = new char[ftpUSER.length() + ftpPW.length() + 2]; // 分配空间
        sprintf(ftp_opt.user_key, "%s:%s", ftpUSER.c_str(), ftpPW.c_str()); //设置用户名密码 user:password
        ftp_opt.file = new char[saveURL.length() + 1]; // 分配空间
        strcpy(ftp_opt.file, saveURL.c_str());  // 设置ftp文件下载保存路径

        cout<<"fpt地址:"<<ftp_opt.url<<endl;
        cout<<"key:"<<ftp_opt.user_key<<endl;
        cout<<"file:"<<ftp_opt.file<<endl;
       

        if(FTP_DOWNLOAD_SUCCESS == ftp_download(ftp_opt))
        {
            spdlog::info("Download success.");
            
            string unzip_cmd = "sudo unzip " + saveURL + " -o -d /opt/vehicle/";
            cout<<unzip_cmd<<endl;
            system(unzip_cmd.c_str());

            string rmzip_cmd = "sudo rm " + saveURL;
            cout<<rmzip_cmd<<endl;
            system(rmzip_cmd.c_str());
        }
        else
        {
            spdlog::error("Download failed.");
        }
        // 释放申请空间
        delete[] ftp_opt.url;
        delete[] ftp_opt.user_key;
        delete[] ftp_opt.file;
    });

}

localVoice::~localVoice()
{
}

int localVoice::sendVoiceVersion()
{
    string pubTopic = "terminal_signal_up";
    string mqttVer = SignalConf::Instance()->getVersion();
    string terminalCode = VoiceServiceConf::Instance()->getDeviceDevId();
    time_t sessionID = time(NULL);

    // 组合要发送的命令,json格式
    json js;
    json head;
    json body;
    head["event"] = string("_VOICE_VERSION");
    head["sessionID"] = std::to_string(sessionID);
    head["version"] = mqttVer;
    head["terminalCode"] = terminalCode;

    body["voiceVersion"] = version;

    js["head"] = head;
    js["body"] = body;
    string message = js.dump(); // 将json格式转为string格式
    spdlog::debug("发送信令：{}",message);
    bool ret = mq->pushMsg(pubTopic, message);
    return ret;
}

int localVoice::start()
{
    // 向mqtt服务器发送版本信令
    sendVoiceVersion();


    
}