#pragma once
#include <opencv.hpp>
#include <map>
#include <algorithm>
#include <numeric>
class preprocessing
{
public:
	typedef int channel_t;

	//构造函数
	preprocessing() = default;
	preprocessing(cv::Mat a) :input(a) {};

	//将RGB图像转换为灰度图像的函数
	cv::Mat bgr2gray();

private:
	//定义一些常量来替代魔法数字
	static const channel_t B = 0;
	static const channel_t G = 1;
	static const channel_t R = 2;
	static const int THRESHOLD_WEIGHT = 20;

	//RGB图像
	cv::Mat input;

	//三通道灰度占比统计量结构体
	struct proportion_t {
		channel_t channel;
		double weight;
	};

	//计算图像的补偿权重
	void calculateWeight(cv::Mat img_channels[3], double weight[3]);

	//转换图像的色彩空间和色彩深度
	void convertColor(cv::Mat& input, cv::Mat& output);
};
