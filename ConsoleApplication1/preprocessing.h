#pragma once
#include <opencv.hpp>
#include <map>
#include <algorithm>
#include <numeric>
class preprocessing {
public:
	typedef int _channel_t;
	preprocessing() = default;
	cv::Mat bgr2Gray();
private:
	//enum Channel { B, G, R };
	//enum Threshold { TH_WHIGHT = 20 };
	static const double templateChannelWeight[3];
	static const _channel_t B = 0;
	static const _channel_t G = 1;
	static const _channel_t R = 2;
	static const int THRESHOLD_WEIGHT = 20;
	cv::Mat img;
	//��װһ������������ͼ���Ȩ�غͲ���
	void calculateWeightAndCompensation(cv::Mat img_channels[3], double weight[3], double compensation[3]);

	//��װһ��������ת��ͼ���ɫ�ʿռ��ɫ�����
	void convertColorSpaceAndDepth(cv::Mat& img, cv::Mat& output);

};
