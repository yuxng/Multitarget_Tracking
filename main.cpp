/*
 * main.cpp
 *
 *  Created on: Aug 12, 2014
 *      Author: yuxiang
 */

#include <iostream>
#include <string>
#include "Tracker.h"

int main(int argc, char** argv)
{
	Tracker tracker;

	tracker.read_image_paths(argv[1]);
	tracker.read_confidence_paths(argv[2]);

	tracker.initialize_tracker();

	// process each frame
	while(tracker.is_the_end() == false)
	{
		tracker.process_frame();

		cv::waitKey(1);
		tracker.next_frame();
	}

	tracker.terminate_tracker();
}
