/*
 * BodyBehaviorDetector.h
 *
 *  Created on: Apr 1, 2022
 *      Author: wy
 */

#ifndef BODYBEHAVIORDETECTOR_H_
#define BODYBEHAVIORDETECTOR_H_

#include <Detector.h>

#define POSITIVE 0
#define DRINK 1
#define PHONE 2
#define SMOKE 3

class BodyBehaviorDetector: public Detector
{
public:
	BodyBehaviorDetector();
	~BodyBehaviorDetector();
	int imgPreprocess(void) override;
	int detect(void) override;
	int classifyBehavior(void);
	int getBehavior(void);
private:
	cv::Mat preProcessImage;
	int imageWidth {0};
	int imageHeight {0};
	int behavior;
};



#endif /* BODYBEHAVIORDETECTOR_H_ */
