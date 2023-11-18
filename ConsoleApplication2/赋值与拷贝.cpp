#include "picstream.h"
#include<opencv.hpp>
#include<iostream>
bool negation3(cv::Mat& img);//彩色图像取反
int main(int argc, char** argv) {
	picstream os;
	cv::Mat img = cv::imread(argv[1], -1);
	cv::Mat img_neg = img.clone();
	//img_neg=img;该方式是将img_neg指向img的地址
	cv::Mat img0 = cv::Mat::zeros(255, 255, CV_8UC3);
	cv::Mat img1 = img0.clone();
	cv::Mat img2 = img0.clone();
	img1 = 255;//只改变第一个通道;
	img2 = cv::Scalar(0, 255, 255);//可以改变三个通道
	negation3(img_neg);
	os.stream.insert(picstream::elemType("初始图像", img));
	os.stream.insert(picstream::elemType("取反图像", img_neg));
	os.stream.insert(picstream::elemType("0蒙版", img0));
	os.stream.insert(picstream::elemType("0_1蒙版", img1));
	os.stream.insert(picstream::elemType("0_2蒙版", img2));
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