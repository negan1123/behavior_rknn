/*
 * RetinaDriverConf.hpp
 *
 *  Created on: 2021年4月14日
 *      Author: syukousen
 */

#ifndef RETINADRIVERCONF_HPP_
#define RETINADRIVERCONF_HPP_
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
using json = nlohmann::json;

/**@class RetinaDriverConf
 * @brief 驾驶员行为分析配置参数类，将数据从retina_driver_config.ini读取出来，写入到相应的数据域 \n
 *    [retinatk]  \n
 *    rknn_path=...../retinatk.rknn  \n
 *    priors_path=..../priors.bin  \n
 *    variance=0.2,0,1			 \n
 *    mean=104, 117, 123		 \n
 *    conf_thresh=0.05			 \n
 *    [driver_behavior]			 \n
 *    rknn_path=...../driver_behavior.rknn  \n
 *    mean=0.485, 0.456, 0.406   \n
 *    std=0.229, 0.224, 0.225   \n
 * */
class RetinaDriverConf
{
public:
	/**@brief 类的单例模式
	* @param	path	json配置文件路径
	* @return	ConfigJson*		BehaviorConf单例
	*/
	static RetinaDriverConf * Instance(std::string path = std::string())
	{
		static RetinaDriverConf * _Instance = nullptr;
		if(nullptr == _Instance)
		{
			_Instance = new RetinaDriverConf();
			_Instance->load();
		}
		return _Instance;
	}
public:
	string body_head_rknnPath;		///< 分割rknn模型路径
	string ANCHORS_PATH;			///< yolov5先验框文件路径
	float head_thresh;				///< 头部阈值
	float body_thresh;				///< 身体阈值

	string smoke_rknnPath;			///< 抽烟模型

private:
	/**@brief 从retina_driver_config.ini文件中将参数加载到对象中
	* @param	None
	* @return	int 加载结果
	* 			0  	成功返回
	* 			-1  失败返回
	*/
	int load(void)
	{
		confFilePath = "../conf/retina_driver_config.json";
		std::ifstream ifile((const char *)confFilePath.c_str());
		if (!ifile.is_open())
		{
			std::cerr << "未能成功加载 " << confFilePath << std::endl;
			return -1;
		}
		json jInfo;
		try
		{	
			ifile >> jInfo;
			json retinatk;
			json head_behavior;
			json body_behavior;

			json body_head;
			json smoke;


			body_head = jInfo.at("body_head");
			smoke = jInfo.at("smoke_cls");

			body_head_rknnPath = body_head.at("rknn_path");
			ANCHORS_PATH = body_head.at("anchors_path");
			body_thresh = body_head.at("body_thresh");
			head_thresh = body_head.at("head_thresh");

			smoke_rknnPath = smoke.at("rknn_path");

		}
		catch(...)
		{
			std::cerr << "parse retina_driver_config file err" << std::endl;
			return -1;
		}
		return 0;
	}

private:
	std::string confFilePath;			///< 常量，保存配置文件路径
};
#endif /* RETINADRIVERCONF_HPP_ */
