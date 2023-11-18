#include "picstream.h"
#include<opencv.hpp>
#include<iostream>
bool negation3(cv::Mat& img);//��ɫͼ��ȡ��
int main(int argc, char** argv) {
	picstream os;
	cv::Mat img = cv::imread(argv[1], -1);
	cv::Mat img_neg = img.clone();
	//img_neg=img;�÷�ʽ�ǽ�img_negָ��img�ĵ�ַ
	cv::Mat img0 = cv::Mat::zeros(255, 255, CV_8UC3);
	cv::Mat img1 = img0.clone();
	cv::Mat img2 = img0.clone();
	img1 = 255;//ֻ�ı��һ��ͨ��;
	img2 = cv::Scalar(0, 255, 255);//���Ըı�����ͨ��
	negation3(img_neg);
	os.stream.insert(picstream::elemType("��ʼͼ��", img));
	os.stream.insert(picstream::elemType("ȡ��ͼ��", img_neg));
	os.stream.insert(picstream::elemType("0�ɰ�", img0));
	os.stream.insert(picstream::elemType("0_1�ɰ�", img1));
	os.stream.insert(picstream::elemType("0_2�ɰ�", img2));
	os.show();

}

bool negation3(cv::Mat& img) {
	for(auto i=0;i<img.rows;i++)
		for (auto j = 0; j < img.cols; j++)
		{
		auto *p = img.ptr<cv::Vec3b>(i,j);
			p->val[0] = 255 - p->val[0];
			p->val[1] = 255 - p->val[1];
			p->val[2] = 255 - p->val[2];
		}
	return 1;
}