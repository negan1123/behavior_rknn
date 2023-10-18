/*
 * lookLeftRight.cpp
 *
 *  Created on: Jan 12, 2022
 *      Author: wy
 */

#include <classifier/lookLeftRight.h>

LookLeftRight::LookLeftRight()
{
	duration = BehaviorConf::Instance()->getLookLeftRightDuration();
	frequency = BehaviorConf::Instance()->getLookLeftRightFrequency();
	yawAngle = BehaviorConf::Instance()->getLookLeftRightYawAngle();
}

LookLeftRight::~LookLeftRight()
{

}

void LookLeftRight::inference(const time_t& time, const cv::Mat& img, const float& yaw, const float& pitch)
{
	yaw_sum += yaw;
	count += 1;
	interval += 1;

	if(count < 30)
		return;

	if(interval < INTERVAL)
		return;

	float yaw_average;
	yaw_average = yaw_sum/count;
	if(abs(yaw - yaw_average) > yawAngle)
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
				pic_path += "/" + std::to_string(it->time) + "_64.jpg";
				cv::imwrite(pic_path, it->img);
				if(cb)
					cb(pic_path);
				records.clear();
				interval = 0;
			}
		}
	}
}
