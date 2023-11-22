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

	//���캯��
	preprocessing() = default;
	preprocessing(cv::Mat a) :data(a) {};

	//��RGBͼ��ת��Ϊ�Ҷ�ͼ��ĺ���
	cv::Mat process();
	
private:
	//����һЩ���������ħ������
	constexpr static  channel_t B = 0;
	constexpr static  channel_t G = 1;
	constexpr static  channel_t R = 2;
	constexpr static  int THRESHOLD_WEIGHT = 20;
	constexpr static double THRESHOLD_NOISE = 0.03;

	//����ͼ��
	cv::Mat data;
	//���ͼ��
	cv::Mat output;

	//��ͨ���Ҷ�ռ��ͳ�����ṹ��
	struct count_t {
		channel_t channel;
		double weight;
	};
	struct spot_t {
		int x;
		int y;
		int num;
	};

	//����ͼ��Ĳ���Ȩ��
	void calculateWeight(cv::Mat img_channels[3], double weight[3]);

	//ת��ͼ���ɫ�ʿռ�
	void convertColor();

	//�ԻҶ�ͼ�����ȥ��
	void deNoise();

	//�Ե��ع�ͼ�����ȡ��
	void invertion();
};

