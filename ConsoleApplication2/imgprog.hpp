#pragma once
//image processing

#include<iostream>
#include<regex>//正则表达式
#include<opencv.hpp>
#include <chrono>
namespace ip {
	using std::string;
	using std::regex;
	using cv::Mat;
	using namespace std::chrono;
	string re_Title(std::string title) {
		regex re("\\..+");//去文件后缀名
		string flag = regex_replace(title, re, "$1");
		return flag;
	}
	void show(Mat img, string title) {
		cv::namedWindow(title, cv::WINDOW_FREERATIO);
		cv::imshow(title, img);
		cv::waitKey(0);
		cv::destroyAllWindows();
	}
	void show(Mat img, string title, string(*re)(string title)) {
		std::string t = re(title);
		cv::namedWindow(t, cv::WINDOW_FREERATIO);
		cv::imshow(t, img);
		cv::waitKey(0);
		cv::destroyAllWindows();
	}
	void time_clock(void(*foo)()) {
		auto start = system_clock::now();
		foo();
		auto end = system_clock::now();
		auto duration = duration_cast<microseconds>(end - start);
		std::cout << "程序段" << foo << "的运行时间：" << double(duration.count())* microseconds::period::num / microseconds::period::den << std::endl;
	}

}