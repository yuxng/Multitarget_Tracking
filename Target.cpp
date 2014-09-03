/*
 * Target.cpp
 *
 *  Created on: Aug 24, 2014
 *      Author: yuxiang
 */

#include "Target.h"

Target::Target(int id, BBOX bbox, TARGET_STATUS status) {
	// TODO Auto-generated constructor stub
	id_ = id;
	bbox_ = bbox;
	status_ = status;
	vx_ = 0;
	vy_ = 0;
}

Target::~Target() {
	// TODO Auto-generated destructor stub
}

int Target::get_id()
{
	return id_;
}

BBOX Target::get_bbox()
{
	return bbox_;
}

TARGET_STATUS Target::get_status()
{
	return status_;
}

float Target::get_velocity_x()
{
	return vx_;
}

float Target::get_velocity_y()
{
	return vy_;
}


void Target::add_sample_bbox(BBOX bbox)
{
	sample_bboxes_.push_back(bbox);
}


// sample a new location of the target
float Target::sample_location(float sigma_x, float sigma_y, BBOX &bbox_sample)
{
	float prob = 0, mx = 0, my = 0, mw = 0, mh = 0, score = 0;
	std::size_t num = sample_bboxes_.size();

	for(std::size_t i = 0; i < num; i++)
	{
		BBOX bbox = sample_bboxes_[i];

		// apply the motion model
		float cx = (bbox.x1 + bbox.x2) / 2 + vx_;	// constant velocity motion
		float cy = (bbox.y1 + bbox.y2) / 2 + vy_;
		float w = bbox.x2 - bbox.x1;
		float h = bbox.y2 - bbox.y1;

		float rx = rng_.gaussian(sigma_x * w);
		float ry = rng_.gaussian(sigma_y * h);

		prob += exp(log_gaussian_prob(rx, 0, sigma_x * w) + log_gaussian_prob(ry, 0, sigma_y * h));

		mx += cx + rx;
		my += cy + ry;
		mw += w;
		mh += h;
		score += bbox.score;
	}

	mx /= num;
	my /= num;
	mw /= num;
	mh /= num;
	score /= num;

	// build the sampled box
	bbox_sample.id = id_;
	bbox_sample.x1 = mx - mw/2;
	bbox_sample.x2 = mx + mw/2;
	bbox_sample.y1 = my - mh/2;
	bbox_sample.y2 = my + mh/2;
	bbox_sample.status = BOX_TRACKED;
	bbox_sample.score = score;

	return prob;
}
