/*
 * HeadBehaviorDetector.h
 *
 *  Created on: Mar 31, 2022
 *      Author: wy
 */

#ifndef HEADBEHAVIORDETECTOR_H_
#define HEADBEHAVIORDETECTOR_H_

#include <Detector.h>

struct HeadBehavior
{
	bool close_eye {false};
	bool yawn {false};
};

class HeadBehaviorDetector: public Detector
{
public:
	HeadBehaviorDetector();
	~HeadBehaviorDetector();
	int imgPreprocess(void) override;
	int detect(void) override;
	int classifyBehavior(void);
	HeadBehavior getBehavior(void);
private:
	cv::Mat preProcessImage;
	int imageWidth {0};
	int imageHeight {0};
	HeadBehavior behavior;
};



#endif /* HEADBEHAVIORDETECTOR_H_ */
