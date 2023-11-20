#pragma once
#include <opencv.hpp>
#include <map>
#include <algorithm>
#include <numeric>
class preprocessing
{
public:
	typedef int channel_t;

	//���캯��
	preprocessing() = default;
	preprocessing(cv::Mat a) :input(a) {};

	//��RGBͼ��ת��Ϊ�Ҷ�ͼ��ĺ���
	cv::Mat bgr2gray();

private:
	//����һЩ���������ħ������
	static const channel_t B = 0;
	static const channel_t G = 1;
	static const channel_t R = 2;
	static const int THRESHOLD_WEIGHT = 20;

	//RGBͼ��
	cv::Mat input;

	//��ͨ���Ҷ�ռ��ͳ�����ṹ��
	struct proportion_t {
		channel_t channel;
		double weight;
	};

	//����ͼ��Ĳ���Ȩ��
	void calculateWeight(cv::Mat img_channels[3], double weight[3]);

	//ת��ͼ���ɫ�ʿռ��ɫ�����
	void convertColor(cv::Mat& input, cv::Mat& output);
};
