/*
 * VoiceServiceConf.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: wy
 */

#ifndef VOICESERVICECONF_HPP_
#define VOICESERVICECONF_HPP_

#include <GlobalConf.hpp>

/**@class VoiceServiceConf
 * @brief 高精度定位工程配置文件
 * */
class VoiceServiceConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	VoiceServiceConf*		VoiceServiceConf单例
	*/
	static VoiceServiceConf* Instance(std::string path = std::string())
	{
		static VoiceServiceConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new VoiceServiceConf(path);
		}
		return _Instance;
	}

	/**@brief 加载json配置文件到内存
	* @param	None
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int loadConfig()
	{
		std::ifstream ifile((const char *)cfgPath.c_str());
		if (!ifile.is_open())
		{
			std::cerr << "未能成功加载 " << cfgPath << std::endl;
			return -1;
		}
		// 先加载全局的配置文件
		GlobalConf::loadConfig(cfgPath);
		json jInfo;
		try
		{
			ifile >> jInfo;
			return loadConfig(jInfo);
		}
		catch(...)
		{
			std::cerr << "parse hplocator.json file err" << std::endl;
			return -1;
		}
		return 0;
	}

	/**@brief 加载json配置到内存
	* @param	jInfo	json字段
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int loadConfig(json &jInfo)
	{
		try
		{
			json apiOptions;
			json voiceOptions;
			json path;

			getJsonInfo(jInfo, "apiOptions", apiOptions);
			getJsonInfo(jInfo, "voiceOptions", voiceOptions);
			getJsonInfo(jInfo, "path", path);

			getJsonInfo(apiOptions, "synthesis", SYNTHESIS);
			getJsonInfo(apiOptions, "app_id", APP_ID);
			getJsonInfo(apiOptions, "api_key", API_KEY);
			getJsonInfo(apiOptions, "secret_key", SECRET_KEY);

			getJsonInfo(voiceOptions, "speed", SPEED);
			getJsonInfo(voiceOptions, "pitch", PITCH);
			getJsonInfo(voiceOptions, "volume", VOLUME);
			getJsonInfo(voiceOptions, "voiceType", VOICE_TYPE);
			getJsonInfo(voiceOptions, "sysvolume", SYS_VOLUME);

			getJsonInfo(path, "db_path", DB_PATH);
			getJsonInfo(path, "voice_path", VOICE_PATH);
		}
		catch(...)
		{
			std::cerr << "parse voiceService.json file err" << std::endl;
			return -1;
		}
		return 0;
	}

	/**@brief 将内存里面的配置加载保存到文件中
	* @param	None
	* @return	None
	*/
	void saveConfig()
	{
		//如果没有设置保存路径，直接返回
		if(cfgPath.empty())
			return;
		json jInfo;
		if(0 == configToJson(jInfo))
		{
			std::ofstream ofile((const char *)cfgPath.c_str());
			ofile << jInfo;
		}
	}

	/**@brief 将内存里面的配置加载json中
	* @param	jInfo		加载后的json
	* @return	int		加载结果
	* 			-1		加载失败
	* 			0		加载成功
	*/
	int configToJson(json &jInfo)
	{
		try
		{
			json apiOptions;
			json voiceOptions;
			json path;

			apiOptions["synthesis"] = SYNTHESIS;
			apiOptions["app_id"] = APP_ID;
			apiOptions["api_key"] = API_KEY;
			apiOptions["secret_key"] = SECRET_KEY;

			voiceOptions["speed"] = SPEED;
			voiceOptions["pitch"] = PITCH;
			voiceOptions["volume"] = VOLUME;
			voiceOptions["voiceType"] = VOICE_TYPE;
			voiceOptions["sysvolume"] = SYS_VOLUME;

			path["db_path"] = DB_PATH;
			path["voice_path"] = VOICE_PATH;

			jInfo["apiOptions"] = apiOptions;
			jInfo["voiceOptions"] = voiceOptions;
			jInfo["path"] = path;

			return 0;
		}
		catch(...)
		{
			std::cerr << "save voiceService.json file err" << std::endl;
			return -1;
		}
	}

	/**@brief 获取是否需要合成新语音
	* @param	None
	* @return	bool		true 需要，false 不需要
	*/
	bool getSynthesis() const {return SYNTHESIS;}

	/**@brief 获取AppID
	* @param	None
	* @return	std::string		AppID
	*/
	std::string getAppID() const {return APP_ID;}

	/**@brief 获取ApiKey
	* @param	None
	* @return	std::string		ApiKey
	*/
	std::string getApiKey()const {return API_KEY;}

	/**@brief 获取SecretKey
	* @param	None
	* @return	std::string		SecretKey
	*/
	std::string getSecretKey() const {return SECRET_KEY;}

	/**@brief 获取语速
	* @param	None
	* @return	int		语速
	*/
	int getSpeed() const {return SPEED;}

	/**@brief 获取音调
	* @param	None
	* @return	int		音调
	*/
	int  getPitch() const {return PITCH;}

	/**@brief 获取音量
	* @param	None
	* @return	int		音量
	*/
	int  getVolume() const {return VOLUME;}

	/**@brief 获取系统音量
	* @param	None
	* @return	int		音量
	*/
	int  getSysVolume() const {return SYS_VOLUME;}

	/**@brief 获取语音库
	* @param	None
	* @return	int		语音库
	*/
	int  getVoiceType() const {return VOICE_TYPE;}

	/**@brief 获取数据库路径
	* @param	None
	* @return	std::string		数据库路径
	*/
	std::string  getDbPath() const {return DB_PATH;}

	/**@brief 获取语音文件路径
	* @param	None
	* @return	std::string		语音文件路径
	*/
	std::string  getVoicePath() const {return VOICE_PATH;}

	/**@brief 设置是否需要合成新语音
	* @param	synthesis	true 需要，false 不需要
	* @return	None
	*/
	void setSynthesis(const bool& synthesis){SYNTHESIS = synthesis;}

	/**@brief 设置AppID
	* @param	appID	AppID
	* @return	None
	*/
	void setAppID(const std::string& appID){APP_ID = appID;}

	/**@brief 设置ApiKey
	* @param	apiKey	ApiKey
	* @return	None
	*/
	void setApiKey(const std::string& apiKey){API_KEY = apiKey;}

	/**@brief 设置SecretKey
	* @param	host		caster服务器地址
	* @return	None
	*/
	void setSecretKey(const std::string& secretKey) {SECRET_KEY = secretKey;}

	/**@brief 设置语速
	* @param	speed	语速
	* @return	None
	*/
	void setSpeed(const int& speed) {SPEED = speed;}

	/**@brief 设置音调
	* @param	pitch		音调
	* @return	None
	*/
	void setPitch(const int& pitch) {PITCH = pitch;}

	/**@brief 设置音量
	* @param	volume			音量
	* @return	None
	*/
	void setVolume(const int& volume) {VOLUME = volume;}

	/**@brief 设置系统音量
	* @param	volume			音量
	* @return	None
	*/
	void setSysVolume(const int& volume) {SYS_VOLUME = volume;}

	/**@brief 设置语音库
	* @param	pwd				语音库
	* @return	None
	*/
	void setVoiceType(const int& voiceType) {VOICE_TYPE = voiceType;}

	/**@brief 设置数据库路径
	* @param	db_path				数据库路径
	* @return	None
	*/
	void setDbPath(const std::string& db_path) {DB_PATH = db_path;}

	/**@brief 设置语音文件路径
	* @param	db_path				语音文件路径
	* @return	None
	*/
	void setVoicePath(const std::string& voice_path) {VOICE_PATH = voice_path;}
private:
	/**@brief 构造函数
	* @param[in]		path	json配置文件路径
	* @return	None
	*/
	VoiceServiceConf(const std::string & path){cfgPath = path;}

private:
	bool SYNTHESIS;
	std::string APP_ID;			///< 服务器地址
	std::string API_KEY;		///< caster服务器地址
	std::string SECRET_KEY;		///< 挂载点
	int PITCH;					///< 用户名
	int SPEED;					///< 密码
	int VOLUME;					///< 合成语音文件的音量
	int VOICE_TYPE;				///< 声音类型
	int SYS_VOLUME;				// 系统音量
	std::string cfgPath;		///< 配置文件路径
	std::string DB_PATH;
	std::string VOICE_PATH;
};

#endif /* VOICESERVICECONF_HPP_ */
