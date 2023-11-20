#include "preprocessing.h"
using namespace std;
using namespace cv;
int main() {
	Mat input = imread("test.jpg");
	preprocessing p(input);
	Mat gray(p.bgr2gray());
	namedWindow("1", WINDOW_FREERATIO);//cv::WINDOW_NORMAL, cv::WINDOW_FREERATIO
	imshow("1", gray);
	waitKey(0);
}