#include <HeadBehaviorDetector.h>
#include <cmath>

/**
 * @brief softmax函数
 * @param input 输入数据vetcor
*/
inline vector<float> softmax(const std::vector<float>& input) {
    std::vector<float> result;
    float sum = 0.0;

    for (float val : input) {
        result.push_back(std::exp(val));
        sum += std::exp(val);
    }

    for (float& val : result) {
        val /= sum;
    }

    return result;
}


HeadBehaviorDetector::HeadBehaviorDetector()
{	
	preProcessImageW = 224;
	preProcessImageH = 224;
	// 以下路径后续可配置为json
    string rknn_model_path = RetinaDriverConf::Instance()->smoke_rknnPath;

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

HeadBehaviorDetector::~HeadBehaviorDetector()
{

}

int HeadBehaviorDetector::imgPreprocess()
{
    // 分类模型预处理：通道转换 + 中心裁剪 + 标准化
    // 由于rknn模型转换问题，不需要进行通道转换 + 标准化操作
    int input_w = inputImg.cols;
    int input_h = inputImg.rows;
    int min = (input_w < input_h) ? input_w : input_h;
    cv::Rect rect((input_w-min)/2, (input_h-min)/2, min, min);
    cv::Mat resimg = inputImg(rect);
    cv::resize(resimg, resimg, cv::Size(preProcessImageW, preProcessImageH));
    cv::cvtColor(resimg, preProcessImage, cv::COLOR_BGR2RGB);
    cv::imwrite("../img/smoke_input.jpg",preProcessImage);
    return 0;
}

int HeadBehaviorDetector::detect()
{
	int ret;

    // 查询模型输入节点、输出节点数量
    rknn_input_output_num io_num;
    rknn_tensor_attr inputs_attr[1];//表示模型的 Tensor 的属性,输入节点属性
    inputs_attr[0].index = 0;
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

	// 释放动态库里面申请的内存
    rknn_outputs_release(ctx, io_num.n_output, outputs);
    return 0;
}

int HeadBehaviorDetector::getResults()
{   
    // 初始化分类结果结构体
    res = {0.0, 0.0};

    if (outData.empty())
    {
        cout << "rknn outData empty!" << endl;
        return -1;
    }

    float *out = ((float *)outData[0]); // 逐个取出outData中的数据

    vector<float> softmax_input; // softmax函数输入
    for (int i = 0; i < 2; i++)
    {
        cout<<"smoke_cls:"<<*out<<endl;
        softmax_input.push_back(*out);
        out++;
    }

        vector<float> softmax_output = softmax(softmax_input);
        res.positive_score = std::round(softmax_output[0]);
        res.smoke_score = std::round(softmax_output[1]);
    spdlog::debug("Smoke_CLS:[positive: {}, smoke: {}]",res.positive_score, res.smoke_score);

    return 0;
}

int HeadBehaviorDetector::getHeadBehavior()
{	

	if (getResults() != 0)
		return -1;
    
    // 判断行为类别
    if (res.positive_score < res.smoke_score)
    {
        return 1;
    }
	
	return 0;
}