#pragma once
#include <opencv.hpp>
#include <map>
#include<vector>
#include <algorithm>
#include <numeric>

class preprocessing
{
public:
	typedef int channel_t;

	//构造函数
	preprocessing() = default;
	preprocessing(cv::Mat a) :data(a) {};

	//将RGB图像转换为灰度图像的函数
	cv::Mat process();
	
private:
	//定义一些常量来替代魔法数字
	constexpr static  channel_t B = 0;
	constexpr static  channel_t G = 1;
	constexpr static  channel_t R = 2;
	constexpr static  int THRESHOLD_WEIGHT = 20;
	constexpr static double THRESHOLD_NOISE = 0.03;

	//输入图像
	cv::Mat data;
	//输出图像
	cv::Mat output;

	//三通道灰度占比统计量结构体
	struct count_t {
		channel_t channel;
		double weight;
	};
	struct spot_t {
		int x;
		int y;
		int num;
	};

	//计算图像的补偿权重
	void calculateWeight(cv::Mat img_channels[3], double weight[3]);

	//转换图像的色彩空间
	void convertColor();

	//对灰度图像进行去噪
	void deNoise();

	//对低曝光图像进行取反
	void invertion();
};

