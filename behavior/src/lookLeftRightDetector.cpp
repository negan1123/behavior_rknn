/*
 * lookLeftRight.cpp
 *
 *  Created on: Jan 25, 2022
 *      Author: wy
 */


#include <lookLeftRightDetector.h>
#include <string.h>

LookLeftRightDetector::LookLeftRightDetector()
{
	rockx_ret_t ret;
	ret = rockx_create(&face_det_handle, ROCKX_MODULE_FACE_DETECTION_V3, nullptr, 0);
	if (ret != ROCKX_RET_SUCCESS) {
		printf("init rockx module ROCKX_MODULE_FACE_DETECTION error %d\n", ret);
	}
	ret = rockx_create(&face_landmark68_handle, ROCKX_MODULE_FACE_LANDMARK_68, nullptr, 0);
	if (ret != ROCKX_RET_SUCCESS) {
		printf("init rockx module ROCKX_MODULE_FACE_LANDMARK_68 error %d\n", ret);
	}
	memset(&face_array, 0, sizeof(rockx_object_array_t));
	memset(&out_angle, 0, sizeof(rockx_face_angle_t));
	memset(&out_landmark68, 0, sizeof(rockx_face_landmark_t));
}

LookLeftRightDetector::~LookLeftRightDetector()
{
	rockx_destroy(face_det_handle);
	rockx_destroy(face_landmark68_handle);
}

int LookLeftRightDetector::input(const std::string& img_path)
{
	rockx_image_read(img_path.data(), &input_image, 1);
}

int LookLeftRightDetector::detect()
{
	rockx_ret_t ret;
	ret = rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
	if (ret != ROCKX_RET_SUCCESS) {
		printf("rockx_face_detect error %d\n", ret);
		rockx_image_release(&input_image);
		return -1;
	}
	ret = rockx_face_landmark(face_landmark68_handle, &input_image, &face_array.object[0].box, &out_landmark68);
	ret = rockx_face_pose(&out_landmark68, &out_angle);
	rockx_image_release(&input_image);
	return 0;
}

void LookLeftRightDetector::getAngle(float& yaw, float& pitch, float& roll)
{
	pitch = out_angle.pitch;
	yaw = out_angle.yaw;
	roll = out_angle.roll;
}

