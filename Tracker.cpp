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
	parameter_.num_sample = 200;

	parameter_.prob_moves[MOVE_ADD] = 0.1;
	parameter_.prob_moves[MOVE_DELETE] = 0.1;
	parameter_.prob_moves[MOVE_STAY] = 0.1;
	parameter_.prob_moves[MOVE_LEAVE] = 0.1;
	parameter_.prob_moves[MOVE_UPDATE] = 0.6;

	parameter_.sigma_det_x = 0.01;
	parameter_.sigma_det_y = 0.01;

	parameter_.det_threshold = 0;
	parameter_.det_weight = 0.4;

	parameter_.num_active2tracked = 2;
	parameter_.frac_lost2inactive = 0.01;

	parameter_.fix_detection_size = 40;
	parameter_.heatmap_scale = 2.1405;
	// parameter_.heatmap_scale = 2.3529;

	parameter_.dir_detection = "cache/detection/";
	parameter_.dir_tracking = "cache/tracking/";
	parameter_.filename = "cache/results.txt";

	// open result file
	result_file_.open(parameter_.filename.c_str());
	result_file_ << "frame_id, target_id, target_status, center_x, center_y, width, height, score" << std::endl;

	// clean cache
	std::string cmd = "rm " + parameter_.dir_detection + "*";
	std::system(cmd.c_str());
	cmd = "rm " + parameter_.dir_tracking + "*";
	std::system(cmd.c_str());
}


// terminate tracker
void Tracker::terminate_tracker()
{
	result_file_.close();
	image_paths_.clear();
	confidence_paths_.clear();
	targets_.clear();
	samples_.clear();
}


// check if the end of the video
bool Tracker::is_the_end()
{
	return image_path_iterator_ == image_paths_.end();
}


// read confidence file
bool Tracker::read_confidence_file(const std::string filename, std::vector<Target> &targets, cv::Mat &confidence)
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
	Target target;
	assert(sizeof(unsigned int) == 4);

	nread = fread(&nums, sizeof(unsigned int), 1, fp);
	assert(nread == 1);
	for(size_t i = 0; i < nums; i++)
	{
		nread = fread(det, sizeof(float), 5, fp);
		assert(nread == 5);

		if(det[4] > parameter_.det_threshold)
		{
			target.id_ = -1;
			target.cx_ = det[0] + det[2] / 2;
			target.cy_ = det[1] + det[3] / 2;
			if(parameter_.fix_detection_size)
			{
				target.width_ = parameter_.fix_detection_size;
				target.height_ = parameter_.fix_detection_size;
			}
			else
			{
				target.width_ = det[2];
				target.height_ = det[3];
			}
			target.sigmax_ = parameter_.sigma_det_x * target.width_;
			target.sigmay_ = parameter_.sigma_det_y * target.height_;
			target.sigmaw_ = parameter_.sigma_det_x * target.width_;
			target.sigmah_ = parameter_.sigma_det_y * target.height_;
			target.score_ = parameter_.det_weight * (det[4] + 0.5) + (1 - parameter_.det_weight) * 1.0;
			target.status_ = TARGET_ADDED;
			target.count_active_ = 0;
			target.count_lost_ = 0;
			target.count_tracked_ = 0;
			targets.push_back(target);
			std::cout << "Detection " << i << " score " << target.score_ << std::endl;
		}
	}

	// read confidence map
	float *data;
	int map_size[2];

	nread = fread(map_size, sizeof(int), 2, fp);
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
	std::vector<Target> targets;
	cv::Mat confidence;
	bool success = read_confidence_file(*confidence_path_iterator_, targets, confidence);
	if(success)
		std::cout << targets.size() << " new detections read" << std::endl;

	if(is_empty_target() == true)
	{
		// initialize the targets by detections
		for(std::size_t i = 0; i < targets.size(); i++)
		{
			targets[i].id_ = target_id_;
			targets[i].status_ = TARGET_ACTIVE;
			SAMPLE_INDEX sample_index(0, i);
			targets[i].sample_indexes_.push_back(sample_index);
			targets_.push_back(targets[i]);
			target_id_++;
		}

		// initialize the sample
		if(targets.size() > 0)
		{
			SAMPLE sample;
			sample.targets = targets;
			samples_.push_back(sample);
		}
	}
	else  // RJMCMC particle filtering
	{
		run_rjmcmc_sampling(targets, confidence);
	}

	// show object detection results
	cv::Mat image_detection = image.clone();
	for(std::size_t i = 0; i < targets.size(); i++)
	{
		float x1 = targets[i].cx_ - targets[i].width_ / 2;
		float x2 = targets[i].cx_ + targets[i].width_ / 2;
		float y1 = targets[i].cy_ - targets[i].height_ / 2;
		float y2 = targets[i].cy_ + targets[i].height_ / 2;
		cv::rectangle(image_detection, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);

		std::ostringstream text;
		text << "D" << i << ", " << std::setprecision(2) << targets[i].score_;
		cv::putText(image_detection, text.str(), cv::Point(x1, y1), cv::FONT_HERSHEY_SIMPLEX, .4 * image.cols / 500, cv::Scalar(0, 255, 0), image.cols / 500);
	}
	cv::namedWindow("Detection", cv::WINDOW_AUTOSIZE);  // Create a window for display.
	cv::imshow("Detection", image_detection);  // Show our image inside it.

	// save detection image
	std::ostringstream filename;
	filename << std::setfill('0');
	filename << parameter_.dir_detection << std::setw(6) << image_path_iterator_ - image_paths_.begin() << ".jpg";
	cv::imwrite(filename.str(), image_detection);

	// show tracking results
	cv::Mat image_tracking = image.clone();
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(targets_[i].status_ != TARGET_TRACKED && targets_[i].status_ != TARGET_LOST)
			continue;
		float x1 = targets_[i].cx_ - targets_[i].width_ / 2;
		float x2 = targets_[i].cx_ + targets_[i].width_ / 2;
		float y1 = targets_[i].cy_ - targets_[i].height_ / 2;
		float y2 = targets_[i].cy_ + targets_[i].height_ / 2;
		cv::Scalar color = cv::Scalar(((i * 120) % 256), ((i * 60) % 256), ((i * 30) % 256));
		if(targets_[i].status_ == TARGET_TRACKED)
			cv::rectangle(image_tracking, cv::Point(x1, y1), cv::Point(x2, y2), color, 2);
		else
			cv::rectangle(image_tracking, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 2);

		std::ostringstream text;
		text << "T" << targets_[i].id_ << ", " << std::setprecision(2) << targets_[i].score_;
		cv::putText(image_tracking, text.str(), cv::Point(x1, y1), cv::FONT_HERSHEY_SIMPLEX, .4 * image.cols / 500, color, image.cols / 500);
	}
	cv::namedWindow("Tracking", cv::WINDOW_AUTOSIZE);  // Create a window for display.
	cv::imshow("Tracking", image_tracking);  // Show our image inside it.

	// save tracking image
	filename.str("");
	filename.clear();
	filename << std::setfill('0');
	filename << parameter_.dir_tracking << std::setw(6) << image_path_iterator_ - image_paths_.begin() << ".jpg";
	cv::imwrite(filename.str(), image_tracking);

	// write tracking results
	int frame_id = image_path_iterator_ - image_paths_.begin();
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		Target target = targets_[i];
		if(target.status_ != TARGET_INACTIVE && target.status_ != TARGET_ADDED)
			result_file_ << frame_id << " " << target.id_ << " " << target.status_ << " " << target.cx_ << " " << target.cy_ << " "
				<< target.width_ << " " << target.height_ << " " << target.score_ << std::endl;
	}

	// clean up
	if(success)
	{
		targets.clear();
		confidence.release();
	}
}


// proceed to the next frame
void Tracker::next_frame()
{
	image_path_iterator_++;
	confidence_path_iterator_++;
}


// check if the target set is empty or not
bool Tracker::is_empty_target()
{
	bool is_empty = true;

	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(targets_[i].status_ != TARGET_INACTIVE)
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

	// apply motion model to each target
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(targets_[i].status_ != TARGET_INACTIVE)
		{
			Target target = targets_[i].apply_motion_model();
			sample.targets.push_back(target);
		}
	}

	return sample;
}


// compute mean sample
SAMPLE Tracker::get_mean_sample()
{
	SAMPLE sample;

	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		targets_[i].sample_indexes_.clear();
		SAMPLE_INDEX sample_index(0, i);
		targets_[i].sample_indexes_.push_back(sample_index);
	}

	sample.targets = targets_;
	return sample;
}


// Reversible jump MCMC sampling
// targets: object detections
// confidence: object detection heat map
void Tracker::run_rjmcmc_sampling(std::vector<Target> targets, cv::Mat confidence)
{
	SAMPLE sample_init = get_initial_sample();

	// data association with Hungarian algorithm
	float *assignment = hungarian(sample_init.targets, targets);

	// assign detections to targets
	num_stayed_ = 0;
	for(std::size_t i = 0; i < sample_init.targets.size(); i++)
	{
		TARGET_STATUS status = sample_init.targets[i].status_;
		int index = assignment[i];
		std::cout << index << " ";
		if(index != -1)
		{
			targets[index].id_ = sample_init.targets[i].id_;
			sample_init.targets[i].cx_ = targets[index].cx_;
			sample_init.targets[i].cy_ = targets[index].cy_;
			sample_init.targets[i].width_ = 0.9*sample_init.targets[i].width_ + 0.1*targets[index].width_;
			sample_init.targets[i].height_ = 0.9*sample_init.targets[i].height_ + 0.1*targets[index].height_;
			sample_init.targets[i].score_ = targets[index].score_;
			if((status == TARGET_ACTIVE && ++sample_init.targets[i].count_active_ >= parameter_.num_active2tracked) || status == TARGET_TRACKED || status == TARGET_LOST)
			{
				sample_init.targets[i].status_ = TARGET_TRACKED;
				sample_init.targets[i].count_lost_ = 0;
				sample_init.targets[i].count_tracked_++;
				num_stayed_++;
			}
		}
		else
		{
			if(status == TARGET_ACTIVE)
				sample_init.targets[i].status_ = TARGET_INACTIVE;
			else
			{
				if(++sample_init.targets[i].count_lost_ >= MAX(parameter_.frac_lost2inactive * sample_init.targets[i].count_tracked_, 2))
				{
					sample_init.targets[i].status_ = TARGET_INACTIVE;
					std::cout << std::endl << "inactive " << sample_init.targets[i].count_lost_ << " " << sample_init.targets[i].count_tracked_ << std::endl;
				}
				else
				{
					sample_init.targets[i].status_ = TARGET_LOST;
					num_stayed_++;
				}
			}
		}
	}
	std::cout << "assignment" << std::endl;

	// collect the targets to be added
	std::vector<Target> targets_add;
	for(std::size_t i = 0; i < targets.size(); i++)
	{
		if(targets[i].id_ == -1)
		{
			targets[i].id_ = target_id_++;
			targets_add.push_back(targets[i]);
		}
	}
	num_added_ = targets_add.size();		// total number of objects can be added
	std::cout << num_added_ << " objects to be added, " << num_stayed_ << " objects to be stayed" << std::endl;

	// generate samples
	SAMPLE sample_prev = sample_init;
	SAMPLE sample;
	std::vector<SAMPLE> samples;
	std::vector<Target> targets_stay;
	samples.push_back(sample_init);
	compute_motion_prior(sample_init);
	if(num_added_ > 0 || num_stayed_ > 0)
	{
		for(int i = 0; i < parameter_.num_sample; i++)
		{
			// determine the move type
			MOVE_TYPE move;
			while(1)
			{
				double val = rng_.uniform((double)0, (double)1);
				if((val -= parameter_.prob_moves[MOVE_ADD]) < 0)
				{
					// add an object
					move = MOVE_ADD;
					if(targets_add.size() > 0)
						break;
				}
				else if((val -= parameter_.prob_moves[MOVE_DELETE]) < 0)
				{
					// delete an object
					move = MOVE_DELETE;
					if(targets_add.size() < (std::size_t)num_added_)
						break;
				}
				else if((val -= parameter_.prob_moves[MOVE_STAY]) < 0)
				{
					// stay move
					move = MOVE_STAY;
					if(targets_stay.size() > 0)
						break;
				}
				else if((val -= parameter_.prob_moves[MOVE_LEAVE]) < 0)
				{
					// leave move
					move = MOVE_LEAVE;
					if(targets_stay.size() < (std::size_t)num_stayed_)
						break;
				}
				else if((val -= parameter_.prob_moves[MOVE_UPDATE]) < 0)
				{
					// update move
					move = MOVE_UPDATE;
					if(targets_stay.size() < (std::size_t)num_stayed_)
						break;
				}
				else
					assert(0);
			}

			std::size_t target_id = 0;
			float acceptance_ratio = 0;

			switch(move)
			{
			case MOVE_ADD:
				sample = add_target(sample_prev, targets_add, target_id, acceptance_ratio);
				break;
			case MOVE_DELETE:
				sample = delete_target(sample_prev, targets_add, target_id, acceptance_ratio);
				break;
			case MOVE_STAY:
				sample = stay_target(sample_prev, targets_stay, target_id, acceptance_ratio);
				break;
			case MOVE_LEAVE:
				sample = leave_target(sample_prev, targets_stay, target_id, acceptance_ratio);
				break;
			case MOVE_UPDATE:
				sample = update_target(sample_prev, targets_stay, targets, confidence, acceptance_ratio);
				break;
			}

			// determine to accept the new sample or not
			double val = rng_.uniform((double)0, (double)1);
			if(acceptance_ratio > val)	// accept the sample
			{
				// update the add and stay sets
				switch(move)
				{
				case MOVE_ADD:
					targets_add.erase(targets_add.begin() + target_id);
					break;
				case MOVE_DELETE:
					targets_add.push_back(sample_prev.targets[target_id]);
					break;
				case MOVE_STAY:
					targets_stay.erase(targets_stay.begin() + target_id);
					break;
				case MOVE_LEAVE:
					targets_stay.push_back(sample_prev.targets[target_id]);
					break;
				case MOVE_UPDATE:
					break;
				}

				// store and update the sample
				samples.push_back(sample);
				sample_prev = sample;

				// update the motion prior
				update_motion_prior(move);
			}
			std::cout << "move type " << move << " acceptance ratio " << acceptance_ratio << " number of stay targets " << targets_stay.size() << std::endl;
		}
	}
	samples_.clear();
	samples_ = samples;

	// extend the target set
	int num_target = targets_.size();
	for(int i = num_target; i < target_id_; i++)
	{
		Target target;
		target.id_ = i;
		target.status_ = TARGET_ADDED;
		targets_.push_back(target);
	}
	std::cout << targets_.size() << " targets after sampling" << std::endl;

	// clean up the target samples
	for(std::size_t i = 0; i < targets_.size(); i++)
		targets_[i].sample_indexes_.clear();

	std::cout << samples_.size() << " samples after sampling" << std::endl;
	// extract the samples for the targets
	for(std::size_t i = 0; i < samples_.size(); i++)
	{
		for(std::size_t j = 0; j < samples_[i].targets.size(); j++)
		{
			int id = samples_[i].targets[j].id_;
			SAMPLE_INDEX sample_index(i, j);
			targets_[id].sample_indexes_.push_back(sample_index);
		}
	}

	// compute the mean sample location for each target
	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		TARGET_STATUS status = TARGET_INACTIVE;
		float cx = 0, cy = 0, w = 0, h = 0, score = 0, vx = 0, vy = 0, count_active = 0, count_lost = 0, count_tracked = 0;
		std::size_t num = targets_[i].sample_indexes_.size();

		for(std::size_t j = 0; j < num; j++)
		{
			SAMPLE_INDEX sample_index = targets_[i].sample_indexes_[j];
			Target target = samples_[sample_index.first].targets[sample_index.second];
			cx += target.cx_;
			cy += target.cy_;
			w += target.width_;
			h += target.height_;
			score += target.score_;
			status = target.status_;
			count_active = target.count_active_;
			count_lost = target.count_lost_;
			count_tracked = target.count_tracked_;
		}
		if(num > 0)
		{
			cx /= num;
			cy /= num;
			w /= num;
			h /= num;
			score /= num;
			if(status == TARGET_TRACKED || status == TARGET_LOST)
			{
				vx = 0.5*targets_[i].vx_ + 0.5*(cx - targets_[i].cx_);
				vy = 0.5*targets_[i].vy_ + 0.5*(cy - targets_[i].cy_);
			}
			else if(status == TARGET_ADDED)
				status = TARGET_ACTIVE;
		}
		// estimate variances
		float sigmax = 0, sigmay = 0, sigmaw = 0, sigmah = 0;
		for(std::size_t j = 0; j < num; j++)
		{
			SAMPLE_INDEX sample_index = targets_[i].sample_indexes_[j];
			Target target = samples_[sample_index.first].targets[sample_index.second];
			sigmax += (target.cx_ - cx) * (target.cx_ - cx);
			sigmay += (target.cy_ - cy) * (target.cy_ - cy);
			sigmaw += (target.width_ - w) * (target.width_ - w);
			sigmah += (target.height_ - h) * (target.height_ - h);
		}
		if(num > 0)
		{
			sigmax = sqrt(sigmax / num);
			sigmay = sqrt(sigmay / num);
			sigmaw = sqrt(sigmaw / num);
			sigmah = sqrt(sigmah / num);
		}
		else
		{
			sigmax = parameter_.sigma_det_x * w;
			sigmay = parameter_.sigma_det_y * h;
			sigmaw = parameter_.sigma_det_x * w;
			sigmah = parameter_.sigma_det_y * h;
		}
		if(sigmax == 0)
			sigmax = parameter_.sigma_det_x * w;
		if(sigmay == 0)
			sigmay = parameter_.sigma_det_y * h;
		if(sigmaw == 0)
			sigmaw = parameter_.sigma_det_x * w;
		if(sigmah == 0)
			sigmah = parameter_.sigma_det_y * h;

		targets_[i].cx_ = cx + fabs(rng_.gaussian(7.5));
		targets_[i].cy_ = cy + fabs(rng_.gaussian(7.5));
		targets_[i].width_ = w;
		targets_[i].height_ = h;
		targets_[i].score_ = score;
		targets_[i].vx_ = vx;
		targets_[i].vy_ = vy;
		targets_[i].sigmax_ = sigmax;
		targets_[i].sigmay_ = sigmay;
		targets_[i].sigmaw_ = sigmaw;
		targets_[i].sigmah_ = sigmah;
		targets_[i].status_ = status;
		targets_[i].count_active_ = count_active;
		targets_[i].count_lost_ = count_lost;
		targets_[i].count_tracked_ = count_tracked;

		if(status != TARGET_INACTIVE)
			std::cout << "target " << i << " " << num << " samples, status " << status << " score " << score
				<< " vx " << vx << " vy " << vy << " sigmax " << sigmax << " sigmay " << sigmay <<
				" sigmaw " << sigmaw << " sigmah " << sigmah <<
				" num tracked " << count_tracked << " num lost " << count_lost << std::endl;
	}

	// just keep the mean sample
	samples_.clear();
	samples_.push_back(get_mean_sample());
}

// Hungarian algorithm for data association
float* Tracker::hungarian(std::vector<Target> bboxes_target, std::vector<Target> bboxes)
{
	float dis, cost;
	float *assign = new float [bboxes_target.size()];
	float *distMat = new float [bboxes_target.size() * bboxes.size()];

	for(std::size_t i = 0; i < bboxes_target.size(); i++)
	{
		for(std::size_t j = 0; j < bboxes.size(); j++)
		{
			dis = target_distance(bboxes_target[i], bboxes[j]);
			if(dis > bboxes_target[i].width_)	// thresholding
				dis = PLUS_INFINITY;
			distMat[i + bboxes_target.size() * j] = dis;
		}
	}
	// solve assignment problem
	assignmentoptimal(assign, &cost, distMat, bboxes_target.size(), bboxes.size(), PLUS_INFINITY);

	delete distMat;
	return assign;
}


// compute bounding box overlap
float Tracker::target_overlap(Target t1, Target t2)
{
	float overlap = 0;

	float x1 = MAX_VALUE(t1.cx_ - t1.width_/2, t2.cx_ - t2.width_/2);
	float y1 = MAX_VALUE(t1.cy_ - t1.height_/2, t2.cy_ - t2.height_/2);
	float x2 = MIN_VALUE(t1.cx_ + t1.width_/2, t2.cx_ + t2.width_/2);
	float y2 = MIN_VALUE(t1.cy_ + t1.height_/2, t2.cy_ + t2.height_/2);
	float a1 = t1.width_ * t1.height_;
	float a2 = t2.width_ * t2.height_;

	if((x1 < x2) && (y1 < y2))
	{
		float intersect = (x2 - x1) * (y2 - y1);
		overlap = intersect / (a1 + a2 - intersect);
	}

	return overlap;
}


// compute bounding box overlap
float Tracker::target_distance(Target t1, Target t2)
{
	return sqrt((t1.cx_ - t2.cx_) * (t1.cx_ - t2.cx_) + (t1.cy_ - t2.cy_) * (t1.cy_ - t2.cy_));
}


// add move
SAMPLE Tracker::add_target(SAMPLE sample_prev, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio)
{
	if(targets.size() == 0)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to add
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)targets.size()));
	SAMPLE sample = sample_prev;
	Target target = targets[idx];
	sample.targets.push_back(target);

	// compute acceptance ratio
	float num_add = targets.size();
	float num_delete = num_added_ - num_add + 1;
	acceptance_ratio = target.score_ * parameter_.prob_moves[MOVE_DELETE] * num_add / num_delete;

	// return the object id added
	target_id = idx;

	return sample;
}


// delete move
SAMPLE Tracker::delete_target(SAMPLE sample_prev, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio)
{
	if(targets.size() == (std::size_t)num_added_)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to delete
	int num_delete = num_added_ - targets.size();
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)num_delete));

	SAMPLE sample = sample_prev;
	std::size_t index = 0;
	for(std::size_t i = 0; i < sample.targets.size(); i++)
	{
		if(sample.targets[i].status_ == TARGET_ADDED)
		{
			if(index == idx)
				break;
			index++;
		}
	}
	// remove the target
	Target target = sample.targets[index];
	sample.targets.erase(sample.targets.begin() + index);

	// return the object id to be deleted
	target_id = index;

	// compute acceptance ratio
	float num_add = targets.size() + 1;
	acceptance_ratio = (1 / target.score_) * (1 / parameter_.prob_moves[MOVE_DELETE]) * (float)num_delete / num_add;

	return sample;
}


// stay move
SAMPLE Tracker::stay_target(SAMPLE sample_prev, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio)
{
	if(targets.size() == 0)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to stay
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)targets.size()));
	Target target = targets[idx];
	Target target_tmp;
	float prob = get_sample_target(target.id_, target_tmp);

	// add the target to sample
	SAMPLE sample = sample_prev;
	sample.targets.push_back(target);

	// compute acceptance ratio
	float motion_ratio = compute_motion_ratio(target, MOVE_STAY);
	float move_ratio = parameter_.prob_moves[MOVE_LEAVE] / parameter_.prob_moves[MOVE_STAY];
	float num_stay = targets.size();
	float num_leave = num_stayed_ - num_stay + 1;
	// std::cout << "In stay target, prob = " << prob << " num leave " << num_leave << " score " << target.score_ <<
	//		" motion ratio " << motion_ratio << " move ratio " << move_ratio << " num stay " << num_stay << std::endl;
	acceptance_ratio = target.score_ * motion_ratio * move_ratio * num_stay / (num_leave * prob);

	// return the object id stayed
	target_id = idx;

	return sample;
}


// leave move
SAMPLE Tracker::leave_target(SAMPLE sample_prev, std::vector<Target> targets, std::size_t &target_id, float &acceptance_ratio)
{
	if(targets.size() == (std::size_t)num_stayed_)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to delete
	int num_leave = num_stayed_ - targets.size();
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)num_leave));

	SAMPLE sample = sample_prev;
	std::size_t index = 0;
	for(std::size_t i = 0; i < sample.targets.size(); i++)
	{
		TARGET_STATUS status = sample.targets[i].status_;
		if(status == TARGET_TRACKED || status == TARGET_LOST)
		{
			if(index == idx)
				break;
			index++;
		}
	}
	// remove the target
	Target target = sample.targets[index];
	sample.targets.erase(sample.targets.begin() + index);

	// return the object id left
	target_id = index;

	// compute acceptance ratio
	float score = target.score_;
	float motion_ratio = compute_motion_ratio(target, MOVE_LEAVE);
	float move_ratio = parameter_.prob_moves[MOVE_STAY] / parameter_.prob_moves[MOVE_LEAVE];
	float prob = get_sample_target(target.id_, target);
	float num_stay = targets.size() + 1;
	acceptance_ratio = (1 / score) * motion_ratio * move_ratio * num_leave * prob / num_stay;

	return sample;
}


// update move
SAMPLE Tracker::update_target(SAMPLE sample_prev, std::vector<Target> targets, std::vector<Target> targets_det,
		cv::Mat confidence, float &acceptance_ratio)
{
	if (targets.size() == (std::size_t)num_stayed_)
	{
		acceptance_ratio = -1;
		return sample_prev;
	}

	// randomly select one object to update
	int num_update = num_stayed_ - targets.size();
	std::size_t idx = floor(rng_.uniform((double)0.0, (double)num_update));

	SAMPLE sample = sample_prev;
	std::size_t index = 0;
	for(std::size_t i = 0; i < sample.targets.size(); i++)
	{
		if(sample.targets[i].status_== TARGET_TRACKED || sample.targets[i].status_== TARGET_LOST)
		{
			if(index == idx)
				break;
			index++;
		}
	}

	Target target = sample.targets[index];
	perturb_target(target);

	// get the confidence score
    // down size by factor 4
	int x = floor(target.cx_ / parameter_.heatmap_scale);
	int y = floor(target.cy_ / parameter_.heatmap_scale);
	// check whether point is in image
	if((x < 0) || (y < 0) || (x > confidence.cols) || (y > confidence.rows))
    {
		acceptance_ratio = -1;
		return sample;
    }
	else
	{
		// compute bounding box overlap
		float overlap = -1;
		for(std::size_t i = 0; i < targets_det.size(); i++)
		{
			float ov = target_overlap(target, targets_det[i]);
			if(ov > overlap)
				overlap = ov;
		}

		target.score_ = parameter_.det_weight * (confidence.at<float>(y, x) + 0.5) + (1 - parameter_.det_weight) * overlap;
		float motion_ratio = compute_motion_ratio(target, MOVE_UPDATE);
		acceptance_ratio = (target.score_ / sample_prev.targets[index].score_) * motion_ratio;

		sample.targets[index] = target;
		return sample;
	}
}


// add Gaussian noise to the bounding box center
void Tracker::perturb_target(Target &target)
{
	// perturb the bounding box center
	target.cx_ += rng_.gaussian(parameter_.sigma_det_x * target.width_);
	target.cy_ += rng_.gaussian(parameter_.sigma_det_y * target.height_);
	target.width_ += rng_.gaussian(0.1);
	target.height_ += rng_.gaussian(0.1);
}


// compute motion prior in log space
void Tracker::compute_motion_prior(SAMPLE sample)
{
	std::vector<Target> targets = sample.targets;

	for(std::size_t i = 0; i < samples_.size(); i++)
	{
		float prior = 0;
		int count = 0;
		std::vector<Target> targets_prev = samples_[i].targets;

		for(std::size_t j = 0; j < targets.size(); j++)
		{
			for(std::size_t k = 0; k < targets_prev.size(); k++)
			{
				if(targets[j].id_ == targets_prev[k].id_)
				{
					// matched target
					count++;
					Target target_new = targets_prev[k].apply_motion_model();

					float log_motion = (log_gaussian_prob(targets[j].cx_, target_new.cx_, target_new.sigmax_)
							+ log_gaussian_prob(targets[j].cy_, target_new.cy_, target_new.sigmay_)
							+ log_gaussian_prob(targets[j].width_, target_new.width_, target_new.sigmaw_)
							+ log_gaussian_prob(targets[j].height_, target_new.height_, target_new.sigmah_)) / 4;

					prior += log_motion;

					// cache the motion prior
					samples_[i].targets[k].motion_prior_ = log_motion;
					break;
				}
			}
		}

		// handle unmatched targets
		prior += log(parameter_.prob_moves[MOVE_LEAVE]) * (targets_prev.size() - count) +
				 log(parameter_.prob_moves[MOVE_ADD]) * (targets.size() - count);

		// save the motion prior
		samples_[i].motion_prior = prior;
		// std::cout << "In compute motion prior, " << "prior " << prior << std::endl;
	}
}


// compute the motion prior ratio for stay move and leave move
float Tracker::compute_motion_ratio(Target target, MOVE_TYPE move)
{
	float prior_sum = 0;
	float prior_new_sum = 0;

	for(std::size_t i = 0; i < samples_.size(); i++)
	{
		int flag = 0;
		float prior = 0;
		float prior_old = 0;
		float motion_prior = samples_[i].motion_prior;
		float motion_prior_new;
		std::vector<Target> targets_prev = samples_[i].targets;

		for(std::size_t k = 0; k < targets_prev.size(); k++)
		{
			if(target.id_ == targets_prev[k].id_)
			{
				// matched target
				flag = 1;
				prior_old = samples_[i].targets[k].motion_prior_;
				if(move == MOVE_LEAVE)
				{
					samples_[i].targets[k].motion_prior_new_ = 0;
				}
				else
				{
					Target target_new = targets_prev[k].apply_motion_model();

					float log_motion = (log_gaussian_prob(target.cx_, target_new.cx_, target_new.sigmax_)
							+ log_gaussian_prob(target.cy_, target_new.cy_, target_new.sigmay_)
							+ log_gaussian_prob(target.width_, target_new.width_, target_new.sigmaw_)
							+ log_gaussian_prob(target.height_, target_new.height_, target_new.sigmah_)) / 4;
					// std::cout << "In compute motion ratio, " << "log motion " << log_motion << " prior old " << prior_old  << std::endl;
					prior = log_motion;
					// cache the motion prior
					samples_[i].targets[k].motion_prior_new_ = prior;
				}
				break;
			}
		}

		if(move == MOVE_STAY)
		{
			if(flag)
				motion_prior_new = motion_prior - log(parameter_.prob_moves[MOVE_LEAVE]) + prior;
			else
				motion_prior_new = motion_prior + log(parameter_.prob_moves[MOVE_ADD]);
		}
		else if(move == MOVE_LEAVE)
		{
			if(flag)
				motion_prior_new = motion_prior - prior_old + log(parameter_.prob_moves[MOVE_LEAVE]);
			else
				motion_prior_new = motion_prior - log(parameter_.prob_moves[MOVE_ADD]);
		}
		else if(move == MOVE_UPDATE)
		{
			if(flag)
				motion_prior_new = motion_prior - prior_old + prior;
			else
				motion_prior_new = motion_prior;
		}

		prior_sum += motion_prior;
		prior_new_sum += motion_prior_new;
		// std::cout << "In compute motion ratio, " << "motion prior " << motion_prior << ", motion prior new " << motion_prior_new << std::endl;
		// std::cout << "In compute motion ratio, " << "prior sum " << prior_sum << ", prior sum new " << prior_new_sum << std::endl;

		// save the motion prior
		samples_[i].motion_prior_new = motion_prior_new;
	}

	return exp(prior_new_sum - prior_sum);
}


// update the motion prior
void Tracker::update_motion_prior(MOVE_TYPE move)
{
	switch(move)
	{
	case MOVE_ADD:
		for(std::size_t i = 0; i < samples_.size(); i++)
			samples_[i].motion_prior += log(parameter_.prob_moves[MOVE_ADD]);
		break;
	case MOVE_DELETE:
		for(std::size_t i = 0; i < samples_.size(); i++)
			samples_[i].motion_prior -= log(parameter_.prob_moves[MOVE_ADD]);
		break;
	case MOVE_STAY:
	case MOVE_LEAVE:
	case MOVE_UPDATE:
		for(std::size_t i = 0; i < samples_.size(); i++)
		{
			samples_[i].motion_prior = samples_[i].motion_prior_new;
			for(std::size_t j = 0; j < samples_[i].targets.size(); j++)
				samples_[i].targets[j].motion_prior_ = samples_[i].targets[j].motion_prior_new_;
		}
	}
}


// get a new sampled box of target id
float Tracker::get_sample_target(int id, Target &target)
{
	float prob = 0;

	for(std::size_t i = 0; i < targets_.size(); i++)
	{
		if(id == targets_[i].id_)
		{
			prob = sample_location(targets_[i], target);
			break;
		}
	}

	return prob;
}


// sample a new location of the target
float Tracker::sample_location(Target target, Target &target_sample)
{
	float prob = 0, mx = 0, my = 0, mw = 0, mh = 0, score = 0;
	std::size_t num = target.sample_indexes_.size();

	for(std::size_t i = 0; i < num; i++)
	{
		SAMPLE_INDEX sample_index = target.sample_indexes_[i];
		Target target_new = samples_[sample_index.first].targets[sample_index.second].apply_motion_model();
		float cx = target_new.cx_;
		float cy = target_new.cy_;
		float w = target_new.width_;
		float h = target_new.height_;

		float rx = rng_.gaussian(parameter_.sigma_det_x * w);
		float ry = rng_.gaussian(parameter_.sigma_det_y * h);
		float rw = rng_.gaussian(parameter_.sigma_det_x * w);
		float rh = rng_.gaussian(parameter_.sigma_det_y * h);

		prob += exp((log_gaussian_prob(rx, 0, target_new.sigmax_) + log_gaussian_prob(ry, 0, target_new.sigmay_)
				  + log_gaussian_prob(rw, 0, target_new.sigmaw_) + log_gaussian_prob(rh, 0, target_new.sigmah_))/4);

		mx += cx + rx;
		my += cy + ry;
		mw += w + rw;
		mh += h + rh;
		score += target_new.score_;
	}

	if(num)
	{
		mx /= num;
		my /= num;
		mw /= num;
		mh /= num;
		score /= num;
	}

	// build the sampled box
	target_sample.id_ = target.id_;
	target_sample.cx_ = mx;
	target_sample.cy_ = my;
	target_sample.width_ = mw;
	target_sample.height_ = mh;
	target_sample.score_ = score;
	target_sample.vx_ = target.vx_;
	target_sample.vy_ = target.vy_;

	return prob;
}
