/*
 * lookStraight.cpp
 *
 *  Created on: Jan 12, 2022
 *      Author: wy
 */

#include <classifier/lookStraight.h>

LookStraight::LookStraight()
{
	duration = BehaviorConf::Instance()->getLookStraightDuration();
	yawAngle = BehaviorConf::Instance()->getLookStraightYawAngle();
	pitchAngle = BehaviorConf::Instance()->getLookStraightPitchAngle();
}

LookStraight::~LookStraight()
{

}

void LookStraight::inference(const time_t& time, const cv::Mat& img, const float& yaw, const float& pitch)
{
	yaw_sum += yaw;
	pitch_sum += pitch;
	count += 1;

	if(count < 30)
		return;

	float yaw_average;
	float pitch_average;
	yaw_average = yaw_sum/count;
	pitch_average = pitch_sum/count;
	if(abs(yaw - yaw_average) <= yawAngle && abs(pitch - pitch_average) <= pitchAngle)
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
			records.push_back(rec);
			auto iter = records.begin();
			if(time - iter->time >= duration)
			{
				iter += records.size()/2;
				std::string pic_path(BehaviorConf::Instance()->getPicVideoPath());
				pic_path += "/" + std::to_string(iter->time) + "_128.jpg";
				cv::imwrite(pic_path, iter->img);
				if(cb)
					cb(pic_path);
				records.clear();
			}
		}
	}
	else {
		records.clear();
	}
}


