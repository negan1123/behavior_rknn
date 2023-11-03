/*
 * voiceDAO.cpp
 *
 *  Created on: Dec 15, 2021
 *      Author: wy
 */

#include <voiceDAO.h>
#include <stdlib.h>
#include <spdlog/spdlog.h>


VoiceDAO::VoiceDAO(const std::string &path)
{
	db_ptr = new database(path);
	*db_ptr << "CREATE TABLE IF NOT EXISTS audio_record_list (id char(32) PRIMARY KEY, "
										"text char(512) NOT NULL, speed integer, "
										"pitch integer, volume integer, "
										"voiceType integer, fileName char(256), "
										"record_time integer, last_play_time integer);";
}

VoiceDAO::~VoiceDAO()
{
	delete db_ptr;
	db_ptr = nullptr;
}

bool VoiceDAO::addRecord(const recordInfo &record)
{
	bool ret = false;
	std::unique_lock<std::mutex> lock(db_mutex);
	time_t now;
	now = time(NULL);
	std::string id;
	srand(now);
	int randInt = 0;
	randInt = rand();
	id = std::to_string(randInt) + "_" + std::to_string(now);
	try
	{
		*db_ptr << "INSERT INTO audio_record_list(id, text, speed, pitch, volume, voiceType, fileName, record_time, last_play_time) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);" << id << record.text << record.speed << record.pitch << record.volume << record.voiceType << record.fileName << now << now;
		ret = true;
	}
	catch(...)
	{
		spdlog::error("addRecord unexpected err");
	}
	return ret;
}

bool VoiceDAO::delRecord()
{
	bool ret = false;
	std::unique_lock<std::mutex> lock(db_mutex);
	time_t now, dif;
	now = time(NULL);
	dif = now - 2592000;
	try
	{
		*db_ptr << "DELETE FROM audio_record_list WHERE last_play_time=?;" << dif;
		ret = true;
	}
	catch(...)
	{
		spdlog::error("deleteRecord unexpected err");
	}
	return ret;
}

bool VoiceDAO::updateRecord(const recordInfo &record)
{
	bool ret = false;
	std::unique_lock<std::mutex> lock(db_mutex);
	time_t now;
	now = time(NULL);
	try
	{
		*db_ptr << "UPDATE audio_record_list SET last_play_time=? WHERE(text=? AND speed=? AND pitch=? AND volume=? AND voiceType=?);" << now << record.text << record.speed << record.pitch << record.volume << record.voiceType;
		ret = true;
	}
	catch(...)
	{
		spdlog::error("updateRecord unexpected err");
	}
	return ret;
}

bool VoiceDAO::checkRecord(recordInfo &record)
{
	int count = 0;
	bool ret = false;
	std::unique_lock<std::mutex> lock(db_mutex);
	try
	{
		*db_ptr << "SELECT * FROM audio_record_list WHERE(text=? AND speed=? AND pitch=? AND volume=? AND voiceType=?);" << record.text << record.speed << record.pitch << record.volume << record.voiceType
				>> [&](const int &id, const std::string &text, const int &speed, const int &pitch, const int &volume, const int &voiceType, const std::string &fileName, const int &record_time, const int &last_play_time){
			record.fileName = fileName;
		};
		if(!record.fileName.empty())
			ret = true;
	}
	catch(...)
	{
		spdlog::error("checkRecord unexpected err");
	}
	return ret;
}

