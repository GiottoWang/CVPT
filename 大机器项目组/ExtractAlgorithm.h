#include <opencv2/opencv.hpp>
#include <vector>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2\imgproc\types_c.h>

using namespace cv;
using namespace std;

class CExtractAlgorithm
{
public:
	Mat splitChannelExtractROI(Mat&srcImg,Mat &SelectChannelImage);
	Mat splitChannelExtractROI_1(Mat& srcImg, Mat& SelectChannelImage);
	void connectedComponentFilter_4(const cv::Mat& srcImage, cv::Mat& dstImage) ;

	void connectedComponentFilter_8(const cv::Mat& srcImage, cv::Mat& dstImage) ;

	void smooth(std::vector<cv::Point>& rawData,int smoothNum);

	//提取单线激光，差分法
	bool detectResult_sjw_vertical(const cv::Mat &srcImage,cv::Mat &outImage, int modelLength);
	

};