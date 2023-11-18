#include "picstream.h"
#include <chrono>
#include<regex>//正则表达式
using namespace std::chrono;
void picstream::show() {
	for(auto it=stream.begin();it!=stream.end();it++){
		cv::namedWindow(it->first, cv::WINDOW_FREERATIO);//cv::WINDOW_NORMAL, cv::WINDOW_FREERATIO
		cv::imshow(it->first, it->second);
	}
		cv::waitKey(0);
		cv::destroyAllWindows();
}
void picstream::time_clock(void(*foo)()) {
	auto start = system_clock::now();
	foo();
	auto end = system_clock::now();
	auto duration = duration_cast<microseconds>(end - start);
	std::cout << "程序段" << foo << "的运行时间：" << double(duration.count())* microseconds::period::num / microseconds::period::den << std::endl;
}

std::string re_Title(std::string title) {
	std::regex re("\\..+");//去文件后缀名
	std::string flag = regex_replace(title, re, "$1");
	return flag;
}