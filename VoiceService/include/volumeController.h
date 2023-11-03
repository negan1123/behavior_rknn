#ifndef VOLUME_CONTROLLER_H_
#define VOLUME_CONTROLLER_H_

#include <iostream>
#include <vector>
#include <VoiceService.hpp>
#include <thread>
#include <string.h>
#include <map>

using namespace std;

class volumeController
{
public:
    /**
     * @brief 构造函数
     */
    volumeController();

    /**
     * @brief 析构函数
     */
    ~volumeController();

    /**
     * @brief 修改系统音量
     * @details 实时读取json文件中的sysvolume字段数据，修改系统音量大小
     * @return 0
    */
    int updateSysVolume(string jsonPath);

    void popenUpdataVol(int json_volume);

    int alsaMixerUpdataVol(int json_volume);

private:
    map<int,string> json_relity_volume_map;
};



#endif /* VOLUME_CONTROLLER_H_ */