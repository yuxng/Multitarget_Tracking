/*
 * Target.cpp
 *
 *  Created on: Aug 24, 2014
 *      Author: yuxiang
 */

#include "Target.h"

Target::Target()
{
	// TODO Auto-generated constructor stub
	id_ = -1;
	cx_ = 0;
	cy_ = 0;
	width_ = 0;
	height_ = 0;
	score_ = 0;
	vx_ = 0;
	vy_ = 0;
	status_ = TARGET_ADDED;
	motion_prior_ = 1;
	motion_prior_new_ = 1;
	count_active_ = 0;
	count_lost_ = 0;
}

Target::~Target() {
	// TODO Auto-generated destructor stub
}

// apply constant velocity model
Target Target::apply_motion_model()
{
	Target target;

	target.id_ = id_;
	target.cx_ = cx_ + vx_;
	target.cy_ = cy_ + vy_;
	target.width_ = width_;
	target.height_ = height_;
	target.score_ = score_;
	target.vx_ = vx_;
	target.vy_ = vy_;
	target.status_ = status_;
	target.count_active_ = count_active_;
	target.count_lost_ = count_lost_;

	return target;
}
