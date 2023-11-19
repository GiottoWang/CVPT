#include "preprocessing.h"
#include <algorithm>
#include <numeric>
const double preprocessing::templateChannelWeight[3] = { 0.114,0.299,0.587 };

void preprocessing::calculateWeightAndCompensation(cv::Mat img_channels[3], double weight[3], double compensation[3]) {
	//三通道灰度占比统计量，eg proportion['B']为，图像整体B通道灰度占三通道灰度之和的比
	std::map<_channel_t, double> proportion;
	proportion.insert(std::pair<_channel_t, double>{B, 0});
	proportion.insert(std::pair<_channel_t, double>{G, 0});
	proportion.insert(std::pair<_channel_t, double>{R, 0});
	auto &proportion_B = proportion[B];
	auto &proportion_G = proportion[G];
	auto &proportion_R = proportion[R];

	//亮通道统计量，eg count[0]为该点的B通道强度最高
	int count[3];//0->B,1->G,2->R
	auto &count_B = count[B];
	auto &count_G = count[G];
	auto &count_R = count[R];

	//归一化
	double normalization[3];
	auto &normalized_B = normalization[B];
	auto &normalized_G = normalization[G];
	auto &normalized_R = normalization[R];

	//
	double re_tmpcw[3];
	auto &re_tmpcw_B = re_tmpcw[B];
	auto &re_tmpcw_G = re_tmpcw[G];
	auto &re_tmpcw_R = re_tmpcw[R];

	//统计原始亮通道个数和三通道灰度占比
	//使用一个单层循环来遍历图像的所有像素，减少时间复杂度
	//使用ptr方法来访问图像的像素值，提高代码的效率和性能
	for (int i = 0; i < img.rows * img.cols; i++)
	{
		auto intensity_B = img_channels[B].ptr<uchar>(i);
		auto intensity_G = img_channels[G].ptr<uchar>(i);
		auto intensity_R = img_channels[R].ptr<uchar>(i);

		if (*intensity_B <= THRESHOLD_WEIGHT && *intensity_G <= THRESHOLD_WEIGHT && *intensity_R <= THRESHOLD_WEIGHT)
			continue;

		uchar temp[3] = { *intensity_B ,*intensity_G,*intensity_R };
		uchar maxElem = *std::max_element(temp, temp + 3);
		if (*intensity_B == maxElem)
			count_B++;
		else if (*intensity_G == maxElem)
			count_G++;
		else
			count_R++;
		double weight_sum = double(*intensity_B + *intensity_G + *intensity_R);
		proportion_B += double(*intensity_B) / weight_sum;
		proportion_G += double(*intensity_G) / weight_sum;
		proportion_R += double(*intensity_R) / weight_sum;

	}//for

	//对灰度值进行一次补偿
	(count_G >= count_R) ? count_G *= 2 : count_R *= 2;

	//补偿后，若图像目标区域曝光较高
	if (count_G + count_R >= count_B / 5)
	{
		//对灰度进行二次补偿
		(count_G >= count_R) ? count_R *= 2 : count_G *= 2;

		//削弱蓝色通道权重，补偿其余两通道权重
		compensation[B] = (count_B - count_G - count_R) / 4;
		compensation[G] = proportion_G + (proportion_B - compensation[B]) / 2;
		compensation[R] = proportion_R + (proportion_B - compensation[B]) / 2;
		//fan_switch = false;
	}
	//补偿后，若图像目标区域曝光较低
	else
	{
		//权重不变
		compensation[B] = proportion_B;
		compensation[G] = proportion_G;
		compensation[R] = proportion_R;
	}

	//compensation归一化
	double compensation_sum = std::accumulate(compensation, compensation + 3, 0);
	for (auto i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}

	auto cmp = [](std::pair<_channel_t, double> a, std::pair<_channel_t, double> b)-> bool {
		// 如果a大于b，返回true，表示a应该排在b前面，即降序
		return a.second > b.second;
	};
	std::sort(proportion.begin(), proportion.end(), cmp);//降序排列

	int i = 0;
	auto choose = [count, i](double& a)->void {
		a = (count[1] + count[2] >= count[0] / 5) ? preprocessing::templateChannelWeight[i] : preprocessing::templateChannelWeight[2 - i];
	};

	for (auto t = proportion.begin(); t != proportion.end(); t++, i++)
	{
		if (t->first == B)
			choose(re_tmpcw_B);
		else if (t->first == G)
			choose(re_tmpcw_G);
		else
			choose(re_tmpcw_R);

	}
	weight[B] = (re_tmpcw_B + normalized_B) / 2;
	weight[G] = (re_tmpcw_G + normalized_G) / 2;
	weight[R] = (re_tmpcw_R + normalized_R) / 2;
}

//转换图像的色彩空间和色彩深度的函数实现
void preprocessing::convertColorSpaceAndDepth(cv::Mat& img, cv::Mat& output) {
	//检查图像的类型和通道数
	if (img.type() != CV_8UC3) {
		throw std::invalid_argument("The input image must be of type CV_8UC3");
	}

	//创建一个空的灰度图像
	output = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);

	//分离图像的三个通道
	cv::Mat channels[3];//0->B,1->G,2->R
	cv::split(img, channels);

	//计算图像的权重和补偿
	double weight[3];
	double compensation[3];
	calculateWeightAndCompensation(channels, weight, compensation);

	//根据权重和补偿计算灰度值
	//使用一个单层循环来遍历图像的所有像素，减少时间复杂度
	//使用ptr方法来访问图像的像素值，提高代码的效率和性能
	for (int i = 0; i < img.rows * img.cols; i++)
	{
		uchar *intensity_B = channels[B].ptr<uchar>(i);
		auto intensity_G = channels[G].ptr<uchar>(i);
		auto intensity_R = channels[R].ptr<uchar>(i);
		
		if (*intensity_B == 0 && *intensity_G == 0 && *intensity_R == 0)
			continue;
		*output.ptr<uchar>(i) = 
			*intensity_B * weight[B] + *intensity_G * weight[G] + *intensity_R * weight[R];
	}
}

//将RGB图像转换为灰度图像的函数实现
cv::Mat preprocessing::bgr2Gray() {
	cv::Mat output; //输出的灰度图像
	convertColorSpaceAndDepth(img, output); //转换
	return output; //返回
}














cv::Mat preprocessing::bgr2Gray() {
	cv::Mat output = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);

	cv::Mat img_channels[3];//0->B,1->G,2->R
	cv::split(img, img_channels);
	//三通道灰度占比统计量，eg proportion_B为，图像整体B通道灰度占三通道灰度之和的比
	std::map<char, double> proportion;
	proportion.insert(std::pair<char, double>{'B', 0});
	proportion.insert(std::pair<char, double>{'G', 0});
	proportion.insert(std::pair<char, double>{'R', 0});
	auto &proportion_B = proportion['B'];
	auto &proportion_G = proportion['G'];
	auto &proportion_R = proportion['R'];
	//亮通道统计量，eg count_B为该点的B通道强度最高
	int count[3];//0->B,1->G,2->R
	auto &count_B = count[0];
	auto &count_G = count[1];
	auto &count_R = count[2];
	//
	double compensation[3];
	auto &compensation_B = compensation[0];
	auto &compensation_G = compensation[1];
	auto &compensation_R = compensation[2];
	//
	double normalization[3];
	auto &normalized_B= normalization[0];
	auto &normalized_G= normalization[1];
	auto &normalized_R= normalization[2];
	//
	double re_tmpcw[3];
	auto &re_tmpcw_b = re_tmpcw[0];
	auto &re_tmpcw_g = re_tmpcw[1];
	auto &re_tmpcw_r = re_tmpcw[2];
	//////////////////////////////////////////////////////////////////
	//统计原始亮通道个数和三通道灰度占比
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			auto &weight_B = img_channels[0].at<uchar>(i, j);
			auto &weight_G= img_channels[1].at<uchar>(i, j);
			auto &weight_R= img_channels[2].at<uchar>(i, j);

			if (weight_B <= 20 && weight_G <= 20 && weight_R <= 20)
				continue;

			uchar temp[3] = { weight_B ,weight_G,weight_R };
			uchar maxElem = *std::max_element(temp, temp + 3);
			if (weight_B == maxElem)
				count_B++;
			else if (weight_G == maxElem)
				count_G++;
			else
				count_R++;
			double weight_sum = double(weight_B + weight_G + weight_R);
			proportion_B += double(weight_B) / weight_sum;
			proportion_G += double(weight_G) / weight_sum;
			proportion_R += double(weight_R) / weight_sum;

		}//for
	}//for
	//////////////////////////////////////////////////////////////////

	//对灰度值进行一次补偿
	(count_G >= count_R) ? count_G *= 2 : count_R *= 2;
	//补偿后，若图像目标区域曝光较高
	if (count_G + count_R >= count_B / 5)
	{	//对灰度进行二次补偿
	    (count_G >= count_R) ? count_R *= 2 : count_G *= 2;
		//削弱蓝色通道权重，补偿其余两通道权重
		compensation_B = (count_B - count_G - count_R) / 4;
		compensation_G = proportion_G + (proportion_B - compensation_B) / 2;
		compensation_R = proportion_R + (proportion_B - compensation_B) / 2;
		//fan_switch = false;
	}
	//补偿后，若图像目标区域曝光较低
	else
	{	//权重不变
		compensation_B =proportion_B;
		compensation_G =proportion_G;
		compensation_R = proportion_R;
	}
	//compensation归一化
	double compensation_sum=std::accumulate(compensation, compensation + 3, 0);
	for (auto i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}
	///////////////////////////////////////////////////////////////////

	auto cmp = [](std::pair<char, double> a, std::pair<char, double> b)-> bool{
		// 如果a大于b，返回true，表示a应该排在b前面，即降序
		return a.second > b.second;
	};
	std::sort(proportion.begin(), proportion.end(), cmp);//降序排列
	int i = 0;

	auto choose = [count,i](double &a)->void{
		a = (count[1] + count[2] >= count[0] / 5) ? preprocessing::templateChannelWeight[i]: preprocessing::templateChannelWeight[2 - i];
	};

	for (auto t = proportion.begin(); t != proportion.end(); t++,i++)
	{
		if (t->first == 'B')
			choose(re_tmpcw_b);
		else if (t->first == 'G')
			choose(re_tmpcw_g);
		else
			choose(re_tmpcw_r);
		
	}
	double finalchannels[3];
	finalchannels[0] = (re_tmpcw_b + normalized_B) / 2;
	finalchannels[1] = (re_tmpcw_g + normalized_G) / 2;
	finalchannels[2] = (re_tmpcw_r + normalized_R) / 2;
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			auto &weight_B = img_channels[0].at<uchar>(i, j);
			auto &weight_G = img_channels[1].at<uchar>(i, j);
			auto &weight_R = img_channels[2].at<uchar>(i, j);
			if (weight_B == 0 && weight_G == 0 && weight_R == 0)
				continue;
			output.at<uchar>(i, j) = weight_B * finalchannels[0] + weight_G * finalchannels[1] + weight_R * finalchannels[2];
		}
	}
	return output;
}