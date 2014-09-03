/*
 * common.cpp
 *
 *  Created on: Aug 29, 2014
 *      Author: yuxiang
 */

#include "common.h"

// compute bounding box overlap
float box_overlap(BBOX b1, BBOX b2)
{
	float overlap = 0;

	float x1 = MAX_VALUE(b1.x1, b2.x1);
	float y1 = MAX_VALUE(b1.y1, b2.y1);
	float x2 = MIN_VALUE(b1.x2, b2.x2);
	float y2 = MIN_VALUE(b1.y2, b2.y2);
	float a1 = (b1.x2 - b1.x1) * (b1.y2 - b1.y1);
	float a2 = (b2.x2 - b2.x1) * (b2.y2 - b2.y1);

	if((x1 < x2) && (y1 < y2))
	{
		float intersect = (x2 - x1) * (y2 - y1);
		overlap = intersect / (a1 + a2 - intersect);
	}

	return overlap;
}


// compute the log of the Gaussian probability
float log_gaussian_prob(float x, float m, float std)
{
	if(std == 0)
		return 0.0;
	else
		return -log(sqrt(2 * M_PI) * std) - ( pow((x - m) / std, 2) / 2.0 );
}
