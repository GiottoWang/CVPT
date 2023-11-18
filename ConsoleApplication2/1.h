#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// 定义一个 ImageGrayer 类，用于实现彩色图像的灰度化
class ImageGrayer
{
public:
	// 定义一个 enum 类型，表示三种灰度化的算法
	enum Algorithm
	{
		FLOAT, // 浮点算法
		INT,   // 整数算法
		SHIFT  // 移位算法
	};
	ImageGrayer(string filename);
	~ImageGrayer();

	// 设置当前的算法选择，接受一个 Algorithm 类型的参数
	void setAlgorithm(Algorithm alg);

	// 获取当前的算法选择，返回一个 Algorithm 类型的值
	Algorithm getAlgorithm();

	// 根据当前的算法选择实现彩色图像的灰度化，返回一个灰度图像
	Mat gray();

private:
	// 原始彩色图像
	Mat src;
	// 灰度图像
	Mat dst;
	// 算法选择
	Algorithm algorithm;
};
