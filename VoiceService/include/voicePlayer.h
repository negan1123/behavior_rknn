/*
 * voicePlayer.h
 *
 *  Created on: Dec 14, 2021
 *      Author: wy
 */

#ifndef INCLUDE_VOICEPLAYER_H_
#define INCLUDE_VOICEPLAYER_H_

#include <string>

/**@class	VoicePlayer
 * @brief	语音播放器类
 */
class VoicePlayer
{
public:
	/**@brief	构造函数
	 * @param	None
	 * @return	None
	 */
	VoicePlayer();

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~VoicePlayer();

	/**@brief	播放指定语音
	 * @param	fileName	需要播放的语音完整路径+文件名
	 * @return	None
	 */
	void play(const std::string &fileName);

	/**@brief	停止播放
	 * @param	None
	 * @return	None
	 */
	void stop();
};


#endif /* INCLUDE_VOICEPLAYER_H_ */
