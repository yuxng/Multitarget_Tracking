/*
 * common.cpp
 *
 *  Created on: Aug 29, 2014
 *      Author: yuxiang
 */

#include "common.h"

// compute the log of the Gaussian probability
float log_gaussian_prob(float x, float m, float std)
{
	if(std == 0)
		return 0.0;
	else
		return -log(sqrt(2 * M_PI) * std) - ( pow((x - m) / std, 2) / 2.0 );
}
