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
	std::string RETINATK_RKNN_PATH;	///< person和head检测的模型路径
	std::string RETINATK_PRIORS_PATH; ///< person和head检测的priors.bin文件路径
	std::string RETINATK_VARIANCE;	///< person和head检测的variance参数
	std::string RETINATK_MEAN;		///< person和head检测的mean参数
	std::string RETINATK_CONF_THRESH;	///< person和head检测的置信度阈值参数
	std::string BOD_RKNN_PATH;		///< 驾驶员行为分类的模型文件路径
	std::string BOD_MEAN;				///< 驾驶员行为分类的mean参数
	std::string BOD_STD;				///< 驾驶员行为分类的std参数
	std::string HEAD_BEHAVIOR_RKNN_PATH;	///< 头部行为检测模型路径
	std::string BODY_BEHAVIOR_RKNN_PATH;	///< 身体行为检测模型路径

	string body_head_rknnPath;		///< 分割rknn模型路径
	string ANCHORS_PATH;			///< yolov5先验框文件路径
	float head_thresh;				///< 头部阈值
	float body_thresh;				///< 身体阈值

	string phone_rknnPath;			///< 手机模型路径
	float phone_thresh;				///< 手机阈值

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
			json phone;
			json smoke;

			retinatk = jInfo.at("retinatk");
			head_behavior = jInfo.at("head_behavior");
			body_behavior = jInfo.at("body_behavior");
			body_head = jInfo.at("body_head");
			phone = jInfo.at("phone");
			smoke = jInfo.at("smoke_cls");

			RETINATK_RKNN_PATH = retinatk.at("rknn_path");
			RETINATK_PRIORS_PATH = retinatk.at("priors_path");
			RETINATK_VARIANCE = retinatk.at("variance");
			RETINATK_MEAN = retinatk.at("mean");
			RETINATK_CONF_THRESH = retinatk.at("conf_thresh");
			HEAD_BEHAVIOR_RKNN_PATH = head_behavior.at("rknn_path");
			BODY_BEHAVIOR_RKNN_PATH = body_behavior.at("rknn_path");

			body_head_rknnPath = body_head.at("rknn_path");
			ANCHORS_PATH = body_head.at("anchors_path");
			body_thresh = body_head.at("body_thresh");
			head_thresh = body_head.at("head_thresh");

			phone_rknnPath = phone.at("rknn_path");
			phone_thresh = phone.at("phone_thresh");

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
