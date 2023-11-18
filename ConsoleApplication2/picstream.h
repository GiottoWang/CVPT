#pragma once
#include <opencv.hpp>
class picstream
{public:
	typedef std::pair<std::string, cv::Mat> elemType;
	typedef std::map<std::string, cv::Mat> streamType;
	streamType stream;
	void show();
	static void time_clock(void(*foo)());

};

