#include "preprocessing.h"
#include <algorithm>
const double preprocessing::templateChannelWeight[3] = { 0.11, 0.3, 0.59 };
cv::Mat preprocessing::rgb2gray() {
	cv::Mat output = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);

	cv::Mat img_channels[3];//0->B,1->G,2->R
	cv::split(img, img_channels);

	double proportion[3];//0->B,1->G,2->R
	auto &proportion_B = proportion[0];
	auto &proportion_G = proportion[1];
	auto &proportion_R = proportion[2];

	int count[3];//0->B,1->G,2->R
	auto &count_B = count[0];
	auto &count_G = count[1];
	auto &count_R = count[2];
	std::map<char, int> w;

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

		}
	}
	(count_G >= count_R) ? count_G *= 2 : count_R *= 2;
	k_gray[0].num =proportion_B;
	k_gray[0].channel_gray = 'b';
	k_gray[1].num =proportion_G;
	k_gray[1].channel_gray = 'g';
	k_gray[2].num = proportion_R;
	k_gray[2].channel_gray = 'r';
	if (count_G + count_R >= count_B / 5)
	{
	    (count_G >= count_R) ? count_R *= 2 : count_G *= 2;
		b = (count_B - count_G - count_R) / 4;
		g = proportion_G + (proportion_B - b) / 2;
		r = proportion_R + (proportion_B - b) / 2;
		//fan_switch = false;
	}
	else
	{
		b =proportion_B;
		g =proportion_G;
		r = proportion_R;
	}
	b1 = float(b) / float(b + g + r);
	g1 = float(g) / float(b + g + r);
	r1 = float(r) / float(b + g + r);
	qsort(k_gray, 3, sizeof(k_gray[0]), cmp_channel);//Ωµ–Ú≈≈¡–
	for (int t = 0; t < 3; t++)
	{
		if (k_gray[t].channel_gray == 'b')
		{
			if (G1 + R1 >= B1 / 5)
				b0 = a[t];
			else
				b0 = a[2 - t];
		}
		if (k_gray[t].channel_gray == 'g')
		{
			if (G1 + R1 >= B1 / 5)
				g0 = a[t];
			else
				g0 = a[2 - t];
		}
		if (k_gray[t].channel_gray == 'r')
		{
			if (G1 + R1 >= B1 / 5)
				r0 = a[t];
			else
				r0 = a[2 - t];
		}
	}

}