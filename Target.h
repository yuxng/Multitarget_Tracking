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
	friend class Tracker;
public:
	Target();
	virtual ~Target();

	Target apply_motion_model();

private:
	int id_;	// target id
	float cx_;	// x coordinate of center location
	float cy_;  // y coordinate of center location
	float width_;	// 2D width of the target
	float height_;	// 2D height of the target
	float score_;	// confidence score of the target
	float vx_;	// velocity in x-axis
	float vy_;	// velocity in y-axis
	TARGET_STATUS status_;

	std::vector<SAMPLE_INDEX> sample_indexes_;

	float motion_prior_;
	float motion_prior_new_;

	int count_active_;
	int count_lost_;
};

#endif /* TARGET_H_ */
