#include <iostream>
#include "ImageGrayer.h"
using namespace std;

int main()
{
	// 创建一个 ImageGrayer 对象，传入一个彩色图像的文件名
	ImageGrayer grayer("test.jpg");

	// 使用 try 语句来捕获可能发生的异常
	try
	{
		// 设置算法选择为浮点算法
		grayer.setAlgorithm(ImageGrayer::SHIFT);

		// 调用 gray 方法来实现彩色图像的灰度化
		Mat gray = grayer.gray();

		// 显示灰度图像
		imshow("Gray Image", gray);

		// 等待用户按键
		waitKey(0);
	}
	// 使用 catch 语句来处理不同类型的异常
	catch (runtime_error &e) // 处理无法读取图像文件的异常
	{
		// 打印异常信息
		cout << e.what() << endl;
	}
	catch (invalid_argument &e) // 处理无效的算法选择的异常
	{
		// 打印异常信息
		cout << e.what() << endl;
	}

	return 0;
}