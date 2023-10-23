#include <RockxDetector.h>

#define test 0

RockxDetector::RockxDetector()
{   
    closeEye_Thres = BehaviorConf::Instance()->getCloseEyeThreshold();
    yawn_Thres = BehaviorConf::Instance()->getYawnThreshold();
    init();
}

RockxDetector::~RockxDetector()
{
    destoryDetector();
}

int RockxDetector::init()
{
    rockx_ret_t ret;    // RockX函数的返回值
    ret = rockx_create(&face_det_handle, ROCKX_MODULE_FACE_DETECTION_V3, nullptr, 0);       // 初始化人脸检测模型
    if (ret != ROCKX_RET_SUCCESS) 
    {
		spdlog::error("init rockx module ROCKX_MODULE_FACE_DETECTION error {}",ret);
        return ret;
	}

    ret = rockx_create(&face_landmark68_handle, ROCKX_MODULE_FACE_LANDMARK_68, nullptr, 0); // 初始化人脸标志点模型
    if (ret != ROCKX_RET_SUCCESS) 
    {
		spdlog::error("init rockx module ROCKX_MODULE_FACE_LANDMARK_68 error {}",ret);
        return ret;
	}

    memset(&face_array, 0, sizeof(rockx_object_array_t));       // 申请face_array [输出]人脸检测结果数组
    memset(&out_landmark68, 0, sizeof(rockx_face_landmark_t));  // 申请out_landmark68	[输入]人脸标志性结果
    memset(&out_angle, 0, sizeof(rockx_face_angle_t));          // 申请out_angle	[输出]脸部角度

    return 0;
}

int RockxDetector::destoryDetector()
{   
    if (face_det_handle != NULL)
        rockx_destroy(face_det_handle);
    if (face_landmark68_handle != NULL)
	    rockx_destroy(face_landmark68_handle);
    return 0;
}

int RockxDetector::input(const std::string& img_path)
{   
	rockx_ret_t ret;
	ret = rockx_image_read(img_path.data(), &input_image, 1);// 构造图像，将param1 本地图像 读取到param2 rockx_image_t
    if (ret != ROCKX_RET_SUCCESS)
    {
        cout<<"rockx_image_read error"<<ret<<endl;
        input_image.size = 0;
        return ret;
    }
	return 0;
}

int RockxDetector::faceDetect()
{   
    rockx_ret_t ret;
    // 人脸检测模型推理
	ret = rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
	if (ret != ROCKX_RET_SUCCESS)
    {
		printf("rockx_face_detect error %d\n", ret);
		rockx_image_release(&input_image);
		return -1;
	}

#if test
    cout<<"--> rockx box:"<<endl;
    cout<<face_array.object[0].box.left<<","<<face_array.object[0].box.top<<","<<face_array.object[0].box.right<<","<<face_array.object[0].box.bottom<<endl;
#endif

    // 人脸标志点模型推理
    ret = rockx_face_landmark(face_landmark68_handle, &input_image, &face_array.object[0].box, &out_landmark68);
	if(ret != 0){
		printf("rockx_face_landmark error %d\n",ret);
        rockx_image_release(&input_image);
		return -1;
	}

    // 人脸欧拉角检测
    ret = rockx_face_pose(&out_landmark68, &out_angle);
	if(ret != 0){
		printf("rockx_face_pose error %d\n",ret);
        rockx_image_release(&input_image);
		return -1;
	}

    rockx_image_release(&input_image);
    return 0;
}

void RockxDetector::getResults(faceAngle& angle, vector<landmark>& landmarks, faceBox& box)
{   
    // 返回人脸框
    rockx_rect_t faceBox = face_array.object[0].box;
    box.left = faceBox.left;
    box.top = faceBox.top;
    box.right = faceBox.right;
    box.bottom = faceBox.bottom;

    // 返回关键点
    rockx_point_t *faceLandmarks = &out_landmark68.landmarks[0];
    for (int i = 0; i < 128; i++)
    {
        landmark lm;
        lm.x = faceLandmarks[i].x;
        lm.y = faceLandmarks[i].y;
        landmarks.push_back(lm);
    }

    // 返回欧拉角
	angle.pitch = out_angle.pitch;
	angle.yaw = out_angle.yaw;
	angle.roll = out_angle.roll;
}

float Point_distance(landmark a, landmark b)
{
    float distance = sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    return distance;
}

float value(float dis1, float dis2, float dis3)
{
    float value = (dis1 + dis2) / (2 * dis3);
    return value;
}

int RockxDetector::isCloseEye(vector<landmark> marks)
{
    float dis_2_6;
    float dis_3_5;
    float dis_1_4;
    
    landmark L1 = marks[36];
    landmark L2 = marks[37];
    landmark L3 = marks[38];
    landmark L4 = marks[39];
    landmark L5 = marks[40];
    landmark L6 = marks[41];

    landmark R1 = marks[42];
    landmark R2 = marks[43];
    landmark R3 = marks[44];
    landmark R4 = marks[45];
    landmark R5 = marks[46];
    landmark R6 = marks[47];

    // 计算左眼是否闭上
    dis_2_6 = Point_distance(L2, L6);
    dis_3_5 = Point_distance(L3, L5);
    dis_1_4 = Point_distance(L1, L4);
    float L_EAR = value(dis_2_6, dis_3_5, dis_1_4);

    // 计算右眼是否闭上
    dis_2_6 = Point_distance(R2, R6);
    dis_3_5 = Point_distance(R3, R5);
    dis_1_4 = Point_distance(R1, R4);
    float R_EAR = value(dis_2_6, dis_3_5, dis_1_4);

    float EAR = (R_EAR + L_EAR) / 2;

#if test
    cout<<"左眼Ear："<<L_EAR<<endl;
    cout<<"右眼Ear："<<R_EAR<<endl;
    cout<<"EAR："<<EAR<<endl;
#endif

    if (EAR < closeEye_Thres)
    {
        return 1;
    }

    return 0;
}

int RockxDetector::isYawn(vector<landmark> marks)
{
    float dis_2_6;
    float dis_3_5;
    float dis_1_4;
    
    landmark M1 = marks[60];
    landmark M2 = marks[61];
    landmark M3 = marks[63];
    landmark M4 = marks[64];
    landmark M5 = marks[65];
    landmark M6 = marks[67];

    // 计算左眼是否闭上
    dis_2_6 = Point_distance(M2, M6);
    dis_3_5 = Point_distance(M3, M5);
    dis_1_4 = Point_distance(M1, M4);
    float MAR = value(dis_2_6, dis_3_5, dis_1_4);
#if test
    cout<<"MAR:"<<MAR<<endl;
#endif

    if (MAR > yawn_Thres)
    {
        return 1;
    }

    return 0;
}