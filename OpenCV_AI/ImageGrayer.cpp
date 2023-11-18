#include "ImageGrayer.h"
// 构造函数，接受一个彩色图像的文件名作为参数，读取并保存图像
ImageGrayer::ImageGrayer(string filename)
{
	src = imread(filename);
	if (src.empty())
	{
		// 抛出一个异常，表示无法读取图像文件
		throw runtime_error("Cannot read image file.");
	}
	// 默认使用浮点算法
	algorithm = FLOAT;
}

// 析构函数，释放图像资源
ImageGrayer::~ImageGrayer()
{
	src.release();
	dst.release();
}

// 设置当前的算法选择，接受一个 Algorithm 类型的参数
void ImageGrayer::setAlgorithm(Algorithm alg)
{
	algorithm = alg;
}

// 获取当前的算法选择，返回一个 Algorithm 类型的值
ImageGrayer::Algorithm ImageGrayer::getAlgorithm()
{
	return algorithm;
}

// 根据当前的算法选择实现彩色图像的灰度化，返回一个灰度图像
Mat ImageGrayer::gray()
{
	// 创建一个和原始图像大小相同的灰度图像
	dst = Mat(src.rows, src.cols, CV_8UC1);

	// 遍历每个像素，计算灰度值
	for (int i = 0; i < src.rows; i++)
	{

		//uchar *src_data = src.ptr<uchar>(i);// 获取原始图像的第 i 行的像素指针

		//uchar *dst_data = dst.ptr<uchar>(i);// 获取灰度图像的第 i 行的像素指针

		for (int j = 0; j < src.cols; j++)

		{
			auto *src_data = src.ptr<Vec3b>(i, j);
			auto *dst_data = dst.ptr<uchar>(i, j);

			//int src_offset = j * 3;//计算原始图像的第 i 行第 j 列的像素的偏移量

			//int dst_offset = j;// 计算灰度图像的第 i 行第 j 列的像素的偏移量

			// 获取原始图像的每个像素的三个颜色分量
			uchar b = src_data->val[0];// src_data[src_offset];
			uchar g = src_data->val[1];// src_data[src_offset + 1];
			uchar r = src_data->val[2];// src_data[src_offset + 2];

			// 根据当前的算法选择计算灰度值
			int gray = 0;
			switch (algorithm)
			{
			case FLOAT: // 浮点算法
				gray = 0.299 * r + 0.587 * g + 0.114 * b;
				break;
			case INT: // 整数算法
				gray = (r * 30 + g * 59 + b * 11) / 100;
				break;
			case SHIFT: // 移位算法
				gray = (r * 28 + g * 151 + b * 77) >> 8;
				break;
			default:
				// 抛出一个异常，表示无效的算法选择
				throw invalid_argument("Invalid algorithm choice.");
			}

			// 将灰度值赋给灰度图像的对应像素
			*dst_data = gray;
		}
	}

	// 返回灰度图像
	return dst;
}
