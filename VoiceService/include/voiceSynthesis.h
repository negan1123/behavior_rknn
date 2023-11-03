/*
 * voiceSynthesis.h
 *
 *  Created on: Dec 14, 2021
 *      Author: wy
 */

#ifndef INCLUDE_VOICESYNTHESIS_H_
#define INCLUDE_VOICESYNTHESIS_H_

#include <string>
#include <speech.h>
#include <map>

/**@class	VoiceSynthesis
 * @brief	语音合成类
 */
class VoiceSynthesis
{
public:
	/**@brief	构造函数
	 * @param	app_id		百度在线短文本语音合成应用的app_id
	 * @param	api_key		百度在线短文本语音合成应用的api_key
	 * @param	secret_key	百度在线短文本语音合成应用的secret_key
	 * @return	None
	 */
	VoiceSynthesis(const std::string &app_id, const std::string &api_key, const std::string &secret_key);

	/**@brief	析构函数
	 * @param	None
	 * @return	None
	 */
	~VoiceSynthesis();

	/**@brief	设置语音合成参数
	 * @param	text		合成语音的文本
	 * @param	speed		合成语音的语速
	 * @param	pitch		合成语音的音调
	 * @param	volume		合成语音的音量
	 * @param	voiceType	合成语音的语音库
	 * @return	None
	 */
	void setOpts(const std::string &text, const int &speed, const int &pitch, const int &volume, const int &voiceType);

	/**@brief	合成语音
	 * @param	fileName	保存合成语音的完整路径+文件名
	 * @return	bool	true 合成成功，false 合成失败
	 */
	bool synthesis(const std::string &fileName);
private:
	aip::Speech *client{nullptr};				///百度语音合成客户端实例
	std::string text;							///合成语音的文本
	std::map<std::string, std::string> options;	///合成语音的其他设置
};


#endif /* INCLUDE_VOICESYNTHESIS_H_ */
