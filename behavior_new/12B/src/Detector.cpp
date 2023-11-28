# include <Detector.h>

static unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char *)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if (model_len != fread(model, 1, model_len, fp))
    {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }
    *model_size = model_len;
    if (fp)
    {
        fclose(fp);
    }
    return model;
}

Detector::Detector()
{

}

Detector::~Detector()
{
    deInitModel();
}

int Detector::initModel(string rknnPath)
{
    // Load model
	int ret = 0;    //rknn_api返回值
	int model_len = 0;  //rknn模型长度
	model = load_model(rknnPath.c_str(), &model_len);

    //调用rknn_init()初始化rknn模型
    ret = rknn_init(&ctx,model,model_len,0);
    std::cout<<"rknn_model:"<<model<<std::endl;
	delete [](char*)model;//释放model空间
	model = nullptr;
	if(ret < 0)
	{
        cout<<"rknn_init fail! ret="<<ret<<endl;
		return -1;
	}

	showModelInfo();
	return 0;
}

int Detector::deInitModel()
{
    // 释放模型的内存
	if(nullptr != model)
	{
		delete [](char*)model;
		model = nullptr;
	}

	// 打开模型文件，获取模型数据
	for(auto iter=outData.begin();iter!=outData.end();++iter)
	{
		char *ptr = (char*)*iter;
		delete []ptr;
	}
	outData.clear();
	return 0;
}

void Detector::showModelInfo()
{
	int ret = 0;
	// 查询rknn sdk和驱动版本
	rknn_sdk_version version;
	ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
	if(ret != 0){
		std::cout << "获取版本失败" << std::endl;
	}
	apiVersion = version.api_version;
	drvVersion = version.drv_version;
	printf("sdk version: %s driver version: %s\n", apiVersion, drvVersion);

	rknn_input_output_num io_num;//input 和 output 的 Tensor 个数
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC)
    {
        printf("rknn_query fail! ret=%d\n", ret);
        return;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

	rknn_tensor_attr inputs_attr[1];//表示模型的 Tensor 的属性,输入节点属性
	rknn_tensor_attr outputs_attr[10];//输出节点属性
	inputs_attr[0].index = 0;
	outputs_attr[0].index = 0;
	outputs_attr[1].index = 1;
	ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(inputs_attr[0]), sizeof(rknn_tensor_attr));//获取某个输入节点的属性
	if(ret < 0) {
        cout<<"RKNN_QUERY_INPUT_ATTR fail! ret="<<ret<<endl;
	    return;
	}
	printf("intput ---------------start\n");
	printf("n_dims = %d\n",inputs_attr[0].n_dims);
	printf("dims array =");
	for(uint32_t j = 0; j < inputs_attr[0].n_dims; j++)
	{
		printf(" %d",inputs_attr[0].dims[j]);
	}
	printf("\n");
	printf("name = %s\t",inputs_attr[0].name);
	printf("n_elems = %d\t",inputs_attr[0].n_elems);
	printf("size = %d\t",inputs_attr[0].size);
	printf("fmt = %d\t",inputs_attr[0].fmt);
	printf("type = %d\n",inputs_attr[0].type);
	printf("qnt_type = %d\t",inputs_attr[0].qnt_type);
	printf("fl = %d\t",inputs_attr[0].fl);
	printf("zp = %d\t",inputs_attr[0].zp);
	printf("scale = %f\n",inputs_attr[0].scale);
	printf("intput ---------------end\n");

	// 本身是需要查每个output的，但是so不让查大于2以后的output
	uint32_t queryCnt = io_num.n_output > 2 ? 2 : io_num.n_output;
	for(uint32_t i = 0; i < queryCnt; i++)
	{
		ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[i]), sizeof(rknn_tensor_attr));//获取某个输出节点的属性
		if(ret < 0) {
            cout<<"RKNN_QUERY_OUTPUT_ATTR fail! ret="<<ret<<endl;
		    return;
		}

		printf("output[%d] ---------------start\n",i);
		printf("n_dims = %d\n",outputs_attr[i].n_dims);
		printf("dims array =");
		for(uint32_t j = 0; j < outputs_attr[i].n_dims; j++)
		{
			printf(" %d",outputs_attr[i].dims[j]);
		}
		printf("\n");
		printf("name = %s\t",outputs_attr[i].name);
		printf("n_elems = %d\t",outputs_attr[i].n_elems);
		printf("size = %d\t",outputs_attr[i].size);
		printf("fmt = %d\t",outputs_attr[i].fmt);
		printf("type = %d\n",outputs_attr[i].type);
		printf("qnt_type = %d\t",outputs_attr[i].qnt_type);
		printf("fl = %d\t",outputs_attr[i].fl);
		printf("zp = %d\t",outputs_attr[i].zp);
		printf("scale = %f\n",outputs_attr[i].scale);

		printf("output[%d] ---------------end\n",i);
	}
}

int Detector::input(cv::Mat &image)
{
	// 输入图片保存
	inputImg = image.clone();
	imgPreprocess();
	return 0;
}

std::string Detector::getRknnApiVer()
{
	return apiVersion;
}

std::string Detector::getRknnDrvVer()
{
	return drvVersion;
}