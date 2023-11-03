#include <volumeController.h>
#include <alsa/asoundlib.h>

vector<string> split(const string &str, const string &pattern)
{
    // const char* convert to char*
    char *strc = new char[strlen(str.c_str()) + 1];
    strcpy(strc, str.c_str());
    vector<string> resultVec;
    char *tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL)
    {
        resultVec.push_back(string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }

    delete[] strc;
    return resultVec;
};

volumeController::volumeController()
{
}

volumeController::~volumeController()
{
}

int volumeController::updateSysVolume(string conffile)
{
    std::thread updateVolume([=]()
                             {
        while(1)
        {   
            
            VoiceServiceConf *confjson = VoiceServiceConf::Instance(conffile);
            confjson->loadConfig();
            int json_volume = confjson->getSysVolume();
            int ret;

            ret = alsaMixerUpdataVol(json_volume);
            if (ret != 0)
            {
                popenUpdataVol(json_volume);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        } });
    updateVolume.detach();
    return 0;
}

void volumeController::popenUpdataVol(int json_volume)
{
    // 查看当前系统声卡
    FILE *cards = NULL;
    char buffer[1024] = {0};
    string cmd0 = "cat /proc/asound/cards | awk '{print $3}'";
    vector<string> c;
    if (cards = popen(cmd0.c_str(), "r"))
    {
        if (fread(buffer, 1, 1024, cards) > 0)
        {
            c = split(buffer, "\n");
        }
        pclose(cards);
    }

    // 操作rk809声卡
    string rk809 = "rockchip_rk809-";
    int card_id = 0;
    for(int i = 0; i<c.size(); i++)
    {
        string str = c[i];
        int comp = rk809.compare(str);
        if (comp == 0)
        {
            card_id = i;
        }
    }

    // 查看当前系统音量
    FILE *f = NULL;
    char buf[1024] = {0};
    string cmd1 = "amixer -c " + to_string(card_id) +" cget numid=1,iface=MIXER,name='Playback Path' | grep '  : values' | awk '{print $2}'";
    vector<string> svec1, svec2;
    if (f = popen(cmd1.c_str(), "r"))
    {
        if (fread(buf, 1, 1024, f) > 0)
        {
            svec1 = split(buf, "\n");
        }
        pclose(f);
    }
    for (auto it = svec1.begin(); it != svec1.end(); it++)
    {
        svec2 = split(*it, "=");
    }
    int nowVolume = atoi(svec2[1].c_str());

    if (json_volume > 10)
    {
        json_volume = 10;
    }
    // 若json文件中音量不等于当前系统音量，则设置系统音量
    if (nowVolume != json_volume)
    {
        cout<<"设置音量："<<nowVolume<<" to "<<json_volume<<endl;
        // popen设置当前volume音量
        FILE *fp = NULL;
        char resbuf[1024] = {0};
        string cmd3 = "amixer -c " + to_string(card_id) +" cset numid=1,iface=MIXER,name='Playback Path' " + to_string(json_volume) + " | grep '  : values' | awk '{print $2}'";
        if (fp = popen(cmd3.c_str(), "r"))
        {
            if (fread(resbuf, 1, 1024, fp) > 0)
            {

            }
            pclose(fp);
        }
    }
}

int volumeController::alsaMixerUpdataVol(int json_volume)
{
    /************声音调节部分*****************/
    int ret;
    snd_mixer_t *mixer;                          // Mixer handle Mixer对象句柄
    snd_mixer_elem_t *master_element;            // Mixer element type 混合器所有通道集合
    ret = snd_mixer_open(&mixer, 0);                   // 打开mixer
    if (ret != 0)
        return -1;
    ret = snd_mixer_attach(mixer, "hw:2");          // 将HCTL连接到打开的mixer上
    if (ret != 0)
        return -1;
    snd_mixer_selem_register(mixer, NULL, NULL); // 注册mixer
    snd_mixer_load(mixer);                       // 加载 mixer
    master_element = snd_mixer_first_elem(mixer); // 获取当前mixer混合器对象
    
    ret = snd_mixer_selem_set_playback_volume_range(master_element, 0, 100); /* 設定音量的範圍 0 ~ 100 */
    if (ret != 0)
    {
        return 1;
    }
    // 获取左右声道当前音量
    long now_left_volume, now_right_volume;
    ret = snd_mixer_selem_get_playback_volume(master_element, SND_MIXER_SCHN_FRONT_LEFT, &now_left_volume);
    if (ret != 0)
    {
        return 2;
    }
    ret = snd_mixer_selem_get_playback_volume(master_element, SND_MIXER_SCHN_FRONT_RIGHT, &now_right_volume);
    if (ret != 0)
    {
        return 3;
    }
    // 设置左右声道音量
    int updata_volume = json_volume * 10;
    if (updata_volume > 100)
    {
        updata_volume = 100;
    }
    if (now_left_volume != updata_volume)
    {   
        cout<<"设置音量："<<now_left_volume<<" to "<<updata_volume<<endl;
        ret = snd_mixer_selem_set_playback_volume_all(master_element, updata_volume);
        if (ret != 0)
        {
            return 4;
        }
    }
    snd_mixer_close(mixer);
    return ret;
}