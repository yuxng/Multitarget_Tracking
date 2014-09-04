/*
 * Tracker.cpp
 *
 *  Created on: Aug 11, 2014
 *      Author: yuxiang
 */

#include "Tracker.h"

Tracker::Tracker() {
	// TODO Auto-generated constructor stub

}

Tracker::~Tracker() {
	// TODO Auto-generated destructor stub
}

void Tracker::read_image_paths(const char *filename)
{
	std::string line;
	std::ifstream file(filename);

	if(file.is_open())
	{
		std::cout << "reading file " << filename << std::endl;
		while(std::getline(file, line))
			image_paths_.push_back(line);
		file.close();
		std::cout << "read file " << filename << " finished" << std::endl;
	}
	else
		std::cout << "Unable to open file " << filename << std::endl;
}


void Tracker::read_confidence_paths(const char *filename)
{
	std::string line;
	std::ifstream file(filename);

	if(file.is_open())
	{
		std::cout << "reading file " << filename << std::endl;
		while(std::getline(file, line))
			confidence_paths_.push_back(line);
		file.close();
		std::cout << "read file " << filename << " finished" << std::endl;
	}
	else
		std::cout << "Unable to open file " << filename << std::endl;

	assert(confidence_paths_.size() == image_paths_.size());
}


// initialize the tracker
void Tracker::initialize_tracker()
{
	target_id_ = 0;
	image_path_iterator_ = image_paths_.begin();
	confidence_path_iterator_ = confidence_paths_.begin();

	// set parameters
	parameter_.num_sample = 3000;

	parameter_.min_overlap = 0.25;

	parameter_.prob_moves[MOVE_ADD] = 0.1;
	parameter_.prob_moves[MOVE_DELETE] = 0.1;
	parameter_.prob_moves[MOVE_STAY] = 0.2;
	parameter_.prob_moves[MOVE_LEAVE] = 0.2;
	parameter_.prob_moves[MOVE_UPDATE] = 0.4;

	parameter_.sigma_det_x = 0.1;
	parameter_.sigma_det_y = 0.1;
}


// check if the end of the video
bool Tracker::is_the_end()
{
	return image_path_iterator_ == image_paths_.end();
}


// process the current frame
void Tracker::process_frame()
{
	std::cout << *image_path_iterator_ << std::endl;
	std::cout << *confidence_path_iterator_ << std::endl;

	// read image
	cv::Mat image;
	image = cv::imread(*image_path_iterator_, CV_LOAD_IMAGE_COLOR);   // Read the file

	if(! image.data )  // Check for invalid input
	{
		std::cout <<  "Could not open or find the image" << std::endl ;
	}

	// read detection bounding boxes and confidence map
	std::vector<BBOX> bboxes;
	cv::Mat confidence;
	bool success = read_confidence_file(*confidence_path_iterator_, bboxes, confidence);
	if(success)
		std::cout << bboxes.size() << " boxes read" << std::endl;

	if(is_empty_target() == true)
	{
		// initialize the targets by detections
		for(std::size_t i = 0; i < bboxes.size(); i++)
		{
			bboxes[i].id = target_id_;
			bboxes[i].status = BOX_TRACKED;
			Target target(target_id_, bboxes[i], ACTIVE);
			target.sample_bboxes_.push_back(bboxes[i]);
			targets_.push_back(target);
			target_id_++;
		}

		// initialize the sample
		if(bboxes.size() > 0)
		{
			SAMPLE sample;
			sample.bboxes = bboxes;
			samples_.push_back(sample);
		}
	}
	else  // RJMCMC particle filtering
	{
		run_rjmcmc_sampling(bboxes, confidence);
	}

	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		BBOX bbox = targets_[i].bbox_;
		cv::rectangle(image, cv::Point(bbox.x1, bbox.y1), cv::Point(bbox.x2, bbox.y2), cv::Scalar(0, 255, 0), 2);
	}

	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);  // Create a window for display.
	cv::imshow("Display window", image);  // Show our image inside it.

	// clean up
	if(success)
	{
		bboxes.clear();
		confidence.release();
	}
}


// proceed to the next frame
void Tracker::next_frame()
{
	image_path_iterator_++;
	confidence_path_iterator_++;
}


// read confidence file
bool Tracker::read_confidence_file(const std::string filename, std::vector<BBOX> &bboxes, cv::Mat &confidence)
{
	FILE *fp;
	size_t nread;

	std::cout << "read " << filename << std::endl;
	fp = fopen(filename.c_str(), "r");
	if(fp == NULL)
	{
		std::cout << "Cannot read detection confidence file " << filename << std::endl;
		return false;
	}

	char header[4];
	nread = fread(header, sizeof(char), 4, fp);
	assert(nread == 4);

	if(header[0] == 'C' && header[1] == 'O' && header[2] == 'N' && header[3] == '4')
	{
		std::cout << "confidence version 4" << std::endl;
	}
	else
	{
		std::cout << "ERROR : invalid header format!" << std::endl;
		fclose(fp);
		return false;
	}

	unsigned int nums;
	float det[6];
	BBOX bbox;
	assert(sizeof(unsigned int) == 4);

	nread = fread(&nums, sizeof(unsigned int), 1, fp);
	assert(nread == 1);
	for(size_t i = 0; i < nums; i++)
	{
		nread = fread(det, sizeof(float), 5, fp);
		assert(nread == 5);

		bbox.id = -1;
		bbox.x1 = det[0];
		bbox.y1 = det[1];
		bbox.x2 = det[0] + det[2];
		bbox.y2 = det[1] + det[3];
		bbox.score = exp(det[4]);
		bbox.status = BOX_ADDED;

		bboxes.push_back(bbox);
	}

	// read confidence map
	float *data;
	int map_size[2];

	nread = fread(map_size, sizeof(int), 2, fp); // my mistake....should have used integer....
	assert(nread == 2);

	data = new float [ map_size[0] * map_size[1] ];
	nread = fread(data, sizeof(float), map_size[0] * map_size[1], fp);

	confidence = cv::Mat(map_size[0], map_size[1], CV_32F);
	for(int row = 0; row < map_size[0]; row++)
	{
		for(int col = 0; col < map_size[1]; col++)
		{
			confidence.at<float>(row, col) = data[ row + col * map_size[0] ];
		}
	}
	delete data;

	fclose(fp);

	return true;
}


// check if the target set is empty or not
bool Tracker::is_empty_target()
{
	bool is_empty = true;

	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(targets_[i].status_ != INACTIVE)
		{
			is_empty = false;
			break;
		}
	}

	return is_empty;
}


// compute initial sample
SAMPLE Tracker::get_initial_sample()
{
	SAMPLE sample;
	BBOX bbox;
	float cx, cy, vx, vy, w, h;

	// apply motion model to each target
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		bbox = targets_[i].bbox_;
		vx = targets_[i].vx_;
		vy = targets_[i].vy_;
		cx = (bbox.x1 + bbox.x2) / 2 + vx;	// constant velocity motion
		cy = (bbox.y1 + bbox.y2) / 2 + vy;
		w = bbox.x2 - bbox.x1;
		h = bbox.y2 - bbox.y1;

		bbox.x1 = cx - w/2;
		bbox.x2 = cx + w/2;
		bbox.y1 = cy - h/2;
		bbox.y2 = cy + h/2;

		sample.bboxes.push_back(bbox);
	}

	return sample;
}


// Reversible jump MCMC sampling
void Tracker::run_rjmcmc_sampling(std::vector<BBOX> bboxes, cv::Mat confidence)
{
	SAMPLE sample_init = get_initial_sample();

	// data association with Hungarian algorithm
	float *assignment = hungarian(sample_init.bboxes, bboxes);

	// assign id to the boxes
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		int index = assignment[i];
		if(index != -1)
		{
			bboxes[index].id = targets_[i].id_;
			bboxes[index].status = BOX_TRACKED;
		}
	}

	// collect the boxes to be added
	std::vector<BBOX> bboxes_add;
	for(std::size_t i = 0; i < bboxes.size(); i++)
	{
		if(bboxes[i].status == BOX_ADDED)
		{
			bboxes[i].id = target_id_++;
			bboxes_add.push_back(bboxes[i]);
		}
	}
	num_added_ = bboxes_add.size();		// total number of objects can be added
	num_stayed_ = targets_.size();
	std::cout << num_added_ << " " << num_stayed_ << std::endl;

	// generate samples
	SAMPLE sample_prev = sample_init;
	SAMPLE sample;
	std::vector<SAMPLE> samples;
	std::vector<BBOX> bboxes_stay;
	compute_motion_prior(sample_init);
	for(int i = 0; i < parameter_.num_sample; i++)
	{
		std::cout << "sample " << i << std::endl;
		// determine the move type
		MOVE_TYPE move;
		std::size_t bbox_id = 0;
		double val = rng_.uniform((double)0, (double)1);
		float acceptance_ratio = 0;

		if((val -= parameter_.prob_moves[MOVE_ADD]) < 0)
		{
			// add an object
			std::cout << "Add move" << std::endl;
			move = MOVE_ADD;
			sample = add_target(sample_prev, bboxes_add, bbox_id, acceptance_ratio);
		}
		else if((val -= parameter_.prob_moves[MOVE_DELETE]) < 0)
		{
			// delete an object
			std::cout << "Delete move" << std::endl;
			move = MOVE_DELETE;
			sample = delete_target(sample_prev, bboxes_add, bbox_id, acceptance_ratio);
		}
		else if((val -= parameter_.prob_moves[MOVE_STAY]) < 0)
		{
			// stay move
			std::cout << "Stay move" << std::endl;
			move = MOVE_STAY;
			sample = stay_target(sample_prev, bboxes_stay, bbox_id, acceptance_ratio);
		}
		else if((val -= parameter_.prob_moves[MOVE_LEAVE]) < 0)
		{
			// leave move
			std::cout << "Leave move" << std::endl;
			move = MOVE_LEAVE;
			sample = leave_target(sample_prev, bboxes_stay, bbox_id, acceptance_ratio);
		}
		else if((val -= parameter_.prob_moves[MOVE_UPDATE]) < 0)
		{
			// update move
			std::cout << "Update move" << std::endl;
			move = MOVE_UPDATE;
			sample = update_target(sample_prev, confidence, acceptance_ratio);
		}
		else
		{
			assert(0);
		}

		// determine to accept the new sample or not
		val = rng_.uniform((double)0, (double)1);
		if(acceptance_ratio > val)	// accept the sample
		{
			// update the bboxes
			switch(move)
			{
			case MOVE_ADD:
				bboxes_add.erase(bboxes_add.begin() + bbox_id);
				break;
			case MOVE_DELETE:
				bboxes_add.push_back(sample_prev.bboxes[bbox_id]);
				break;
			case MOVE_STAY:
				bboxes_stay.erase(bboxes_stay.begin() + bbox_id);
				break;
			case MOVE_LEAVE:
				bboxes_stay.push_back(sample_prev.bboxes[bbox_id]);
				break;
			}

			// store and update the sample
			samples.push_back(sample);
			sample_prev = sample;

			// update the motion prior
			update_motion_prior(move);
			std::cout << "accept " << acceptance_ratio << std::endl;
		}
		std::cout << "number of targets " << sample_prev.bboxes.size() << std::endl;
	}
	samples_.clear();
	samples_ = samples;

	// extend the target set
	int num_target = targets_.size();
	for(int i = num_target; i < target_id_; i++)
	{
		BBOX *bbox = new BBOX[1];
		bbox->id = i;
		bbox->status = BOX_TRACKED;
		Target target(i, *bbox, ACTIVE);
		targets_.push_back(target);
	}
	std::cout << targets_.size() << " targets after sampling" << std::endl;

	// clean up the target samples
	for(std::size_t i = 0; i < targets_.size(); i++)
		targets_[i].sample_bboxes_.clear();

	std::cout << samples_.size() << " samples after sampling" << std::endl;
	// extract the samples for the  targets
	for(std::size_t i = 0; i < samples_.size(); i++)
	{
		for(std::size_t j = 0; j < samples_[i].bboxes.size(); j++)
		{
			BBOX bbox = samples_[i].bboxes[j];
			targets_[bbox.id].sample_bboxes_.push_back(bbox);
		}
	}

	// compute the mean sample location for each target
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		float cx = 0, cy = 0, w = 0, h = 0, score = 0;
		std::size_t num = targets_[i].sample_bboxes_.size();
		for(std::size_t j = 0; j < num; j++)
		{
			BBOX bbox = targets_[i].sample_bboxes_[j];
			cx += (bbox.x1 + bbox.x2) / 2;
			cy += (bbox.y1 + bbox.y2) / 2;
			w += bbox.x2 - bbox.x1;
			h += bbox.y2 - bbox.y1;
			score += bbox.score;
		}
		if(num > 0)
		{
			cx /= num;
			cy /= num;
			w /= num;
			h /= num;
		}

		targets_[i].bbox_.x1 = cx - w/2;
		targets_[i].bbox_.x2 = cx + w/2;
		targets_[i].bbox_.y1 = cy - h/2;
		targets_[i].bbox_.y2 = cy + h/2;
	}
}

// Hungarian algorithm for data association
float* Tracker::hungarian(std::vector<BBOX> bboxes_target, std::vector<BBOX> bboxes)
{
	float overlap, dis, cost;
	float *assign = new float [bboxes_target.size()];
	float *distMat = new float [bboxes_target.size() * bboxes.size()];

	for(std::size_t i = 0; i < bboxes_target.size(); i++)
	{
		for(std::size_t j = 0; j < bboxes.size(); j++)
		{
			overlap = box_overlap(bboxes_target[i], bboxes[j]);
			if(overlap < parameter_.min_overlap)
				dis = PLUS_INFINITY;
			else
				dis = 1 - overlap;
			distMat[i + bboxes_target.size() * j] = dis;
		}
	}
	// solve assignment problem
	assignmentoptimal(assign, &cost, distMat, bboxes_target.size(), bboxes.size(), PLUS_INFINITY);

	delete distMat;
	return assign;
}


// add move
SAMPLE Tracker::add_target(SAMPLE sample_prev, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio)
{
	if(bboxes.size() == 0)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to add
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)bboxes.size()));
	SAMPLE sample = sample_prev;
	BBOX bbox = bboxes[idx];
	sample.bboxes.push_back(bbox);

	// compute acceptance ratio
	float num_add = bboxes.size();
	float num_delete = num_added_ - num_add + 1;
	acceptance_ratio = bbox.score * parameter_.prob_moves[MOVE_DELETE] * num_add / num_delete;

	// return the object id added
	bbox_id = idx;

	return sample;
}


// delete move
SAMPLE Tracker::delete_target(SAMPLE sample_prev, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio)
{
	std::cout << bboxes.size() << " " << num_added_ << std::endl;
	if(bboxes.size() == (std::size_t)num_added_)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to delete
	int num_delete = num_added_ - bboxes.size();
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)num_delete));

	SAMPLE sample = sample_prev;
	std::size_t index = 0;
	for(std::size_t i = 0; i < sample.bboxes.size(); i++)
	{
		if(sample.bboxes[i].status == BOX_ADDED)
		{
			if(index == idx)
				break;
			index++;
		}
	}
	// remove the target
	BBOX bbox = sample.bboxes[index];
	sample.bboxes.erase(sample.bboxes.begin() + index);

	// return the object id to be deleted
	bbox_id = index;

	// compute acceptance ratio
	float num_add = bboxes.size() + 1;
	acceptance_ratio = (1 / bbox.score) * (1 / parameter_.prob_moves[MOVE_DELETE]) * (float)num_delete / num_add;

	return sample;
}


// stay move
SAMPLE Tracker::stay_target(SAMPLE sample_prev, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio)
{
	if(bboxes.size() == 0)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to stay
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)bboxes.size()));
	BBOX bbox = bboxes[idx];
	float prob = get_sample_box(bbox.id, bbox);

	// add the target to sample
	SAMPLE sample = sample_prev;
	sample.bboxes.push_back(bbox);

	// compute acceptance ratio
	float motion_ratio = compute_motion_ratio(bbox, MOVE_STAY);
	float move_ratio = parameter_.prob_moves[MOVE_LEAVE] / parameter_.prob_moves[MOVE_STAY];
	float num_stay = bboxes.size();
	float num_leave = num_stayed_ - num_stay + 1;
	acceptance_ratio = bbox.score * motion_ratio * move_ratio * num_stay / (num_leave * prob);

	// return the object id stayed
	bbox_id = idx;

	return sample;
}


// leave move
SAMPLE Tracker::leave_target(SAMPLE sample_prev, std::vector<BBOX> bboxes, std::size_t &bbox_id, float &acceptance_ratio)
{
	if(bboxes.size() == (std::size_t)num_stayed_)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to delete
	int num_leave = num_stayed_ - bboxes.size();
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)num_leave));
	std::cout << "num_leave " << num_leave << " idx " << idx << std::endl;

	SAMPLE sample = sample_prev;
	std::size_t index = 0;
	for(std::size_t i = 0; i < sample.bboxes.size(); i++)
	{
		if(sample.bboxes[i].status == BOX_TRACKED)
		{
			if(index == idx)
				break;
			index++;
		}
	}
	// remove the target
	BBOX bbox = sample.bboxes[index];
	std::cout << "leave target " << index << " score " << bbox.score << " size of sample boxes " << sample.bboxes.size() << std::endl;
	sample.bboxes.erase(sample.bboxes.begin() + index);

	// return the object id left
	bbox_id = index;

	// compute acceptance ratio
	float prob = get_sample_box(bbox.id, bbox);
	float motion_ratio = compute_motion_ratio(bbox, MOVE_LEAVE);
	float move_ratio = parameter_.prob_moves[MOVE_STAY] / parameter_.prob_moves[MOVE_LEAVE];
	float num_stay = bboxes.size() + 1;
	acceptance_ratio = (1 / bbox.score) * motion_ratio * move_ratio * num_leave * prob / num_stay;

	return sample;
}


// update move
SAMPLE Tracker::update_target(SAMPLE sample_prev, cv::Mat confidence, float &acceptance_ratio)
{
	if (sample_prev.bboxes.size() == 0)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to update
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)sample_prev.bboxes.size()));
	BBOX bbox = sample_prev.bboxes[idx];
	perturb_box(bbox);

	// get the confidence score
    // down size by factor 4
	int x = floor((bbox.x1 + bbox.x2) / 2 / 4);
	int y = floor((bbox.y1 + bbox.y2) / 2 / 4);
	// check whether point is in image
	if((x < 0) || (y < 0) || (x > confidence.cols) || (y > confidence.rows))
    {
		acceptance_ratio = -1;
		return sample_prev;
    }
	else
	{
		bbox.score = exp(confidence.at<float>(y, x));

		// backup the motion prior
		float *motion_prior = new float[samples_.size()];
		for(std::size_t i = 0; i < samples_.size(); i++)
			motion_prior[i] = samples_[i].motion_prior;

		float motion_ratio_leave = compute_motion_ratio(sample_prev.bboxes[idx], MOVE_LEAVE);
		update_motion_prior(MOVE_LEAVE);
		float motion_ratio_stay = compute_motion_ratio(bbox, MOVE_STAY);

		// restore the origin motion prior
		for(std::size_t i = 0; i < samples_.size(); i++)
			samples_[i].motion_prior = motion_prior[i];
		delete motion_prior;

		acceptance_ratio = (bbox.score / sample_prev.bboxes[idx].score) * motion_ratio_leave * motion_ratio_stay;

		SAMPLE sample = sample_prev;
		sample.bboxes[idx] = bbox;

		return sample;
	}
}


// add Gaussian noise to the bounding box center
void Tracker::perturb_box(BBOX &bbox)
{
	float width = bbox.x2 - bbox.x1;
	float height = bbox.y2 - bbox.y1;
	float cx = (bbox.x1 + bbox.x2) / 2;
	float cy = (bbox.y1 + bbox.y2) / 2;

	// perturb the bounding box center
	cx += rng_.gaussian(parameter_.sigma_det_x * width);
	cy += rng_.gaussian(parameter_.sigma_det_y * height);

	// assign the box
	bbox.x1 = cx - width / 2;
	bbox.x2 = cx + width / 2;
	bbox.y1 = cy - height / 2;
	bbox.y2 = cy + height / 2;
}


void Tracker::compute_motion_prior(SAMPLE sample)
{
	std::vector<BBOX> bboxes = sample.bboxes;

	std::cout << samples_.size() << " samples" << std::endl;
	for(std::size_t i = 0; i < samples_.size(); i++)
	{
		float prior = 1;
		int count = 0;
		std::vector<BBOX> bboxes_prev = samples_[i].bboxes;

		for(std::size_t j = 0; j < bboxes.size(); j++)
		{
			for(std::size_t k = 0; k < bboxes_prev.size(); k++)
			{
				if(bboxes[j].id == bboxes_prev[k].id)
				{
					// matched target
					count++;
					int id = bboxes_prev[k].id;
					float vx = get_velocity_x(id);
					float vy = get_velocity_y(id);
					float w = bboxes_prev[k].x2 - bboxes_prev[k].x1;
					float h = bboxes_prev[k].y2 - bboxes_prev[k].y1;
					float cx = (bboxes_prev[k].x1 + bboxes_prev[k].x2) / 2 + vx;	// constant velocity motion
					float cy = (bboxes_prev[k].y1 + bboxes_prev[k].y2) / 2 + vy;

					float log_motion = log_gaussian_prob((bboxes[j].x1 + bboxes[j].x2) / 2, cx, parameter_.sigma_det_x * w)
							+ log_gaussian_prob((bboxes[j].y1 + bboxes[j].y2) / 2, cy, parameter_.sigma_det_y * h);

					prior *= exp(log_motion);
				}
			}
		}

		// handle unmatched targets
		prior *= pow(parameter_.prob_moves[MOVE_LEAVE], bboxes_prev.size() - count) *
				pow(parameter_.prob_moves[MOVE_ADD], bboxes.size() - count);

		// save the motion prior
		samples_[i].motion_prior = prior;
	}
}


// compute the motion prior ratio for stay move
float Tracker::compute_motion_ratio(BBOX bbox, MOVE_TYPE move)
{
	float prior_sum = 0;
	float prior_new_sum = 0;

	for(std::size_t i = 0; i < samples_.size(); i++)
	{
		int flag = 0;
		float prior = 1;
		float motion_prior = samples_[i].motion_prior;
		float motion_prior_new;
		std::vector<BBOX> bboxes_prev = samples_[i].bboxes;

		for(std::size_t k = 0; k < bboxes_prev.size(); k++)
		{
			if(bbox.id == bboxes_prev[k].id)
			{
				// matched target
				flag = 1;
				int id = bboxes_prev[k].id;
				float vx = get_velocity_x(id);
				float vy = get_velocity_y(id);
				float w = bboxes_prev[k].x2 - bboxes_prev[k].x1;
				float h = bboxes_prev[k].y2 - bboxes_prev[k].y1;
				float cx = (bboxes_prev[k].x1 + bboxes_prev[k].x2) / 2 + vx;	// constant velocity motion
				float cy = (bboxes_prev[k].y1 + bboxes_prev[k].y2) / 2 + vy;

				float log_motion = log_gaussian_prob((bbox.x1 + bbox.x2) / 2, cx, parameter_.sigma_det_x * w)
						+ log_gaussian_prob((bbox.y1 + bbox.y2) / 2, cy, parameter_.sigma_det_y * h);

				prior = exp(log_motion);
				break;
			}
		}

		if(move == MOVE_STAY)
		{
			if(flag)
				motion_prior_new = motion_prior / parameter_.prob_moves[MOVE_LEAVE] * prior;
			else
				motion_prior_new = motion_prior * parameter_.prob_moves[MOVE_ADD];
		}
		else if(move == MOVE_LEAVE)
		{
			if(flag)
				motion_prior_new = motion_prior / prior * parameter_.prob_moves[MOVE_LEAVE];
			else
				motion_prior_new = motion_prior / parameter_.prob_moves[MOVE_ADD];
		}

		prior_sum += motion_prior;
		prior_new_sum += motion_prior_new;

		// save the motion prior
		samples_[i].motion_prior_new = motion_prior_new;
	}

	return prior_new_sum / prior_sum;
}


// update the motion prior
void Tracker::update_motion_prior(MOVE_TYPE move)
{
	switch(move)
	{
	case MOVE_ADD:
		for(std::size_t i = 0; i < samples_.size(); i++)
			samples_[i].motion_prior *= parameter_.prob_moves[MOVE_ADD];
		break;
	case MOVE_DELETE:
		for(std::size_t i = 0; i < samples_.size(); i++)
			samples_[i].motion_prior /= parameter_.prob_moves[MOVE_ADD];
		break;
	case MOVE_STAY:
	case MOVE_LEAVE:
	case MOVE_UPDATE:
		for(std::size_t i = 0; i < samples_.size(); i++)
			samples_[i].motion_prior = samples_[i].motion_prior_new;
	}
}


// get the x velocity of target id
float Tracker::get_velocity_x(int id)
{
	float v = 0;
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(id == targets_[i].id_)
			v = targets_[i].vx_;
	}

	return v;
}


// get the y velocity of target id
float Tracker::get_velocity_y(int id)
{
	float v = 0;
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(id == targets_[i].id_)
			v = targets_[i].vy_;
	}

	return v;
}


// get a new sampled box of target id
float Tracker::get_sample_box(int id, BBOX &bbox)
{
	float prob;

	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(id == targets_[i].id_)
			prob = targets_[i].sample_location(parameter_.sigma_det_x, parameter_.sigma_det_y, bbox);
	}

	return prob;
}
