#include <classify/Smoke.h>

Smoke::Smoke()
{
	duration = BehaviorConf::Instance()->getSmokeDuration();
	frequency = BehaviorConf::Instance()->getSmokeFrequency();
}

Smoke::~Smoke()
{

}

void Smoke::inference(const time_t& time, const cv::Mat& img)
{
    count += 1;
    if (count < interval)
    {
        return;
    }

    Record rec;         // 用于存储没系安全带图片的结构体
    rec.time = time;    // 初始化分类检测时间
    rec.img = img.clone();  // 初始化分类图片

    if(recordList.empty())
	{
		recordList.push_back(rec);
		return;
	}

    Record back;
    back = recordList.back();  // 取出vector中最后一张图片，即最近加入列表的图片

    // 检测图片是否有效，图片超时检测
    if(rec.time != back.time)
    {
        for(auto it = recordList.begin(); it != recordList.end(); it++)
        {
            if((time - it->time) > duration)    // 列表中只存储在 duration 时间范围内的图片，超时则从列表中删除
            {
                it = recordList.erase(it);
                if(recordList.empty())     // 防止删除越界
                    break;
            }
            else
                continue;
        }
        recordList.push_back(rec);
        printf("抽烟:now recordList = %d\n",recordList.size());
        if(recordList.size() >= frequency)     // 当列表中图片数量大于等于 frequency 则判定为打电话
        {
            auto it = recordList.begin() + recordList.size()/2;   // 取列表中间的图片作为报警图片
            string pic_path = BehaviorConf::Instance()->getPicVideoPath();
            pic_path += "/" + std::to_string(it->time) + "_4.jpg";   // 获取配置文件中保存地址
            cv::imwrite(pic_path, it->img); // 保存图片
            spdlog::info("触发抽烟条件");
            // 调用回调函数，将图片送至上传器
            if(cb)
                cb(pic_path);
            recordList.clear();    // 列表清空
            count = 0;
        }        
    }
}