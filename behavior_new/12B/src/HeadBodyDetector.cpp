#include <HeadBodyDetector.h>

// 供std::max_elemen函数用的比对函数，求出人像列表和头像列表的置信度最高的box
bool cmp(ClassInfo a,ClassInfo b)
{
      return a.score < b.score;
}

// sigmoid激活函数
float sigmoid(float x)
{	
	if (x >= 0)
	{
		return 1.0 / (1.0 + exp(-x));
	}
	return exp(x) / (1.0 + exp(x));
}

int HeadBodyDetector::calculateGridInfo(float *GridData, int a, int h, int w, float *anchors, float stride)
{	
    // 创建三类ClassInfo,用于整合到ScoreList中
    ClassInfo Head;
    ClassInfo Body;
	ClassInfo Phone;
    GridInfo Grid[a][h][w]; // 创建Grid三维数组
	// 以smallGrid举例 smallGrid共1*3*48*80*8=92160个数据
	// rknn输出数据outputs中，每个数据站两位bit 所以outputs.size为 92160*2
	// 转为float *smallGrid后，每个数据站4位bit, 所以memcpy中size为92160*4
	ssize_t size = a * w * h * 8 * sizeof(*GridData); // 初始化每个尺度的size,用于memcpy内存拷贝到Grid中
    memcpy(Grid, GridData, size);

	// 处理grid中conf数据
	// x = (sigmoid(x)*2 -0.5 + grid_cell_x)*stride
	// y = (sigmoid(y)*2 -0.5 + grid_cell_y)*stride
	// w = (sigmoid(w)*2)^2 * anchors_w
	// h = (sigmoid(h)*2)^2 * anchors_h
	// conf = sigmoid(conf)
	// scores = sigmoid(scores) * conf
    for (int i = 0; i < a; i++)
    {
        for (int j = 0; j < h; j++)
        {
            for (int k = 0; k < w; k++)
            {	
                float conf = Grid[i][j][k].conf;
                conf = sigmoid(conf); // 计算conf数据
                // 阈值过滤,减少计算量
                if (conf > 0.45)
                {   
                    // 计算坐标
                    float x = Grid[i][j][k].x;
                    float y = Grid[i][j][k].y;
                    float w = Grid[i][j][k].w;
                    float h = Grid[i][j][k].h;
                    x = ((sigmoid(x) * 2 - 0.5) + k) * stride; // 计算x
                    y = ((sigmoid(y) * 2 - 0.5) + j) * stride;
                    w = pow(sigmoid(w) * 2, 2) * (*(anchors + (2 * i)));
                    h = pow(sigmoid(h) * 2, 2) * (*(anchors + (2 * i + 1)));

                    // 计算得分
                    float headScore = Grid[i][j][k].headScore;
                    float bodyScore = Grid[i][j][k].bodyScore;
					float phoneScore = Grid[i][j][k].phoneScore;
                    headScore = sigmoid(headScore) * (conf);
                    bodyScore = sigmoid(bodyScore) * (conf);
					phoneScore = sigmoid(phoneScore) * (conf);

                    // 设置当前目标数据
                    setClassInfo(Head, x, y, w, h, headScore);
                    setClassInfo(Body, x, y, w, h, bodyScore);
					setClassInfo(Phone, x, y, w, h, phoneScore);

                    // 将对应ClassInfo加入对应List(此处可以增加阈值过滤,减少ScoreList内存使用)
                    if (Head.score > HeadThres)
                    {   
                        HeadList.push_back(Head);
                    }
                    if (Body.score > BodyThres)
                    {   
                        BodyList.push_back(Body);
                    }
					if (Phone.score > PhoneThres)
					{
						PhoneList.push_back(Phone);
					}	
				}
			}
		}
	}
	return 0;
}

HeadBodyDetector::HeadBodyDetector()
{	
	preProcessImageW = 640;
	preProcessImageH = 640;
	// 以下路径后续可配置为json
    string rknn_model_path = RetinaDriverConf::Instance()->body_head_rknnPath;
	string filePath = RetinaDriverConf::Instance()->ANCHORS_PATH;
	// 获取阈值
	BodyThres = RetinaDriverConf::Instance()->body_thresh;
	HeadThres = RetinaDriverConf::Instance()->head_thresh;
	PhoneThres = 0.536;

	loadAnchors(filePath);
	initModel(rknn_model_path);

	rknn_sdk_version version;
	int ret =0;
	ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
	if(ret != 0){
		std::cout << "获取版本失败" << std::endl;
	}
	else
	{
		for(int i=0; i<5; ++i){
			apiVersion.push_back(version.api_version[i]);
			drvVersion.push_back(version.drv_version[i]);
		}
	}
}

HeadBodyDetector::~HeadBodyDetector()
{

}

int HeadBodyDetector::loadAnchors(string filePath)
{	
    float buf; // 用于读取anchors文件每行内容
               // 读取文件
    ifstream infile;
    infile.open(filePath); // 打开文件
    if (!infile)
    {
        cout << "无法打开文件！" << endl;
        return -1;
    }
    while (!infile.eof()) // 判断是否结束
    {
        infile >> buf;
        anchors.push_back(buf);
    }
    infile.close(); // 关闭文件

	return 0;
}

int HeadBodyDetector::imgPreprocess()
{	
	top_bottom_padding = 0.0;
	left_right_padding = 0.0;
	cv::Mat resimg;	// 压缩后的图像
	// 压缩图片
	rescale_ratio = std::min((float)preProcessImageW/inputImg.cols, (float)preProcessImageH/inputImg.rows);	// 获取压缩比例
	cv::resize(inputImg, resimg, cv::Size(), rescale_ratio, rescale_ratio);	// 压缩图片
	// 图片填充
	if (resimg.cols != resimg.rows)
	{
		if (resimg.cols > resimg.rows)	// w > h
		{
			top_bottom_padding = (preProcessImageH - resimg.rows)/2;
		}
		else
		{
			left_right_padding = (preProcessImageW - resimg.cols)/2;
		}
		cv::copyMakeBorder(resimg, resimg, 
							top_bottom_padding, 
							top_bottom_padding, 
							left_right_padding, 
							left_right_padding, 
							cv::BorderTypes::BORDER_CONSTANT,
							cv::Scalar(114, 114, 114));
	}
	// 检测输入大小
	if (resimg.cols != preProcessImageW || resimg.rows != preProcessImageH)
	{	
		cv::resize(inputImg, resimg, cv::Size(preProcessImageW,preProcessImageH), 0, 0);
	}
	// 通道转换
	cv::cvtColor(resimg, preProcessImage, cv::COLOR_BGR2RGB);
	// cv::imwrite("../img/hbpre.jpg",preProcessImage);
	return 0;
}

int HeadBodyDetector::detect()
{
	int ret;

    // 查询模型输入节点、输出节点数量
    rknn_input_output_num io_num;
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0)
    {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }

	// 创建模型输入输出节点
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    rknn_output outputs[io_num.n_output];
    memset(outputs, 0, sizeof(outputs));

    // 设置输入节点
    inputs[0].index = 0;
    inputs[0].size = preProcessImage.cols * preProcessImage.rows * preProcessImage.channels();
    inputs[0].pass_through = false; // 需要type和fmt
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].fmt = RKNN_TENSOR_NHWC; // tensor格式 NCHW / NHWC
    inputs[0].buf = preProcessImage.data;

    // 数据输入
    ret = rknn_inputs_set(ctx, 1, inputs);
    if (ret < 0)
    {
        cout << "rknn_inputs_set fail! ret=" << ret << endl;
        return -1;
    }

    // 检测处理
    ret = rknn_run(ctx, nullptr);
    if (ret < 0)
    {
        cout << "rknn_run fail! ret=" << ret << endl;
        return -1;
    }

    // rknn 输出数据格式设置
    for (int i = 0; i < io_num.n_output; i++)
    {
        outputs[i].want_float = true;   // 输出数据格式 TURE为float32,FALSE为原始格式
        outputs[i].is_prealloc = false; // 内存分配 TURE为程序员分配，FALSE为rknn自动分配
        outputs[i].index = i;
    }

    // 将输出结果保存在outputs中
    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
    if (ret < 0)
    {
        cout << "rknn_outputs_get fail! ret=" << ret << endl;
        return -1;
    }

    // 清除上次输出数据
    for (auto iter = outData.begin(); iter != outData.end(); iter++)
    {
        char *ptr = (char *)*iter;
        delete[] ptr;
    }
    outData.clear();

    // 保存输出
    for (int i = 0; i < io_num.n_output; i++)
    {   
        float *buf = new float[outputs[i].size];
        memcpy(buf, outputs[i].buf, outputs[i].size); // 将rknn输出拷贝到Buf中
        outData.push_back(buf);                       // 将Buf加入outData中
    }

    // 重置stride
    stride.clear();

    // 检查模型输出节点数量, 若为4则创建内容为 [4,8,16,32] 的数组
    if (io_num.n_output == 4)
    {
        for (int i = 0; i < io_num.n_output; i++)
        {
            float s = 4 * pow(2, i); // s = 4 * 2^i
            stride.push_back(s);
        }
    }
    else
    {
        for (int i = 0; i < io_num.n_output; i++)
        {
            float s = 8 * pow(2, i); // s = 4 * 2^i
            stride.push_back(s);
        }
    }

	// 释放动态库里面申请的内存
    rknn_outputs_release(ctx, io_num.n_output, outputs);

	return 0;
}

ClassInfo xywh2xyxy(ClassInfo Info)
{	
	ClassInfo InfoBak = Info; // 备份ClassInfo
	// x1 = x - w/2
	// y1 = y - h/2
	// x2 = x + w/2
	// y2 = y + h/2
	Info.x = InfoBak.x - InfoBak.w / 2;
	Info.y = InfoBak.y - InfoBak.h / 2;
	Info.w = InfoBak.x + InfoBak.w / 2;
	Info.h = InfoBak.y + InfoBak.h / 2;
	// 此时 info中 xywh 意义变为 x1,y1,x2,y2
	return Info;
}

ClassInfo HeadBodyDetector::getTrueBox(ClassInfo Info)
{	
	// 此时 info中 xywh 意义变为 x1,y1,x2,y2
	ClassInfo InfoBak = Info; // 备份ClassInfo

	// 减去黑框
	Info.x -= left_right_padding;
	Info.w -= left_right_padding;
	Info.y -= top_bottom_padding;
	Info.h -= top_bottom_padding;

	// 除以压缩比例,转换为真实框坐标
	Info.x /= rescale_ratio;
	Info.w /= rescale_ratio;
	Info.y /= rescale_ratio;
	Info.h /= rescale_ratio;
	return Info;

}

int HeadBodyDetector::getBestResult()
{	

	// 初始化结构体
    HeadMax = {0};
    BodyMax = {0};
	PhoneMax = {0};

    // 初始化得分列表
    HeadList.clear();
    BodyList.clear();
	PhoneList.clear();

    if (outData.empty())
    {
        cout << "rknn outData empty!" << endl;
        return -1;
    }

 	// 取出outData中的数据指针
    for (int i = 0; i < outData.size(); i++)
    {   
        // cout<<i<<endl;
        float *Grid = ((float *)outData[i]); // 逐个取出outData中的数据
        float anchor[6];
        // 将对应锚框坐标传入anchor,其中每个grid对应6个锚框数据
        for (int inx = 0; inx < 6; inx++)
        {
            float a = anchors[i * 6 + inx];
            anchor[inx] = a;
        }

        // 获取对应grid网格大小
        float s = stride.front(); // 获取list 中第一个元素
        stride.pop_front();       // 将当前列表第一个元素删除

        // 后处理计算
        calculateGridInfo(Grid, 3, preProcessImageH / s, preProcessImageW / s, anchor, s);
    }



	// 1.取出每个List中的得分最大解
	// 2.xywh2xyxy 坐标转化,info中 xywh 意义变为 x1,y1,x2,y2
	// 3.缩放最优解,转化为真实框坐标
	if (!HeadList.empty())
	{
		HeadMax = *max_element(HeadList.begin(),HeadList.end(),cmp);
		HeadMax = xywh2xyxy(HeadMax);
		HeadMax = getTrueBox(HeadMax);
	}
	if (!BodyList.empty())
	{
		BodyMax = *max_element(BodyList.begin(),BodyList.end(),cmp);
		BodyMax = xywh2xyxy(BodyMax);
		BodyMax = getTrueBox(BodyMax);
	}
	if (!PhoneList.empty())
	{
		PhoneMax = *max_element(PhoneList.begin(),PhoneList.end(),cmp);
		PhoneMax = xywh2xyxy(PhoneMax);
		PhoneMax = getTrueBox(PhoneMax);
	}

	spdlog::debug("Head:[x1={};y1={};x2={};y2={};socre={}]",HeadMax.x, HeadMax.y, HeadMax.w, HeadMax.h, HeadMax.score);
	spdlog::debug("Body:[x1={};y1={};x2={};y2={};socre={}]",BodyMax.x, BodyMax.y, BodyMax.w, BodyMax.h, BodyMax.score);
	spdlog::debug("Phone:[x1={};y1={};x2={};y2={};socre={}]",PhoneMax.x, PhoneMax.y, PhoneMax.w, PhoneMax.h, PhoneMax.score);
	return 0;
}

int HeadBodyDetector::createHeadBodyImage()
{	

	if (getBestResult() != 0)
		return -1;

	//将头像和人像清空，外部获取头像和人像检测成功与否，是判断图像是否为空
	HeadImage = cv::Mat();
	BodyImage = cv::Mat();

	float headScore = HeadMax.score;
	float bodyScore = BodyMax.score;
	if (headScore > HeadThres)
	{
		cv::Rect preRect = getHeadBox();	//获取头部先验框
		HeadImage = inputImg(preRect);
	}

	if (bodyScore != 0)
	{
		BodyImage = inputImg(getBodyBox());
	}
	return 0;
}

cv::Rect HeadBodyDetector::getHeadBox()
{	
	cv::Rect ret(0,0,0,0);

	float x1 = HeadMax.x-10;
	float y1 = HeadMax.y-10;
	float x2 = HeadMax.w+10;
	float y2 = HeadMax.h+20;

	if (HeadMax.score > HeadThres)
	{
		// 坐标限定，检查越界
		if(x1 < 0)
			x1 = 0;
		if(x2 < 0)
			x2 = 0;
		if(y1 < 0)
			y1 = 0;
		if(y2 < 0)
			y2 = 0;

		if(x1 > inputImg.cols)
			x1 = inputImg.cols;
		if(x2 > inputImg.cols)
			x2 = inputImg.cols;
		if(y1 > inputImg.rows)
			y1 = inputImg.rows;
		if(y2 > inputImg.rows)
			y2 = inputImg.rows;		
		ret = cv::Rect(x1,y1,x2-x1,y2-y1);	
	}
	return ret;
}

cv::Rect HeadBodyDetector::getBodyBox()
{	
	cv::Rect ret(0,0,0,0);

	float x1 = BodyMax.x;
	float y1 = BodyMax.y;
	float x2 = BodyMax.w;
	float y2 = BodyMax.h;

	if (BodyMax.score > BodyThres)
	{
		// 坐标限定，检查越界
		if(x1 < 0)
			x1 = 0;
		if(x2 < 0)
			x2 = 0;
		if(y1 < 0)
			y1 = 0;
		if(y2 < 0)
			y2 = 0;

		if(x1 > inputImg.cols)
			x1 = inputImg.cols;
		if(x2 > inputImg.cols)
			x2 = inputImg.cols;
		if(y1 > inputImg.rows)
			y1 = inputImg.rows;
		if(y2 > inputImg.rows)
			y2 = inputImg.rows;		
		ret = cv::Rect(x1,y1,x2-x1,y2-y1);	
	}
	return ret;
}

cv::Mat HeadBodyDetector::getHeadImage()
{
	return HeadImage;
}

cv::Mat HeadBodyDetector::getBodyImage()
{
	return BodyImage;
}

cv::Rect HeadBodyDetector::getPhoneBox()
{	
	cv::Rect ret(0,0,0,0);

	float x1 = PhoneMax.x;
	float y1 = PhoneMax.y;
	float x2 = PhoneMax.w;
	float y2 = PhoneMax.h;

	if (PhoneMax.score > PhoneThres)
	{
		// 坐标限定，检查越界
		if(x1 < 0)
			x1 = 0;
		if(x2 < 0)
			x2 = 0;
		if(y1 < 0)
			y1 = 0;
		if(y2 < 0)
			y2 = 0;

		if(x1 > inputImg.cols)
			x1 = inputImg.cols;
		if(x2 > inputImg.cols)
			x2 = inputImg.cols;
		if(y1 > inputImg.rows)
			y1 = inputImg.rows;
		if(y2 > inputImg.rows)
			y2 = inputImg.rows;		
		ret = cv::Rect(x1,y1,x2-x1,y2-y1);	
	}
	return ret;
}

int HeadBodyDetector::getBodyBehavior()
{	
    // 判断是否检测到手机
	float phoneScore = PhoneMax.score;
	if (phoneScore > PhoneThres)
	{   
		return 1;
	}
	return 0;
}



std::string HeadBodyDetector::getRknnApiVer()
{
	return apiVersion;
}

std::string HeadBodyDetector::getRknnDrvVer()
{
	return drvVersion;
}