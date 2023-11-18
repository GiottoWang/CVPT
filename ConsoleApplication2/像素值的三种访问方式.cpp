//#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include"imgprog.hpp"
using namespace ip;

void foo1() {
	//cv::Mat _img(2048,816,CV_8UC3,cv::Scalar(0,0,0));
	cv::Mat _img(2048, 816, CV_8UC1);
	for (auto i = 0; i < _img.rows; i++)
	{
		for (auto j = 0; j < _img.cols; j++){
			//std::cout << "row: " << i << std::endl;
			//std::cout << "col: " << j << std::endl;
			 //auto *v3=_img.ptr<cv::Vec3b>(i,j);//or_img.ptr<cv::Vec3b>(i)[j]
			 //v3->val[0] = 255;
			 //v3->val[1] = 255;
			 //v3->val[2] = 255;
			*(_img.ptr<uchar>(i, j))=255;
		}

	}
}

void foo2() {
	cv::Mat _img(2048, 816, CV_8UC3, cv::Scalar(0, 0, 0));
	for (auto it = _img.begin<cv::Vec3b>(); it != _img.end<cv::Vec3b>(); it++) {	
		(*it).val[0] = 255;
		(*it).val[1] = 255;
		(*it).val[2] = 255;
	}

	//std::cout << (int)_img.at<uchar>(0,0)<<std::endl;

	}

void foo3() {
	//cv::Mat _img(2048, 816, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Mat _img(2048, 816, CV_8UC1);
	for (auto i = 0; i < _img.rows; i++)
	{
		for (auto j = 0; j < _img.cols; j++) {
			
			_img.at<uchar>(i, j) = 255;
			
			//std::cout << "row: " << i << std::endl;
			//std::cout << "col: " << j << std::endl;
			//std::cout << (int)_img.at<uchar>(i,j) << std::endl;

		}

	}
}

int main(int argc, char* argv[]) {
	time_clock(foo1);
	time_clock(foo2);
	time_clock(foo3);
	//foo1();
	//foo2();
	//foo3();

}
