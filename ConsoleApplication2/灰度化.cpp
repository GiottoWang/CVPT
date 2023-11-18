#include "imgprog.hpp"
#include <opencv.hpp>
using namespace ip;
int main(int argc, char* argv[]) {
	using std::cout;
	using std::endl;
	cv::Mat img = cv::imread(argv[1], -1);//argv从命令行读取字符串，可以在属性->调试->命令参数中设置
	if (img.empty())return -1;
	cv::Mat img_gray,img_hsv,img_hls;
	cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
	cv::cvtColor(img, img_hsv, cv::COLOR_BGR2HSV);
	cv::cvtColor(img, img_hls, cv::COLOR_BGR2HLS);
	show(img, std::string(argv[1]), re_Title);
	show(img_gray, "GRAY");
	show(img_hsv, "HSV");
	show(img_hls, "HLS");

}