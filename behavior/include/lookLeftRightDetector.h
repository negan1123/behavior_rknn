/*
 * lookLeftRight.h
 *
 *  Created on: Jan 25, 2022
 *      Author: wy
 */

#ifndef INCLUDE_LOOKLEFTRIGHT_H_
#define INCLUDE_LOOKLEFTRIGHT_H_

#include <modules/face.h>
#include <utils/rockx_config_util.h>
#include <rockx.h>
#include <string>

class LookLeftRightDetector
{
public:
	LookLeftRightDetector();
	~LookLeftRightDetector();
	int input(const std::string& img_path);
	int detect();
	void getAngle(float& yaw, float& pitch, float& roll);
private:
	rockx_handle_t face_det_handle;
	rockx_handle_t face_landmark68_handle;
	rockx_image_t input_image;
	rockx_object_array_t face_array;
	rockx_face_angle_t out_angle;
	rockx_face_landmark_t out_landmark68;
};



#endif /* INCLUDE_LOOKLEFTRIGHT_H_ */
