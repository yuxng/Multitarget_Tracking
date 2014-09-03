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
	BOX_ADDED,
	BOX_TRACKED
}BOX_STATUS;


typedef struct bbox
{
	int id;
	float x1;
	float y1;
	float x2;
	float y2;
	float score;
	BOX_STATUS status;
}BBOX;

typedef enum
{
	ACTIVE,
	INACTIVE,
	TRACKED,
	LOST
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


typedef struct sample
{
	float motion_prior;
	float motion_prior_new;
	std::vector<BBOX> bboxes;
}SAMPLE;


typedef struct parameter
{
	int num_sample;		// number of samples

	// probability of moves
	float prob_moves[MOVE_NUM];

	// minimum bounding box overlap in data association
	float min_overlap;

	// standard deviation to perturb detections
	float sigma_det_x;
	float sigma_det_y;
}PARAMETER;

float box_overlap(BBOX b1, BBOX b2);
float log_gaussian_prob(float x, float m, float std);
void assignmentoptimal(float *assignment, float *cost, float *distMatrixIn, int nOfRows, int nOfColumns, float infval);

#endif /* COMMON_H_ */
