#pragma once
#include <opencv.hpp>
#include <map>
class preprocessing {
private:
	static const double templateChannelWeight[3];
	cv::Mat img;
public:
	preprocessing() = default;
	cv::Mat rgb2gray();
};
