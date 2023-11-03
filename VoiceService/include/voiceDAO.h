/*
 * voiceDAO.h
 *
 *  Created on: Dec 15, 2021
 *      Author: wy
 */

#ifndef INCLUDE_VOICEDAO_H_
#define INCLUDE_VOICEDAO_H_

#include <sqlite_modern_cpp.h>
#include <string>
#include <mutex>

using namespace sqlite;

/**@class	struct recInfo
 * @brief	数据库记录信息结构体
 */
using recordInfo = struct recInfo
					{
						std::string text;
						int speed = 0;
						int pitch = 0;
						int volume = 0;
						int voiceType = 0;
						std::string fileName;
					};

/**@class	VoiceDAO
 * @brief	语音数据库管理类
 */
class VoiceDAO
{
public:
	/**@brief	构造函数
	 * @param	None
	 * @return	None
	 */
	VoiceDAO(const std::string &path);

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~VoiceDAO();

	/**@brief	向数据库里添加记录
	 * @param	record	需要添加的记录的信息
	 * @return	bool	true 添加成功，false 添加失败
	 */
	bool addRecord(const recordInfo &record);

	/**@brief	从数据库里删除记录
	 * @details	删除30天以前的记录
	 * @return	bool	true 删除成功，false 删除失败
	 */
	bool delRecord();

	/**@brief	更新数据库里的记录
	 * @details	更新记录的最后一次播放时间
	 * @param	record	需要更新的记录的信息
	 * @return	bool	true 更新成功，false 更新失败
	 */
	bool updateRecord(const recordInfo &record);

	/**@brief	查找数据库是否有指定记录
	 * @param	record	需要查找的记录的信息
	 * @return	bool	true 查找成功，false 查找失败
	 */
	bool checkRecord(recordInfo &record);
private:
	database* db_ptr{nullptr};		///数据库实例指针
	std::mutex	db_mutex;			///访问数据库锁
};


#endif /* INCLUDE_VOICEDAO_H_ */
