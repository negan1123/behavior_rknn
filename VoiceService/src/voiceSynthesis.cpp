/*
 * voiceSynthesis.cpp
 *
 *  Created on: Dec 14, 2021
 *      Author: wy
 */

#include <voiceSynthesis.h>
#include <fstream>
#include <thread>
#include <spdlog/spdlog.h>

VoiceSynthesis::VoiceSynthesis(const std::string &app_id, const std::string &api_key, const std::string &secret_key)
{
	client = new aip::Speech(app_id, api_key, secret_key);
}

VoiceSynthesis::~VoiceSynthesis()
{
	delete client;
	client = nullptr;
}

void VoiceSynthesis::setOpts(const std::string &text, const int &speed, const int &pitch, const int &volume, const int &voiceType)
{
	this->text = text;
	options["spd"] = std::to_string(speed);
	options["pit"] = std::to_string(pitch);
	options["vol"] = std::to_string(volume);
	options["per"] = std::to_string(voiceType);
}

bool VoiceSynthesis::synthesis(const std::string &fileName)
{
	bool ret = false;
	std::string data;
	std::ofstream mp3_file;
	mp3_file.open(fileName.c_str(), std::ios::out | std::ios::binary);
	client->text2audio(text, options, data);
	if (data.size() > 200)
	{
		mp3_file << data;
		mp3_file.close();
		ret = true;
	}
	else
	{
		spdlog::error("合成失败!");
		mp3_file.close();
	}
	return ret;
}
