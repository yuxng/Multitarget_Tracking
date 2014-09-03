/*
 * Tracker.h
 *
 *  Created on: Aug 11, 2014
 *      Author: yuxiang
 */

#ifndef TRACKER_H_
#define TRACKER_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "common.h"
#include "Target.h"

class Tracker {
public:
	Tracker();
	virtual ~Tracker();

	void read_image_paths(const char* filename);
	void read_confidence_paths(const char* filename);
	bool read_confidence_file(const std::string filename, std::vector<BBOX> &bboxes, cv::Mat &confidence);

	bool is_the_end();
	bool is_empty_target();

	void initialize_tracker();
	void process_frame();
	void next_frame();

	float get_velocity_x(int id);
	float get_velocity_y(int id);
	float get_sample_box(int id, BBOX &bbox);

	// sampling related
	void run_rjmcmc_sampling(std::vector<BBOX> bboxes, cv::Mat confidence);
	float* hungarian(std::vector<BBOX> bboxes_target, std::vector<BBOX> bboxes);
	void perturb_box(BBOX &bbox);
	SAMPLE get_initial_sample();
	SAMPLE add_target(SAMPLE sample, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio);
	SAMPLE delete_target(SAMPLE sample, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio);
	SAMPLE stay_target(SAMPLE sample, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio);
	SAMPLE leave_target(SAMPLE sample, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio);
	SAMPLE update_target(SAMPLE sample, cv::Mat confidence, float &acceptance_ratio);

	void compute_motion_prior(SAMPLE sample);
	void update_motion_prior(MOVE_TYPE move);
	float compute_motion_ratio(BBOX bbox, MOVE_TYPE move);

private:
	std::vector<std::string> image_paths_;
	std::vector<std::string>::iterator image_path_iterator_;

	std::vector<std::string> confidence_paths_;
	std::vector<std::string>::iterator confidence_path_iterator_;

	int num_added_;
	int num_stayed_;

	// targets
	int target_id_;
	std::vector<Target> targets_;

	// samples
	std::vector<SAMPLE> samples_;

	PARAMETER parameter_;
	cv::RNG rng_;	// random number generator
};

#endif /* TRACKER_H_ */
