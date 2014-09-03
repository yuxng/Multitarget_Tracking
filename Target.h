/*
 * Target.h
 *
 *  Created on: Aug 24, 2014
 *      Author: yuxiang
 */

#ifndef TARGET_H_
#define TARGET_H_

#include "common.h"

class Target {
public:
	Target(int id, BBOX bbox, TARGET_STATUS status);
	virtual ~Target();

	int get_id();
	BBOX get_bbox();
	TARGET_STATUS get_status();
	float get_velocity_x();
	float get_velocity_y();
	void add_sample_bbox(BBOX bbox);
	float sample_location(float sigma_x, float sigma_y, BBOX &bbox_sample);

private:
	int id_;
	float vx_;	// velocity in x-axis
	float vy_;	// velocity in y-axis
	BBOX bbox_;
	TARGET_STATUS status_;
	std::vector<BBOX> sample_bboxes_;

	cv::RNG rng_;	// random number generator
};

#endif /* TARGET_H_ */
