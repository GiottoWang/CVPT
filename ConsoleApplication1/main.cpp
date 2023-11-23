#include "ImageProcessing.h"
using namespace std;
using namespace cv;

int main() {
	Mat img = imread("1.jpg");
	PA p=new PA_A(img);
	Preprocess q(p);
	Mat gray = q.execute();
	namedWindow("1", WINDOW_NORMAL);
	imshow("1", gray);
	waitKey(0);
	delete p;
	return 0;
}
