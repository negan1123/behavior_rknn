#include <BlockCameraDetector.h>

BlockCameraDetector::BlockCameraDetector()
{
    preProcessImageW = 640;
	preProcessImageH = 360;
    unblock_num_thres = BehaviorConf::Instance()->getUnblockNum();
    block_part_num_thres = BehaviorConf::Instance()->getBlockPartNum();
}

BlockCameraDetector::~BlockCameraDetector()
{

}

int BlockCameraDetector::imgPreprocess()
{
    cv::Mat resimg;	// 压缩后的图像
    cv::Mat grayImg;	// 压缩后的图像
    // 压缩图片
	rescale_ratio = std::min((float)preProcessImageW/inputImg.cols, (float)preProcessImageH/inputImg.rows);	// 获取压缩比例
    cv::resize(inputImg, resimg, cv::Size(), rescale_ratio, rescale_ratio);	// 压缩图片
    cv::cvtColor(resimg, grayImg, cv::COLOR_BGR2GRAY);   // 转换为灰度图像
    // Canny边缘处理图像
    cv::Canny(grayImg, preProcessImage, 0, 255, 3);
    // cv::imwrite("../img_output/canny.jpg",preProcessImage);
    return 0;
}

int BlockCameraDetector::isBlockCamera(cv::Mat img, int unblock_num_thres)
{   
    int rows = img.rows;
    int cols = img.cols;
    int unblock_num = 0;
    int block_flag = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            uchar piexs_value = img.at<uchar>(i, j);   // 获取当前像素点像素值
            //  判断当前像素点是否为遮挡像素点
            if (piexs_value != 0)
            {
                unblock_num++;
            }
        }
    }
    // cout<<unblock_num<<endl;
    if (unblock_num < unblock_num_thres)
    {
        block_flag = 1;
    }
    return block_flag;
}

int BlockCameraDetector::detect()
{   
    block_num = 0;  // 初始化遮挡部分数量
    int rows,cols;
    rows = preProcessImage.rows;
    cols = preProcessImage.cols;

    cv::Rect rec;

    // 检测左上遮挡
    rec = cv::Rect(0,0,cols/2,rows/2);  // xywh
    cv::Mat img_lefttop = preProcessImage(rec);
    // cv::imwrite("../img_output/img_lefttop.jpg",img_lefttop);
    block_num += isBlockCamera(img_lefttop,unblock_num_thres);

    // 检测左下遮挡
    rec = cv::Rect(0,rows/2,cols/2,rows/2);  // xywh
    cv::Mat img_leftbottom = preProcessImage(rec);
    // cv::imwrite("../img_output/img_leftbottom.jpg",img_leftbottom);
    block_num += isBlockCamera(img_leftbottom,unblock_num_thres);

    // 检测右上遮挡
    rec = cv::Rect(cols/2,0,cols/2,rows/2);  // xywh
    cv::Mat img_righttop = preProcessImage(rec);
    // cv::imwrite("../img_output/img_righttop.jpg",img_righttop);
    block_num += isBlockCamera(img_righttop,unblock_num_thres);

    // 检测右下遮挡
    rec = cv::Rect(cols/2,rows/2,cols/2,rows/2);  // xywh
    cv::Mat img_rightbottom = preProcessImage(rec);
    // cv::imwrite("../img_output/img_rightbottom.jpg",img_rightbottom);
    block_num += isBlockCamera(img_rightbottom,unblock_num_thres);

    return 0;

}

int BlockCameraDetector::getBehavior()
{
    // 判断是否遮挡
    if (block_num > block_part_num_thres)
    {
        return 1;
    }
    return 0;
}