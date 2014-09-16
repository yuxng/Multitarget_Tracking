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
	bool read_confidence_file(const std::string filename, std::vector<Target> &targets, cv::Mat &confidence);

	bool is_the_end();
	bool is_empty_target();

	void initialize_tracker();
	void process_frame();
	void next_frame();

	float get_sample_target(int id, Target &target);
	float sample_location(Target target, float sigma_x, float sigma_y, Target &target_sample);
	float target_overlap(Target t1, Target t2);
	float target_distance(Target t1, Target t2);

	// sampling related
	void run_rjmcmc_sampling(std::vector<Target> targets, cv::Mat confidence);
	float* hungarian(std::vector<Target> bboxes_target, std::vector<Target> bboxes);
	void perturb_target(Target &target);
	SAMPLE get_initial_sample();
	SAMPLE add_target(SAMPLE sample, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio);
	SAMPLE delete_target(SAMPLE sample, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio);
	SAMPLE stay_target(SAMPLE sample, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio);
	SAMPLE leave_target(SAMPLE sample, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio);
	SAMPLE update_target(SAMPLE sample, std::vector<Target> targets, cv::Mat confidence, float &acceptance_ratio);

	void compute_motion_prior(SAMPLE sample);
	void update_motion_prior(MOVE_TYPE move);
	float compute_motion_ratio(Target target, MOVE_TYPE move);

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
