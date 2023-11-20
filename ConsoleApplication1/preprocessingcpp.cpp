#include "preprocessing.h"
//将RGB图像转换为灰度图像的函数实现
cv::Mat preprocessing::bgr2gray() {
	cv::Mat output; //输出的灰度图像
	convertColor(input, output); //转换
	return output; //返回
}

//转换图像的色彩空间和色彩深度的函数实现
void preprocessing::convertColor(cv::Mat& input, cv::Mat& output) {
	//检查图像的类型和通道数
	if (input.type() != CV_8UC3) {
		throw std::invalid_argument("The input image must be of type CV_8UC3");
	}

	//空灰度图像
	output = cv::Mat::zeros(input.rows, input.cols, CV_8UC1);

	//分离图像通道
	cv::Mat channel[3];//0->B,1->G,2->R
	cv::split(input, channel);

	//计算图像的补偿权重
	double weight[3] = { 0,0,0 };
	calculateWeight(channel, weight);

	//根据补偿权重计算灰度值
	//output = channel[B] * weight[B] + channel[G] * weight[G] + channel[R] * weight[R];
	for (int i = 0; i < input.rows; i++) {
		auto intensity_B = channel[B].ptr<uchar>(i);
		auto intensity_G = channel[G].ptr<uchar>(i);
		auto intensity_R = channel[R].ptr<uchar>(i);
		auto intensity = output.ptr<uchar>(i);
		for (int j = 0; j < input.cols; j++)
		{
			if (intensity_B[j] == 0 && intensity_G[j] == 0 && intensity_R[j] == 0)
				continue;
			intensity[j] = intensity_B[j] * weight[B] + intensity_G[j] * weight[G] + intensity_R[j] * weight[R];
		}
	}
}

//计算图像补偿权重函数的实现
void preprocessing::calculateWeight(cv::Mat channel[3], double weight[3]) {
	//一些统计量数组
	proportion_t proportion[3];//三通道灰度占比统计量，eg proportion[B]为，图像整体B通道灰度占三通道灰度之和的比
	proportion[B] = { B,0 };
	proportion[G] = { G,0 };
	proportion[R] = { R,0 };
	int count[3] = { 0,0,0 };//亮通道统计量，eg count[0]为该点的B通道强度最高
	double compensation[3] = { 0,0,0 };//第一次补偿后的权重
	double normalization[3] = { 0,0,0 };//归一化第一次补偿权重
	double templateWeight[3] = { 0.114,0.299,0.587 };//模板权重
	double re_tmpW[3] = { 0,0,0 };//重分配后的模板权重

	//统计原始亮通道个数和三通道灰度占比
	for (int i = 0; i < input.rows; i++)
	{	//临时指针，存放三通道的行指针
		auto intensity_B = channel[B].ptr<uchar>(i);
		auto intensity_G = channel[G].ptr<uchar>(i);
		auto intensity_R = channel[R].ptr<uchar>(i);
		for (int j = 0; j < input.cols; j++)
		{	//如果图像强度小于阈值则跳过统计
			if (intensity_B[j] <= THRESHOLD_WEIGHT && intensity_G[j] <= THRESHOLD_WEIGHT && intensity_R[j] <= THRESHOLD_WEIGHT)
				continue;

			uchar temp[3] = { intensity_B[j] ,intensity_G[j],intensity_R[j] };
			uchar maxElem = *std::max_element(temp, temp + 3);
			if (intensity_B[j] == maxElem)
				count[B]++;
			else if (intensity_G[j] == maxElem)
				count[G]++;
			else
				count[R]++;

			double weight_sum = double(intensity_B[j] + intensity_G[j] + intensity_R[j]);
			proportion[B].weight += double(intensity_B[j]) / weight_sum;
			proportion[G].weight += double(intensity_G[j]) / weight_sum;
			proportion[R].weight += double(intensity_R[j]) / weight_sum;
		}
	}

	//---------------------------------------------一次权重补偿------------------------------------------------//

	//对灰度进行一次补偿
	(count[G] >= count[R]) ? count[G] *= 2 : count[R] *= 2;

	//补偿后，若图像目标区域曝光较高
	if (count[G] + count[R] >= count[B] / 5)
	{
		//对灰度进行二次补偿
		(count[G] >= count[R]) ? count[R] *= 2 : count[G] *= 2;

		//第一次权重补偿：削弱蓝色通道权重，补偿其余两通道权重
		compensation[B] = (count[B] - count[G] - count[R]) / 4;
		compensation[G] = proportion[G].weight + (proportion[B].weight - compensation[B]) / 2;
		compensation[R] = proportion[R].weight + (proportion[B].weight - compensation[B]) / 2;
		//fan_switch = false;
	}
	//补偿后，若图像目标区域曝光较低
	else
	{
		//权重不变
		compensation[B] = proportion[B].weight;
		compensation[G] = proportion[G].weight;
		compensation[R] = proportion[R].weight;
	}

	//归一化第一次权重补偿
	double compensation_sum = std::accumulate(compensation, compensation + 3, 0);
	for (int i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}

	//---------------------------------------------二次权重补偿------------------------------------------------//
	//对proportion进行降序排列
	auto cmp = [](proportion_t a, proportion_t b)-> bool {
		// 如果a大于b，返回true，表示a应该排在b前面，即降序
		return a.weight > b.weight;
	};
	std::sort(proportion, proportion + 3, cmp);//降序排列

	//重新分配模板权重
	int i = 0;
	auto choose = [count, templateWeight, &i](double& a)->void {
		a = (count[G] + count[R] >= count[B] / 5) ? templateWeight[i] : templateWeight[2 - i];
	};
	for (; i < 3; i++)
	{
		switch (proportion[i].channel)
		{
		case B:
			choose(re_tmpW[B]);
			break;
		case G:
			choose(re_tmpW[G]);
			break;
		case R:
			choose(re_tmpW[R]);
		default:
			break;
		}
	}

	//二次权重补偿
	weight[B] = (re_tmpW[B] + normalization[B]) / 2;
	weight[G] = (re_tmpW[G] + normalization[G]) / 2;
	weight[R] = (re_tmpW[R] + normalization[R]) / 2;
}