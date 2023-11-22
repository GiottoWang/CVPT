#include "preprocessing.h"
//将RGB图像转换为灰度图像的函数实现
cv::Mat preprocessing::process() {
	convertColor(); //转换
	deNoise();
	return this->data; //返回
}

void preprocessing::deNoise() {
	cv::Mat result = data.clone();
	cv::Mat mark = cv::Mat::zeros(data.rows, data.cols, CV_8UC1);  //区域生长标记
	std::vector<spot_t> spot;
	int num = 0;
	int around;
	std::vector<cv::Point2f> seeds;
	cv::Point2f seed;
	cv::Point2f seed_wait; //待生长种子点
	constexpr float direct[9][2] = { {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {0,0} };  //9邻域
	int flag;
	for (int i = 0; i < result.rows; i++)
	{
		for (int j = 0; j < result.cols; j++)
		{
			if (result.at<uchar>(i, j) != 0 && mark.at<uchar>(i, j) == 0)
			{
				seed.x = float(i);
				seed.y = float(j);
				seeds.emplace_back(seed);
			}
			while (!seeds.empty())
			{
				seed = seeds.back();     //取出最后一个元素
				seeds.pop_back();         //删除栈中最后一个元素,防止重复扫描
				around = 8;			//以8邻域去除孤立点
				for (int k = 0; k < 8; k++)    //遍历种子点的8邻域
				{
					seed_wait.x = seed.x + direct[k][0];    //第i个坐标0行，即x坐标值
					seed_wait.y = seed.y + direct[k][1];    //第i个坐标1行，即y坐标值
					if (seed_wait.x < 0 || seed_wait.y < 0 ||
						(seed_wait.x >= data.rows) || (seed_wait.y >= data.cols))
					{
						around--;
						continue;
					}

					flag = mark.at<uchar>(seed_wait.x, seed_wait.y);
					if (flag == 0)
					{
						if (data.at<uchar>(seed_wait.x, seed_wait.y) != 0)
						{
							num++;
							mark.at<uchar>(seed_wait.x, seed_wait.y) = 255; //flag=255表示该点已经是区域生长中的点，不再做判断
							seeds.emplace_back(seed_wait);    //将满足生长条件的待生长种子点放入种子栈中
						}
						else                            //******************/
						{								//*			       */
							around--;					//*	 去除单个孤立点   */
						}								//*			       */
						if (around == 0)				//*			       */
						{								//*			       */
							result.at<uchar>(i, j) = 0;	//******************/
						}
					}
				}
				if (seeds.empty() && num != 0)
				{
					spot.emplace_back(spot_t{ i, j, num });
				}
			}
			num = 0;
		}
	}
	auto cmp = [](spot_t a, spot_t b)-> bool {
		// 如果a大于b，返回true，表示a应该排在b前面，即降序
		return a.num > b.num;
	};
	std::sort(spot.begin(), spot.end(), cmp);
	mark = 0;
	for (int n = 2; n < spot.size(); n++)
	{
		if (spot[n].num < spot[0].num * THRESHOLD_NOISE)//光斑去不干净可以将0.03加大
		{
			seed.x = spot[n].x;
			seed.y = spot[n].y;
			seeds.emplace_back(seed);
		}
		while (!seeds.empty())
		{
			seed = seeds.back();     //取出最后一个元素
			seeds.pop_back();         //删除栈中最后一个元素,防止重复扫描
			for (int k = 0; k < 9; k++)    //遍历种子点的8邻域
			{
				seed_wait.x = seed.x + direct[k][0];    //第i个坐标0行，即x坐标值
				seed_wait.y = seed.y + direct[k][1];    //第i个坐标1行，即y坐标值
				if (seed_wait.x < 0 || seed_wait.y < 0 ||
					(seed_wait.x >= data.rows) || (seed_wait.y >= data.cols))
					continue;
				flag = mark.at<uchar>(seed_wait.x, seed_wait.y);
				if (flag == 0)
				{
					if (data.at<uchar>(seed_wait.x, seed_wait.y) != 0)
					{
						result.at<uchar>(seed_wait.x, seed_wait.y) = 0;
						mark.at<uchar>(seed_wait.x, seed_wait.y) = 255; //flag=1表示该点已经是区域生长中的点，不再做判断
						seeds.emplace_back(seed_wait);    //将满足生长条件的待生长种子点放入种子栈中
					}
				}
			}
		}
	}
	data = result.clone();
}

//转换图像的色彩空间和色彩深度的函数实现
void preprocessing::convertColor() {
	//检查图像的类型和通道数
	if (data.type() != CV_8UC3) {
		throw std::invalid_argument("The input image must be of type CV_8UC3");
	}

	//空灰度图像
	cv::Mat result = cv::Mat::zeros(data.rows, data.cols, CV_8UC1);

	//分离图像通道
	cv::Mat channel[3];//0->B,1->G,2->R
	cv::split(data, channel);

	//计算图像的补偿权重
	double weight[3] = { 0,0,0 };
	calculateWeight(channel, weight);

	//根据补偿权重计算灰度值
	//output = channel[B] * weight[B] + channel[G] * weight[G] + channel[R] * weight[R];
	for (int i = 0; i < data.rows; i++) {
		auto intensity_B = channel[B].ptr<uchar>(i);
		auto intensity_G = channel[G].ptr<uchar>(i);
		auto intensity_R = channel[R].ptr<uchar>(i);
		auto intensity = result.ptr<uchar>(i);
		for (int j = 0; j < data.cols; j++)
		{
			if (intensity_B[j] == 0 && intensity_G[j] == 0 && intensity_R[j] == 0)
				continue;
			intensity[j] = intensity_B[j] * weight[B] + intensity_G[j] * weight[G] + intensity_R[j] * weight[R];
		}
	}
	data = result.clone();
	result.release();
}

//计算图像补偿权重函数的实现
void preprocessing::calculateWeight(cv::Mat channel[3], double weight[3]) {
	//一些统计量数组
	count_t count[3];//三通道灰度占比统计量，eg count[B]为，图像整体B通道灰度占三通道灰度之和的比
	count[B] = { B,0 };
	count[G] = { G,0 };
	count[R] = { R,0 };
	int light[3] = { 0,0,0 };//亮通道统计量，eg light[0]为该点的B通道强度最高
	double compensation[3] = { 0,0,0 };//第一次补偿后的亮通道权重统计量
	double normalization[3] = { 0,0,0 };//归一化第一次补偿权重
	double templateWeight[3] = { 0.114,0.299,0.587 };//模板权重
	double re_tmpW[3] = { 0,0,0 };//重分配后的模板权重

	//统计原始亮通道个数和三通道灰度占比
	for (int i = 0; i < data.rows; i++)
	{	//临时指针，存放三通道的行指针
		auto intensity_B = channel[B].ptr<uchar>(i);
		auto intensity_G = channel[G].ptr<uchar>(i);
		auto intensity_R = channel[R].ptr<uchar>(i);
		for (int j = 0; j < data.cols; j++)
		{	//如果图像强度小于阈值则跳过统计
			if (intensity_B[j] <= THRESHOLD_WEIGHT && intensity_G[j] <= THRESHOLD_WEIGHT && intensity_R[j] <= THRESHOLD_WEIGHT)
				continue;

			uchar temp[3] = { intensity_B[j] ,intensity_G[j],intensity_R[j] };
			uchar maxElem = *std::max_element(temp, temp + 3);
			if (intensity_B[j] == maxElem)
				light[B]++;
			else if (intensity_G[j] == maxElem)
				light[G]++;
			else
				light[R]++;

			double weight_sum = double(intensity_B[j] + intensity_G[j] + intensity_R[j]);
			count[B].weight += double(intensity_B[j]) / weight_sum;
			count[G].weight += double(intensity_G[j]) / weight_sum;
			count[R].weight += double(intensity_R[j]) / weight_sum;
		}
	}

	//---------------------------------------------一次权重补偿------------------------------------------------//

	//对灰度进行一次补偿
	(light[G] >= light[R]) ? light[G] *= 2 : light[R] *= 2;

	//补偿后，若图像目标区域曝光较高
	if (light[G] + light[R] >= light[B] / 5)
	{
		//对灰度进行二次补偿
		(light[G] >= light[R]) ? light[R] *= 2 : light[G] *= 2;

		//第一次权重补偿：削弱蓝色通道权重，补偿其余两通道权重
		compensation[B] = (light[B] - light[G] - light[R]) / 4;
		compensation[G] = count[G].weight + (count[B].weight - compensation[B]) / 2;
		compensation[R] = count[R].weight + (count[B].weight - compensation[B]) / 2;
		//fan_switch = false;
	}
	//补偿后，若图像目标区域曝光较低
	else
	{
		//权重不变
		compensation[B] = count[B].weight;
		compensation[G] = count[G].weight;
		compensation[R] = count[R].weight;
	}

	//归一化第一次权重补偿
	double compensation_sum = std::accumulate(compensation, compensation + 3, 0);
	for (int i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}

	//---------------------------------------------二次权重补偿------------------------------------------------//
	//对count进行降序排列
	auto cmp = [](count_t a, count_t b)-> bool {
		// 如果a大于b，返回true，表示a应该排在b前面，即降序
		return a.weight > b.weight;
	};
	std::sort(count, count + 3, cmp);//降序排列

	//重新分配模板权重
	int i = 0;
	auto choose = [light, templateWeight, &i](double& a)->void {
		a = (light[G] + light[R] >= light[B] / 5) ? templateWeight[i] : templateWeight[2 - i];
	};
	for (; i < 3; i++)
	{
		switch (count[i].channel)
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