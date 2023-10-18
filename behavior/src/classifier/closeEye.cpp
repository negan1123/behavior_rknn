/*
 * closeEye.cpp
 *
 *  Created on: Jan 11, 2022
 *      Author: wy
 */

#include <classifier/closeEye.h>

CloseEye::CloseEye()
{
	duration = BehaviorConf::Instance()->getCloseEyeDuration();
	frequency = BehaviorConf::Instance()->getCloseEyeFrequency();
}

CloseEye::~CloseEye()
{

}

void CloseEye::inference(const time_t& time, const cv::Mat& img)
{
	Record rec;
	rec.time = time;
	rec.img = img.clone();

	if(records.empty())
	{
		records.push_back(rec);
		return;
	}

	Record temp;
	temp = records.back();

	if(rec.time != temp.time)
	{
		for(auto it = records.begin(); it != records.end();)
		{
			if((time - it->time) > duration)
			{
				it = records.erase(it);
				if(records.empty())
					break;
			}
			else
				break;
		}
		records.push_back(rec);
		if(records.size() >= frequency)
		{
			auto it = records.begin() + records.size()/2;
			std::string pic_path(BehaviorConf::Instance()->getPicVideoPath());
			pic_path += "/" + std::to_string(it->time) + "_1.jpg";
			cv::imwrite(pic_path, it->img);
			if(cb)
				cb(pic_path);
			records.clear();
		}
	}
}
