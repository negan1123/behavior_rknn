/*
 * voicePlayer.cpp
 *
 *  Created on: Dec 14, 2021
 *      Author: wy
 */

#include <voicePlayer.h>
#include <spdlog/spdlog.h>

VoicePlayer::VoicePlayer()
{

}

VoicePlayer::~VoicePlayer()
{

}

void VoicePlayer::play(const std::string &fileName)
{
	std::string cmd;
	cmd = "su - firefly -c \"/usr/bin/play -q " + fileName + " &\"";
	spdlog::debug("play cmd: {}",cmd);
	int ret_cmd = 0;
	ret_cmd = system(cmd.c_str());
	if(ret_cmd)
		spdlog::error("play error!");
}

void VoicePlayer::stop()
{
	std::string cmd;
	cmd = "sudo killall play";
	int ret_cmd = 0;
	ret_cmd = system(cmd.c_str());
	if(ret_cmd)
		spdlog::error("stop error!");
}
