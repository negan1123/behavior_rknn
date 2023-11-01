/*
 * UploadBehavior.cpp
 *
 *  Created on: 2021年1月8日
 *      Author: syukosen
 */
#include <UploadBehavior.h>


using namespace asio;

#define SEND_BUF_SIZE		(50*1024*1024)
/**@union uBigLittleEndianChange
	* @brief 整形，float大小端转化联合
*/
typedef union _uBigLittleEndianChange_ {
	struct{
		char byte3;
		char byte2;
		char byte1;
		char byte0;
	}sBytes;				///<	转化的字节结构
	float fData;		///<	需转换的float值
	int iData;			///<	需转换的int值
}uBigLittleEndianChange;

unsigned int *table=NULL;			///<	crc32码表
static bool isTableInit = false;	///<	crc32码表是否初始化
/**@brief 位逆转
* @param[in] 	input   多项式值
* @param[in] 	bw   		多项式的位数
* @retval		unsigned int  位逆转后的值
*/

static unsigned int bitrev(unsigned int input, int bw)
{
	int i;
	unsigned int var;
	var = 0;
	for(i=0;i<bw;i++)
	{
		if(input & 0x01)
		{
			var |= 1<<(bw-1-i);
		}
		input>>=1;
	}
	return var;
}

/**@brief 码表生成
* @param[in] 	poly   多项式值 如:X32+X26+...X1+1,poly=(1<<26)|...|(1<<1)|(1<<0)
* @retval		None
*/
void crc32_init(unsigned int poly)
{
	int i;
	int j;
	unsigned int c;
	if(NULL == table)
		table = (unsigned int *)malloc(256*sizeof(unsigned int));
	poly=bitrev(poly,32);
	for(i=0; i<256; i++){
		c = i;
		for (j=0; j<8; j++){
			if(c&1){
				c=poly^(c>>1);
			}
			else{
				c=c>>1;
			}
		}
		table[i] = c;
	}
	isTableInit = true;
}

/**@brief crc32校验算法，多项式为值0x4C11DB7
* @param[in] 	crc   初始值
* @param[in] 	input  校验数据
* @param[in] 	len   校验数据长度
* @retval		None
*/
unsigned int crc32(unsigned int crc, void* input, int len){
	int i;
	unsigned char index;
	unsigned char* pch;
	pch = (unsigned char*)input;
	if(!isTableInit)
		crc32_init(0x4C11DB7);
	for(i=0;i<len;i++){
		index = (unsigned char)(crc^*pch);
		crc = (crc>>8)^table[index];
		pch++;
	}
	return crc;
}

/**@brief 字符串转换为大端的int
* @param[in] 		p		 字符串
* @param[out] 	i  转换后的int值
* @retval		None
*/
void chartoint(unsigned char * p, int & i){
	unsigned char tmp1,tmp2,tmp3,tmp4;
	tmp1 = *p++;
	tmp2 = *p++;
	tmp3 = *p++;
	tmp4 = *p;
	i = (tmp1 << 24) | (tmp2 << 16) | (tmp3 << 8) | tmp4;
}

/**@brief int转换为字符串
* @param[out] p		 转换后字符串
* @param[in] 	i  	 转换的int值
* @retval		None
*/
void inttochar(unsigned char * p, int & i){
	*p++ = (i >> 24) & 0xFF;
	*p++ = (i >> 16) & 0xFF;
	*p++ = (i >> 8) & 0xFF;
	*p = i & 0xFF;
}


UploadBehavior::UploadBehavior(std::string server, int port, std::string savePath)
{	
	cout<<"UploadBehavior 初始化"<<endl;
	this->server = server;
	this->port = port;
	this->savePath = savePath;
	sendbuf = new char[SEND_BUF_SIZE];
	IMEI = BehaviorConf::Instance()->getImei();
	IMSI = BehaviorConf::Instance()->getImsi();
	std::string mac = BehaviorConf::Instance()->getMac();
	sscanf(mac.c_str(),"%02X:%02X:%02X:%02X:%02X:%02X", &MAC[0],&MAC[1],&MAC[2],&MAC[3],&MAC[4],&MAC[5]);
	VER = "1.0.1";
	mSaveVideo = BehaviorConf::Instance()->isSaveDetectVideo();
}

UploadBehavior::~UploadBehavior()
{
	if(nullptr != sendbuf)
	{
		delete[] sendbuf;
		sendbuf = nullptr;
	}
}

void UploadBehavior::queueBehavior(std::string behaviorpath)
{
	std::lock_guard<std::mutex> lck (mtx);
	if(behaviorQueue.size() > 100)
	{
		remove(behaviorpath.c_str());
		return;
	}

	behaviorQueue.push(behaviorpath);
	// 保存了录像
	if(mSaveVideo)
	{
		if(behaviorPic2VideoQueue.size() > 100)
			return;
		std::size_t found = behaviorpath.find_last_of('.');
		std::string videoPath = behaviorpath.substr(0, found) + ".mp4";
		behaviorPic2VideoQueue.push(videoPath);
	}
}

int UploadBehavior::packetDataAndSend(time_t time, int behavior, char pictype, char *picdata, int picdatalen)
{
	// tcp 短链接发送行为和图片
	int ret = -1;
	io_context io_context;
	asio::error_code error;
	ip::tcp::socket socket(io_context);
	ip::tcp::endpoint remove_ep(ip::address_v4::from_string(server), port);
	try{
		//与服务器连接
		socket.connect(remove_ep, error);
		if(error)
		{
			spdlog::error("连接TCP服务器失败！");
			// spdlog::error(error.message());
		}
		else
		{
			char *cmd = sendbuf;
			int index = 0;
			int iconvert;
			int crc32value = 0;
			/* cmdlen =  IMEI  +   IMSI   +  MAC   +  time   +  behavior   +  ver  +  PHONE  +  CARPLATENUM  +  DEVICE_ID  + 图片类型 + piclen
			 * 			 8字节  +   8字节   +  6字节  +  4字节   +  2字节     +  3字节  +  8字节  +  8字节  +  26字节  + 1字节
			 * */
			int cmdlen = 77 + picdatalen;
			uBigLittleEndianChange uchange;
			// 添加帧头
			*(cmd+index) = 0x20; index++;
			*(cmd+index) = 0x30; index++;
			// 添加长度
			uchange.iData = cmdlen;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;
			// 将imei前8个字节转换为int,大端模式发送
			std::string changeStr = IMEI.substr(0,8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 将imei后5个字节转换为int,大端模式发送
			changeStr = IMEI.substr(8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 将imsi前8个字节转换为int,大端模式发送
			changeStr = IMSI.substr(0,8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 将imsi后7个字节转换为int,大端模式发送
			changeStr = IMSI.substr(8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 添加MAC
			for(int i = 0; i < 6; i++)
			{
				*(cmd+index) = MAC[i]; index++;
			}

			// 大端模式发送定位时间
			uchange.iData = time+8*3600;
			//uchange.iData = time;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 添加行为
			*(cmd+index) = (behavior >> 8) & 0XFF; index++;
			*(cmd+index) = behavior & 0XFF; index++;

			// 版本号
			std::string var1,var2,var3;
			std::stringstream istring;
			istring << VER;
			std::getline(istring,var1,'.');
			std::getline(istring,var2,'.');
			std::getline(istring,var3,'.');
			*(cmd+index) = std::stoi(var1); index++;
			*(cmd+index) = std::stoi(var2); index++;
			*(cmd+index) = std::stoi(var3); index++;

			// 将手机号前8个字节转换为int,大端模式发送
			std::string phone;
			phone = VAITerminal_JTConf::Instance()->phone;
			changeStr = phone.substr(0,8);
			iconvert = stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 将手机号后3个字节转换为int,大端模式发送
			changeStr = phone.substr(8);
			iconvert = stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 字符串形式发送车牌号
			std::string carPlateNum = VAITerminal_JTConf::Instance()->carPlateNum;
			char outBody[1024] = {0};
			int outlen = 1024;
			libjt808::u2g((char *)carPlateNum.data(), carPlateNum.size(), outBody, outlen);
			carPlateNum = std::string(outBody);

			if(carPlateNum.size() < 11){
				int addNum = 11 -carPlateNum.size();
				for(int i = 0;i < addNum; i++){
					carPlateNum = carPlateNum + "&";
				}
			}
			if(carPlateNum.size() > 11){
				int deleteNum = carPlateNum.size() - 11;
				for(int i = 0;i<deleteNum;i++){
					carPlateNum.pop_back();
				}
			}			
			for(auto &ch: carPlateNum){
				*(cmd+index) = ch; index++;
			}

			// 字符串形式发送设备ID
			std::string device_id;
			device_id = VAITerminal_JTConf::Instance()->getDeviceDevId();
			for(auto &ch: device_id){
				*(cmd+index) = ch; index++;
			}

			// 添加图片类型
			*(cmd+index) = pictype; index++;

			// 添加图片
			memcpy(cmd+index, picdata, picdatalen);	index+=picdatalen;

			// 数据校验
			crc32value = crc32(0xFFFFFFFF, (void*)cmd, index);
			crc32value ^= 0xFFFFFFFF;
			inttochar((unsigned char *)&cmd[index],crc32value);	index += 4;

			// 发送注册信息
			std::cout<<"-->packetDataAndSend：发送信息"<<std::endl;
			socket.write_some(buffer(cmd, index), error);
			std::cout<<"<--packetDataAndSend：done"<<error<<std::endl;
			if(!error)
			{
#if 1
				char buff[256] = {0};
				int len = socket.read_some(buffer(buff, 256), error);
				
				/**
				 * 2字节包头(0x2430)   1字节状态码   32字节设备型号   32字节设备ID  32字节千寻账号  32字节密码  包尾(0x0A)
				 */
				if(len >= 4)
				{
					if(buff[0] == 0x20 && buff[1] == 0x30)
					{
						char errcode = buff[2];
						if(0 == errcode)
						{
						}
					}
					std::cout<<"<--packetDataAndSend：接收信息长度 "<<len<<std::endl;
					spdlog::info("收到图片回复");
					cout<<"收到图片回复"<<endl;
					ret = 0;
				}
				else
				{
					spdlog::debug("注册服务器回复数据长度错误");
					cout<<"注册服务器回复数据长度错误"<<endl;
				}
#endif
			}
			else
			{
				spdlog::debug("注册向服务器发送指令失败");
			}
			socket.close();
		}
	}
	catch(...)
	{
		std::string log_msg = "catch err";
		// spdlog::debug(log_msg);
	}
	return ret;
}

int UploadBehavior::packetVideoDataAndSend(time_t time, int behavior, char *picdata, int picdatalen)
{
	// tcp 短链接发送行为和图片
	int ret = -1;
	io_context io_context;
	asio::error_code error;
	ip::tcp::socket socket(io_context);
	ip::tcp::endpoint remove_ep(ip::address_v4::from_string(server), port);
	try{
		//与服务器连接
		socket.connect(remove_ep, error);
		if(error)
		{
			spdlog::error("连接TCP服务器失败！");
		}
		else
		{
			char *cmd = sendbuf;
			int index = 0;
			int iconvert;
			int crc32value = 0;
			/* cmdlen =  IMEI  +   IMSI   +  MAC   +  time   +  behavior   +  ver  + piclen
			 * 			 8字节  +   8字节   +  6字节  +  4字节   +  2字节     +  3字节
			 * */
			int cmdlen = 76 + picdatalen;
			uBigLittleEndianChange uchange;
			// 添加帧头
			*(cmd+index) = 0x20; index++;
			*(cmd+index) = 0x31; index++;
			// 添加长度
			uchange.iData = cmdlen;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;
			// 将imei前8个字节转换为int,大端模式发送
			std::string changeStr = IMEI.substr(0,8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 将imei后5个字节转换为int,大端模式发送
			changeStr = IMEI.substr(8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 将imsi前8个字节转换为int,大端模式发送
			changeStr = IMSI.substr(0,8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 将imsi后7个字节转换为int,大端模式发送
			changeStr = IMSI.substr(8);
			iconvert = std::stoi(changeStr);
			uchange.iData = iconvert;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 添加MAC
			for(int i = 0; i < 6; i++)
			{
				*(cmd+index) = MAC[i]; index++;
			}

			// 大端模式发送定位时间
			uchange.iData = time+8*3600;
			//uchange.iData = time;
			*(cmd+index) = uchange.sBytes.byte0; index++;
			*(cmd+index) = uchange.sBytes.byte1; index++;
			*(cmd+index) = uchange.sBytes.byte2; index++;
			*(cmd+index) = uchange.sBytes.byte3; index++;

			// 添加行为
			*(cmd+index) = (behavior >> 8) & 0XFF; index++;
			*(cmd+index) = behavior & 0XFF; index++;

			// 版本号
			std::string var1,var2,var3;
			std::stringstream istring;
			istring << VER;
			std::getline(istring,var1,'.');
			std::getline(istring,var2,'.');
			std::getline(istring,var3,'.');
			*(cmd+index) = std::stoi(var1); index++;
			*(cmd+index) = std::stoi(var2); index++;
			*(cmd+index) = std::stoi(var3); index++;

            // 将手机号前8个字节转换为int,大端模式发送
            std::string phone;
            phone = VAITerminal_JTConf::Instance()->phone;
            changeStr = phone.substr(0,8);
            iconvert = stoi(changeStr);
            uchange.iData = iconvert;
            *(cmd+index) = uchange.sBytes.byte0; index++;
            *(cmd+index) = uchange.sBytes.byte1; index++;
            *(cmd+index) = uchange.sBytes.byte2; index++;
            *(cmd+index) = uchange.sBytes.byte3; index++;

            // 将手机号后3个字节转换为int,大端模式发送
            changeStr = phone.substr(8);
            iconvert = stoi(changeStr);
            uchange.iData = iconvert;
            *(cmd+index) = uchange.sBytes.byte0; index++;
            *(cmd+index) = uchange.sBytes.byte1; index++;
            *(cmd+index) = uchange.sBytes.byte2; index++;
            *(cmd+index) = uchange.sBytes.byte3; index++;

            // 字符串形式发送车牌号
            std::string carPlateNum = VAITerminal_JTConf::Instance()->carPlateNum;
            std::cout<<carPlateNum<<std::endl;
            char outBody[1024] = {0};
            int outlen = 1024;
            libjt808::u2g((char *)carPlateNum.data(), carPlateNum.size(), outBody, outlen);
			// std::cout<< outBody <<std::endl;
			// std::cout<< outlen <<std::endl;
            carPlateNum = std::string(outBody);
			// std::cout<<carPlateNum<<std::endl;
			// std::cout<<"车牌号字节数："<<carPlateNum.size()<<std::endl;
			if(carPlateNum.size() < 11){
				int addNum = 11 -carPlateNum.size();
				for(int i = 0;i < addNum; i++){
					carPlateNum = carPlateNum + "&";
				}
			}
			if(carPlateNum.size() > 11){
				int deleteNum = carPlateNum.size() - 11;
				for(int i = 0;i<deleteNum;i++){
					carPlateNum.pop_back();
				}
			}


            for(auto &ch: carPlateNum){
                    *(cmd+index) = ch; index++;
            }

            // 字符串形式发送设备ID
            std::string device_id;
            device_id = VAITerminal_JTConf::Instance()->getDeviceDevId();
            for(auto &ch: device_id){
                    *(cmd+index) = ch; index++;
            }

			// 添加图片
			memcpy(cmd+index, picdata, picdatalen);	index+=picdatalen;

			// 数据校验
			crc32value = crc32(0xFFFFFFFF, (void*)cmd, index);
			crc32value ^= 0xFFFFFFFF;
			inttochar((unsigned char *)&cmd[index],crc32value);	index += 4;

			// 发送注册信息
			socket.write_some(buffer(cmd, index), error);
			if(!error)
			{
#if 1
				char buff[256] = {0};
				int len = socket.read_some(buffer(buff, 256), error);
				/**
				 * 2字节包头(0x2430)   1字节状态码   32字节设备型号   32字节设备ID  32字节千寻账号  32字节密码  包尾(0x0A)
				 */
				if(len >= 4)
				{
					if(buff[0] == 0x20 && buff[1] == 0x31)
					{
						char errcode = buff[2];
						if(0 == errcode)
						{
						}
					}
					spdlog::info("收到视频回复");
					cout<<"video:收到视频回复"<<endl;
					ret = 0;
				}
				else
				{
					spdlog::debug("注册服务器回复数据长度错误");
					cout<<"video:注册服务器回复数据长度错误"<<endl;
				}
#endif
			}
			else
			{
				// spdlog::debug("注册向服务器发送指令失败");
				cout<<"video:注册向服务器发送指令失败"<<endl;
			}
			socket.close();
		}
	}
	catch(...)
	{
		std::string log_msg = "catch err";
		// spdlog::debug(log_msg);
	}
	return ret;
}

void UploadBehavior::sendBehaviorVideo()
{
	std::string front;
	size_t filenamestart;
	std::string filename;
	time_t time;
	int	   behavier;

	// 从队列里面获取要发送的视频名称
	mtxVideo.lock();
	if(behaviorVideoQueue.empty())
	{
		mtxVideo.unlock();
		return;
	}
	front = behaviorVideoQueue.front();
	mtxVideo.unlock();
	filenamestart = front.find_last_of("/");
	if(filenamestart != std::string::npos)
	{
		filename = front.substr(filenamestart+1);
	}
	else
	{
		filename = front;
	}
	// 队列里面的视频名词格式  前面是时间，后面是行为 34324424_3445.mp4
	// 解析时间，行为和视频后缀
	std::string::size_type sz;
	sz = filename.find("_");
	time = std::stoi(filename.substr(0,sz));
	behavier = std::stoi(filename.substr(sz+1));

	// 读取视频数据
	std::ifstream is (front.c_str(), std::ifstream::binary);
	if (is) {
		// get length of file:
		int sendret;
		int length;
		is.seekg (0, is.end);
		length = is.tellg();
		is.seekg (0, is.beg);
		if(length <= 0)
		{
			mtxVideo.lock();
			behaviorVideoQueue.pop();
			remove(front.c_str());
			is.close();
			mtxVideo.unlock();
			return;
		}
		char * buffer = new char [length];
		is.read (buffer,length);
		is.close();

		// 按照协议格式发送图片和行为给服务器
		sendret = packetVideoDataAndSend(time, behavier, buffer, length);
		delete[] buffer;
		if(0 == sendret)
		{
			mtxVideo.lock();
			behaviorVideoQueue.pop();
			remove(front.c_str());
			mtxVideo.unlock();
		}
	}
	else
	{
		mtxVideo.lock();
		behaviorVideoQueue.pop();
		mtxVideo.unlock();
	}
}

void UploadBehavior::sendBehavior()
{
	std::string front;
	size_t filenamestart;
	std::string filename;
	time_t pic_time;
	int	   pic_behavier;
	char   pictype;
	std::string filetype;

	// 从队列里面获取要发送的图片名称
	mtx.lock();
	if(behaviorQueue.empty())
	{
		mtx.unlock();
		return;
	}
	front = behaviorQueue.front();
	mtx.unlock();
	filenamestart = front.find_last_of("/");
	if(filenamestart != std::string::npos)
	{
		filename = front.substr(filenamestart+1);
	}
	else
	{
		filename = front;
	}
	// 队列里面的图片名词格式  前面是时间，后面是行为 34324424_3445.jpg
	// 解析时间，行为和图片后缀
	std::string::size_type sz;
	sz = filename.find("_");
	pic_time = std::stoi(filename.substr(0,sz));
	pic_behavier = std::stoi(filename.substr(sz+1));
	filetype = filename.substr(filename.find_last_of(".")+1);
	if(filetype == std::string("jpg") || filetype == std::string("JPG") || filetype == std::string("jpeg") || filetype == std::string("JPEG"))
		pictype = 0;
	else
		pictype = 1;

	// 读取图片数据
	std::ifstream is (front.c_str(), std::ifstream::binary);
	if (is) {
		// get length of file:
		int sendret;
		int length;
		is.seekg (0, is.end);
		length = is.tellg();
		is.seekg (0, is.beg);
		if(length <= 0)
		{
			mtx.lock();
			behaviorQueue.pop();
			remove(front.c_str());
			is.close();
			mtx.unlock();
			return;
		}
		char * buffer = new char [length];
		is.read (buffer,length);
		is.close();
		// 按照协议格式发送图片和行为给服务器
		sendret = packetDataAndSend(pic_time, pic_behavier, pictype, buffer, length);
		delete[] buffer;
		mtx.lock();
		if(0 == sendret)
		{	
			std::cout<<"sendBehavior:消息发送成功 "<<sendret<<std::endl;
			behaviorQueue.pop();
			remove(front.c_str());
		}
		mtx.unlock();
	}
	else
	{
		mtx.lock();
		behaviorQueue.pop();
		mtx.unlock();
	}
}

void UploadBehavior::pic2Video()
{
	std::string front;
	size_t filenamestart;
	std::string filename;
	time_t video_time;
	int ret;
	// 从队列里面获取要生成的录像行为
	mtx.lock();
	if(behaviorPic2VideoQueue.empty())
	{
		mtx.unlock();
		return;
	}
	front = behaviorPic2VideoQueue.front();
	mtx.unlock();

	filenamestart = front.find_last_of("/");
	if(filenamestart != std::string::npos)
	{
		filename = front.substr(filenamestart+1);
	}
	else
	{
		filename = front;
	}
	// 队列里面的图片名词格式  前面是时间，后面是行为 34324424_3445.jpg
	// 解析时间，行为和图片后缀
	std::string::size_type sz;
	sz = filename.find("_");
	video_time = std::stoi(filename.substr(0,sz));

	ret = RtspVideoCircleRecorder::Instance()->cutBehaviorVideo(video_time, front);
	if(-1 == ret)
	{
		mtx.lock();
		behaviorPic2VideoQueue.pop();
		mtx.unlock();
	}
	else if(0 == ret)
	{
		mtx.lock();
		behaviorPic2VideoQueue.pop();
		mtx.unlock();

		mtxVideo.lock();
		behaviorVideoQueue.push(front);
		mtxVideo.unlock();
	}
}

void UploadBehavior::initLastData()
{
	struct dirent* ent(0);
	DIR* pDir(opendir(savePath.c_str()));
	std::string absolutePath;
	if(pDir == NULL)
	{
		return;
	}
	// 查找设备目录下的所有天数文件加，保存在dirlist里
	while ((ent = readdir(pDir)) != 0)
	{
		absolutePath = savePath+"/"+std::string(ent->d_name);
		/**在Linux文件系统中 .和..也是特殊的子目录*/
		if(strstr(ent->d_name,".jpg") != NULL)
		{
			if(strstr(ent->d_name, "forRockX.jpg") != NULL)
				remove(ent->d_name);
			else
				behaviorQueue.push(absolutePath);
		}
		if(strstr(ent->d_name,".mp4") != NULL)
		{
			behaviorVideoQueue.push(absolutePath);
		}
	}
	closedir(pDir);
}

void UploadBehavior::start()
{
	// 将上次没有传送成功的输入加入上传队列
	initLastData();
	
	// 开启一个线程进行上传图像
	std::thread threadUpdata([&]() {
		while(1)
		{
			// std::cout<<"behaviorVideoQueue size:"<<behaviorVideoQueue.size()<<std::endl;
			sendBehavior();
			if(mSaveVideo)
				sendBehaviorVideo();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});
	threadUpdata.detach();
	// 开启了记录录像，进行录像截取
	if(mSaveVideo)
	{
		std::thread threadPic2Video([&]() {
			while(1)
			{
				pic2Video();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		});
		threadPic2Video.detach();
	}
}
