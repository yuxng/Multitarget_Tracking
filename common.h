/*
 * common.h
 *
 *  Created on: Aug 18, 2014
 *      Author: yuxiang
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <vector>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define PLUS_INFINITY 1.0E32
#define MAX_VALUE(x,y) ((x) < (y) ? (y) : (x))
#define MIN_VALUE(x,y) ((x) > (y) ? (y) : (x))

typedef enum
{
	TARGET_ADDED,
	TARGET_ACTIVE,
	TARGET_INACTIVE,
	TARGET_TRACKED,
	TARGET_LOST
}TARGET_STATUS;


#define MOVE_NUM 5
typedef enum
{
	MOVE_ADD,
	MOVE_DELETE,
	MOVE_STAY,
	MOVE_LEAVE,
	MOVE_UPDATE
}MOVE_TYPE;

class Target;
typedef struct sample
{
	float motion_prior;
	float motion_prior_new;
	std::vector<Target> targets;
}SAMPLE;

typedef std::pair<std::size_t, std::size_t> SAMPLE_INDEX;

typedef struct parameter
{
	int num_sample;		// number of samples

	// probability of moves
	float prob_moves[MOVE_NUM];

	// standard deviation to perturb detections
	float sigma_det_x;
	float sigma_det_y;

	// detection threshold
	float det_threshold;

	int num_active2tracked;
	float frac_lost2inactive;

	float fix_detection_size;
}PARAMETER;

float log_gaussian_prob(float x, float m, float std);
void assignmentoptimal(float *assignment, float *cost, float *distMatrixIn, int nOfRows, int nOfColumns, float infval);

#endif /* COMMON_H_ */
