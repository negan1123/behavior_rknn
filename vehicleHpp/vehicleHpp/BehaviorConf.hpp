/*
 * BehaviorConf.hpp
 *
 *  Created on: 2021年4月14日
 *      Author: syukousen
 */

#ifndef BEHAVIORCONF_HPP_
#define BEHAVIORCONF_HPP_

#include <GlobalConf.hpp>
/**@class BehaviorConf
 * @brief 驾驶员行为分析设置文件管理类，主要加载json配置的视频采集的参数，行为分析上传服务器信息
 * */
class BehaviorConf : public GlobalConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	ConfigJson*		BehaviorConf单例
	*/
	static BehaviorConf * Instance(std::string path = std::string())
	{
		static BehaviorConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new BehaviorConf(path);
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
		GlobalConf::loadConfig(cfgPath);
		json jInfo;
		try
		{
			ifile >> jInfo;
			return loadConfig(jInfo);
		}
		catch(...)
		{
			std::cerr << "parse behavior.json file err" << std::endl;
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
			json server;
			json video;
			json behavior;
			json speed;
			json closeEye;
			json lookLeftRight;
			json lookStraight;

			getJsonInfo(jInfo, "server", server);
			getJsonInfo(jInfo, "video", video);
			getJsonInfo(jInfo, "behavior", behavior);
			getJsonInfo(jInfo, "speed", speed);

			//获取server相关信息
			getJsonInfo(server, "host", SERVER_HOST);
			getJsonInfo(server, "port", SERVER_PORT);
			getJsonInfo(server, "mode", SERVER_MODE);

			//获取图像相关信息
			getJsonInfo(video, "type", VIDEO_TYPE);
			getJsonInfo(video, "dev", VIDEO_DEV);
			getJsonInfo(video, "fmt", VIDEO_CAP_FMT);
			getJsonInfo(video, "width", VIDEO_CAP_WIDTH);
			getJsonInfo(video, "height", VIDEO_CAP_HEIGHT);
			getJsonInfo(video, "interval", VIDEO_CAP_INTERVAL);
			getJsonInfo(video, "savePic", SAVE_DETECT_PIC);
			getJsonInfo(video, "saveVideo", SAVE_DETECT_VIDEO);
			getJsonInfo(video, "picVideoPath", picVideoPath);
			getJsonInfo(video, "videoDuration", videoDuration);
			getJsonInfo(video, "circleVideoPath", circleVideoPath);
			getJsonInfo(video, "mppEnable", mppEnable);
			getJsonInfo(video, "decodeIDR", decodeIDR);
			getJsonInfo(video, "saveDecodeImg", saveDecodeImg);
			getJsonInfo(video, "decodeImgPath", decodeImgPath);

			//获取违规行为检测间隔时间等相关信息
			getJsonInfo(behavior, "closeEye", closeEye);
			getJsonInfo(behavior, "lookLeftRight", lookLeftRight);
			getJsonInfo(behavior, "lookStraight", lookStraight);
			getJsonInfo(closeEye, "duration", CLOSE_EYE_DURATION);
			getJsonInfo(closeEye, "frequency", CLOSE_EYE_FREQUENCY);
			getJsonInfo(lookLeftRight, "duration", LOOK_LEFT_RIGHT_DURATION);
			getJsonInfo(lookLeftRight, "frequency", LOOK_LEFT_RIGHT_FREQUENCY);
			getJsonInfo(lookLeftRight, "yawAngle", LOOK_LEFT_RIGHT_YAW_ANGLE);
			getJsonInfo(lookStraight, "duration", LOOK_STRAIGHT_DURATION);
			getJsonInfo(lookStraight, "yawAngle", LOOK_STRAIGHT_YAW_ANGLE);
			getJsonInfo(lookStraight, "pitchAngle", LOOK_STRAIGHT_PITCH_ANGLE);

			//获取检测行为的速度阈值
			getJsonInfo(speed, "thresholdOne", thresholdOne);
			getJsonInfo(speed, "thresholdTwo", thresholdTwo);
		}
		catch(...)
		{
			std::cerr << "parse behavior.json file err" << std::endl;
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
			json server;
			json video;
			json behavior;
			json speed;
			json closeEye;
			json lookLeftRight;
			json lookStraight;

			server["host"] = SERVER_HOST;
			server["port"] = SERVER_PORT;
			server["mode"] = SERVER_MODE;

			video["type"] = VIDEO_TYPE;
			video["dev"] = VIDEO_DEV;
			video["fmt"] = VIDEO_CAP_FMT;
			video["width"] = VIDEO_CAP_WIDTH;
			video["height"] = VIDEO_CAP_HEIGHT;
			video["interval"] = VIDEO_CAP_INTERVAL;
			video["savePic"] = SAVE_DETECT_PIC;
			video["saveVideo"] = SAVE_DETECT_VIDEO;
			video["picVideoPath"] = picVideoPath;
			video["videoDuration"] = videoDuration;
			video["circleVideoPath"] = circleVideoPath;
			video["mppEnable"] = mppEnable;
			video["decodeIDR"] = decodeIDR;
			video["saveDecodeImg"] = saveDecodeImg;
			video["decodeImgPath"] = decodeImgPath;

			closeEye["duration"] = CLOSE_EYE_DURATION;
			closeEye["frequency"] = CLOSE_EYE_FREQUENCY;
			lookLeftRight["duration"] = LOOK_LEFT_RIGHT_DURATION;
			lookLeftRight["frequency"] = LOOK_LEFT_RIGHT_FREQUENCY;
			lookLeftRight["yawAngle"] = LOOK_LEFT_RIGHT_YAW_ANGLE;
			lookStraight["duration"] = LOOK_STRAIGHT_DURATION;
			lookStraight["yawAngle"] = LOOK_STRAIGHT_YAW_ANGLE;
			lookStraight["pitchAngle"] = LOOK_STRAIGHT_PITCH_ANGLE;
			behavior["closeEye"] = closeEye;
			behavior["lookLeftRight"] = lookLeftRight;
			behavior["lookStraight"] = lookStraight;

			speed["thresholdOne"] = thresholdOne;
			speed["thresholdTwo"] = thresholdTwo;

			jInfo["server"] = server;
			jInfo["video"] = video;
			jInfo["behavior"] = behavior;
			jInfo["speed"] = speed;
			return 0;
		}
		catch(...)
		{
			std::cerr << "save global.json file err" << std::endl;
			return -1;
		}
	}

	/**@brief 获取服务器地址
	* @param	None
	* @return	std::string		服务器地址
	*/
	std::string getServerHost() const {return SERVER_HOST;}

	/**@brief 获取服务器端口
	* @param	None
	* @return	int		服务器端口
	*/
	int getServerPort()const {return SERVER_PORT;}

	/**@brief 获取上传服务器模式 1udp  2 tcp短链接  3 tcp长链接
	* @param	None
	* @return	int		rtp是否通过tcp传输
	*/
	int getServerMode() const {return SERVER_MODE;}

	/**@brief 获取驾驶员行为分析的视频来源类型 0 摄像头  1   rtsp
	* @param	None
	* @return	int		视频来源类型
	*/
	int getVideoType() const {return VIDEO_TYPE;}

	/**@brief 获取驾驶员行为分析的视频来源路径
	* @param	None
	* @return	std::string		视频来源路径
	*/
	std::string getVideoDev() const {return VIDEO_DEV;}

	/**@brief 获取驾驶员行为分析的视频来源的格式  0  yuyv   1  mjpeg
	* @param	None
	* @return	int		视频来源的格式
	*/
	int getVideoFmt() const {return VIDEO_CAP_FMT;}

	/**@brief 视频来源的图像宽度
	* @param	None
	* @return	int		图像宽度
	*/
	int getVideoWidth() const {return VIDEO_CAP_WIDTH;}

	/**@brief 视频来源的图像高度
	* @param	None
	* @return	int		图像高度
	*/
	int getVideoHeight() const {return VIDEO_CAP_HEIGHT;}

	/**@brief 获取视频采集间隔
	* @param	None
	* @return	int		采集间隔
	*/
	int  getVideoInterval() const {return VIDEO_CAP_INTERVAL;}

	/**@brief 是否保存检测后的照片
	* @param	None
	* @return	int		是否保存检测后的照片
	*/
	int  isSaveDetectPic() const {return SAVE_DETECT_PIC;}

	/**@brief 是否保存检测后的录像
	* @param	None
	* @return	int		是否保存检测后的录像
	*/
	int  isSaveDetectVideo() const {return SAVE_DETECT_VIDEO;}

	/**@brief 获取检测到驾驶员行为录像保存的时间
	* @param	None
	* @return	int		是否保存检测后的照片
	*/
	int  getVideoDuration() const {return videoDuration;}

	/**@brief 获取驾驶行为记录保存路径
	* @param	None
	* @return	std::string		记录保存路径
	*/
	std::string getCircleVideoPath() const {return circleVideoPath;}

	/**@brief 获取录像保存路径
	* @param	None
	* @return	std::string		录像保存路径
	*/
	std::string getPicVideoPath() const {return picVideoPath;}

	/**@brief 获取闭眼行为单次推理持续时长
	* @param	None
	* @return	int		闭眼行为检测单次推理持续时长
	*/
	int getCloseEyeDuration() const {return CLOSE_EYE_DURATION;}

	/**@brief 获取闭眼行为单次预警需要检测出违规行为的次数
	* @param	None
	* @return	int		闭眼行为单次预警需要检测出违规行为的次数
	*/
	int getCloseEyeFrequency() const {return CLOSE_EYE_FREQUENCY;}

	/**@brief 获取左顾右盼行为单次推理持续时长
	* @param	None
	* @return	int		左顾右盼行为单次推理持续时长
	*/
	int getLookLeftRightDuration() const {return LOOK_LEFT_RIGHT_DURATION;}

	/**@brief 获取左顾右盼行为单次预警需要检测出违规行为的次数
	* @param	None
	* @return	int		左顾右盼行为单次预警需要检测出违规行为的次数
	*/
	int getLookLeftRightFrequency() const {return LOOK_LEFT_RIGHT_FREQUENCY;}

	/**@brief 获取左顾右盼行为偏转角界限
	* @param	None
	* @return	float	左顾右盼行为偏转角界限
	*/
	float getLookLeftRightYawAngle() const {return LOOK_LEFT_RIGHT_YAW_ANGLE;}

	/**@brief 获取长时间直视行为单次推理持续时长
	* @param	None
	* @return	int		长时间直视行为单次推理持续时长
	*/
	int getLookStraightDuration() const {return LOOK_STRAIGHT_DURATION;}

	/**@brief 获取长时间直视行为偏转角界限
	* @param	None
	* @return	float	长时间直视行为偏转角界限
	*/
	float getLookStraightYawAngle() const {return LOOK_STRAIGHT_YAW_ANGLE;}

	/**@brief 获取长时间直视行为俯仰角界限
	* @param	None
	* @return	float	长时间直视行为俯仰角界限
	*/
	float getLookStraightPitchAngle() const {return LOOK_STRAIGHT_PITCH_ANGLE;}

	/**@brief 获取解码模式
	* @param	None
	* @return	bool	是否使用硬解码
	*/
	bool isMppEnable() const {return mppEnable;}

	/**@brief 获取解码方式，全解或者抽帧
	* @param	None
	* @return	bool	是否只解码IDR
	*/
	bool isDecodeIdr() const {return decodeIDR;}

	/**@brief 解码后的图像保存路径
	* @param	None
	* @return	std::string		解码后的图像保存路径
	*/
	const std::string& getDecodeImgPath() const {return decodeImgPath;}

	/**@brief 是否保存解码后的图像
	* @param	None
	* @return	bool		是否保存解码后的图像
	*/
	bool isSaveDecodeImg() const {return saveDecodeImg;}

	/**@brief 获取速度阈值1
	* @param	None
	* @return	int	速度阈值1
	*/
	int getThresholdOne() const {return thresholdOne;}

	/**@brief 获取速度阈值2
	* @param	None
	* @return	int	速度阈值2
	*/
	int getThresholdTwo() const {return thresholdTwo;}

	/**@brief 设置服务器地址
	* @param	host	服务器地址
	* @return	None
	*/
	void setServerHost(const std::string& host){SERVER_HOST = host;}

	/**@brief 设置服务器端口
	* @param	port	服务器端口
	* @return	None
	*/
	void setServerPort(int port){SERVER_PORT = port;}

	/**@brief 设置上传服务器模式 1udp  2 tcp短链接  3 tcp长链接
	* @param	mode	rtp是否通过tcp传输
	* @return	None
	*/
	void setServerMode(int mode){SERVER_MODE = mode;}

	/**@brief 设置驾驶员行为分析的视频来源类型 0 摄像头  1   rtsp
	* @param	type	视频来源类型
	* @return	None
	*/
	void setVideoType(int type){VIDEO_TYPE = type;}

	/**@brief 设置驾驶员行为分析的视频来源路径
	* @param	dev				视频来源路径
	* @return	None
	*/
	void setVideoDev(const std::string& dev){VIDEO_DEV = dev;}

	/**@brief 设置驾驶员行为分析的视频来源的格式  0  yuyv   1  mjpeg
	* @param	fmt		视频来源的格式
	* @return	None
	*/
	void setVideoFmt(int fmt){VIDEO_CAP_FMT = fmt;}

	/**@brief 设置视频来源的图像宽度
	* @param	width	图像宽度
	* @return	None
	*/
	void setVideoWidth(int width){VIDEO_CAP_WIDTH = width;}

	/**@brief 设置视频来源的图像高度
	* @param	height	图像高度
	* @return	None
	*/
	void setVideoHeight(int height){VIDEO_CAP_HEIGHT = height;}

	/**@brief 设置视频采集间隔
	* @param	interval	采集间隔
	* @return	None
	*/
	void setVideoInterval(int interval){VIDEO_CAP_INTERVAL = interval;}

	/**@brief 设置是否保存检测后的照片
	* @param	savePic 	是否保存检测后的照片
	* @return	None
	*/
	void setSaveDetectPic(int savePic){SAVE_DETECT_PIC = savePic;}

	/**@brief 设置闭眼行为单次推理持续时长
	* @param	closeEyeDuration	闭眼行为检测单次推理持续时长
	* @return	None
	*/
	void setCloseEyeDuration(int closeEyeDuration) {CLOSE_EYE_DURATION = closeEyeDuration;}

	/**@brief 设置闭眼行为单次预警需要检测出违规行为的次数
	* @param	closeEyeFreqeuncy	闭眼行为单次预警需要检测出违规行为的次数
	* @return	None
	*/
	void setCloseEyeFrequency(int closeEyeFreqeuncy) {CLOSE_EYE_FREQUENCY = closeEyeFreqeuncy;}

	/**@brief 设置左顾右盼行为单次推理持续时长
	* @param	lookLeftRightDuration	左顾右盼行为单次推理持续时长
	* @return	None
	*/
	void setLookLeftRightDuration(int lookLeftRightDuration) {LOOK_LEFT_RIGHT_DURATION = lookLeftRightDuration;}

	/**@brief 设置左顾右盼行为单次预警需要检测出违规行为的次数
	* @param	lookLeftRightFrequency	左顾右盼行为单次预警需要检测出违规行为的次数
	* @return	None
	*/
	void setLookLeftRightFrequency(int lookLeftRightFrequency) {LOOK_LEFT_RIGHT_FREQUENCY = lookLeftRightFrequency;}

	/**@brief 设置左顾右盼行为偏转角界限
	* @param	lookLeftRightYawAngle	左顾右盼行为偏转角界限
	* @return	None
	*/
	void setLookLeftRightYawAngle(float lookLeftRightYawAngle) {LOOK_LEFT_RIGHT_YAW_ANGLE = lookLeftRightYawAngle;}

	/**@brief 设置长时间直视行为单次推理持续时长
	* @param	lookStraightDuration	长时间直视行为单次推理持续时长
	* @return	None
	*/
	void setLookStraightDuration(int lookStraightDuration) {LOOK_STRAIGHT_DURATION = lookStraightDuration;}

	/**@brief 设置长时间直视行为偏转角界限
	* @param	lookStraightYawAngle	长时间直视行为偏转角界限
	* @return	None
	*/
	void setLookStraightYawAngle(float lookStraightYawAngle) {LOOK_STRAIGHT_YAW_ANGLE = lookStraightYawAngle;}

	/**@brief 设置长时间直视行为俯仰角界限
	* @param	lookStraightPitchAngle	长时间直视行为俯仰角界限
	* @return	None
	*/
	void setLookStraightPitchAngle(float lookStraightPitchAngle) {LOOK_STRAIGHT_PITCH_ANGLE = lookStraightPitchAngle;}

	/**@brief 设置速度阈值1
	* @param	threshold	速度阈值
	* @return	None
	*/
	void setThresholdOne(int threshold) {thresholdOne = threshold;}

	/**@brief 设置速度阈值2
	* @param	threshold	速度阈值
	* @return	None
	*/
	void setThresholdTwo(int threshold) {thresholdTwo = threshold;}

private:
	/**@brief 构造函数
	* @param[in]		path	json配置文件路径
	* @return	None
	*/
	BehaviorConf(const std::string & path){cfgPath = path;}

private:
	std::string SERVER_HOST;				///< 服务器地址
	int SERVER_PORT;						///< 服务器端口
	int SERVER_MODE;						///< 终端连接服务器模式：1/2/3(见配置文件中的描述，现默认tcp短链接，该配置没用)
	int VIDEO_TYPE;							///< 视频类型，摄像头或rtsp
	std::string VIDEO_DEV;					///< 视频设备名
	int VIDEO_CAP_FMT;						///< 视频格式
	int VIDEO_CAP_WIDTH;					///< 采集宽度
	int VIDEO_CAP_HEIGHT;					///< 采集高度
	int VIDEO_CAP_INTERVAL;					///< 采集间隔
	int SAVE_DETECT_PIC;					///< 图像是否保存
	int SAVE_DETECT_VIDEO;					///< 录像是否保存
	int videoDuration;						///< 检测到驾驶员行为保存录像间隔
	int CLOSE_EYE_DURATION;					///< 闭眼行为单次推理持续时长
	int CLOSE_EYE_FREQUENCY;				///< 闭眼行为单次预警需要检测出违规行为的次数
	int LOOK_LEFT_RIGHT_DURATION;			///< 左顾右盼行为单次推理持续时长
	int LOOK_LEFT_RIGHT_FREQUENCY;			///< 左顾右盼行为单次预警需要检测出违规行为的次数
	float LOOK_LEFT_RIGHT_YAW_ANGLE;		///< 左顾右盼行为偏转角界限
	int LOOK_STRAIGHT_DURATION;				///< 长时间直视行为单次推理持续时长
	float LOOK_STRAIGHT_YAW_ANGLE;			///< 长时间直视行为偏转角界限
	float LOOK_STRAIGHT_PITCH_ANGLE;		///< 长时间直视行为俯仰角界限
	std::string picVideoPath;				///< 驾驶行为保存路径
	std::string circleVideoPath;			///< 录像保存路径
	std::string 	cfgPath;				///< 配置文件路径
	bool mppEnable;							///< 是否用硬解码
	bool decodeIDR;							///< 是否只解码IDR
	bool saveDecodeImg;						///< 是否保存解码后的图像
	std::string decodeImgPath;				///< 解码后的图像保存路径
	int thresholdOne;						///< 速度阈值1，大于此值才检测闭眼
	int thresholdTwo;						///< 速度阈值2，大于此值才检测长时间直视与左顾右盼
};
#endif /* BEHAVIORCONF_HPP_ */
