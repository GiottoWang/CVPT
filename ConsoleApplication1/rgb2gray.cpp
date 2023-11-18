#include "opencv2/opencv.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

using namespace cv;
int Gray_Threshold;//背景去噪阈值
bool fan_switch = true;//是否取反标志量
struct GrayWeight
{
	char channel_gray;
	int num;
};
int cmp_channel(const void* a, const void* b)
{
	return ((struct GrayWeight*)b)->num - ((struct GrayWeight*)a)->num;
}
bool rgb2gray(const Mat & srcImage, Mat &outImage)
{
	float a[3] = { 0.114,0.299,0.587 };
	float b0, g0, r0;
	float b2, g2, r2;
	float b = 0, g = 0, r = 0;
	float b1, g1, r1;
	float b3 = 0, g3 = 0, r3 = 0;
	Mat Image = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	Mat channel[3];
	Mat B_gray, G_gray, R_gray;
	GrayWeight k_gray[3];
	split(srcImage, channel);
	B_gray = channel[0];
	G_gray = channel[1];
	R_gray = channel[2];
	int B1 = 0, G1 = 0, R1 = 0;
	float B2 = 0, G2 = 0, R2 = 0;
	for (int i = 0; i < srcImage.rows; i++)
	{
		for (int j = 0; j < srcImage.cols; j++)
		{
			if (B_gray.at<uchar>(i, j) <= 20 && G_gray.at<uchar>(i, j) <= 20 && R_gray.at<uchar>(i, j) <= 20)
				continue;
			int B = B_gray.at<uchar>(i, j);
			int G = G_gray.at<uchar>(i, j);
			int R = R_gray.at<uchar>(i, j);

			if (B >= (G >= R ? G : R))
				B1++;
			if (G >= (B >= R ? B : R))
				G1++;
			if (R >= (B >= G ? B : G))
				R1++;

			b3 += float(B) / float(B + G + R);
			g3 += float(G) / float(B + G + R);
			r3 += float(R) / float(B + G + R);

		}
	}
	if (G1 >= R1)
		G1 *= 2;
	else
		R1 *= 2;
	k_gray[0].num = b3;
	k_gray[0].channel_gray = 'b';
	k_gray[1].num = g3;
	k_gray[1].channel_gray = 'g';
	k_gray[2].num = r3;
	k_gray[2].channel_gray = 'r';
	if (G1 + R1 >= B1 / 5)
	{

		if (G1 >= R1)
			R1 *= 2;
		else
			G1 *= 2;
		b = (B1 - G1 - R1) / 4;
		g = g3 + (b3 - b) / 2;
		r = r3 + (b3 - b) / 2;
		fan_switch = false;
	}
	else
	{
		b = b3;
		g = g3;
		r = r3;
	}
	b1 = float(b) / float(b + g + r);
	g1 = float(g) / float(b + g + r);
	r1 = float(r) / float(b + g + r);
	qsort(k_gray, 3, sizeof(k_gray[0]), cmp_channel);//降序排列
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

	b2 = (b1 + b0) / 2;
	g2 = (g1 + g0) / 2;
	r2 = (r1 + r0) / 2;
	//Mat Weight = (Mat_<float>(1, 3) << b2, g2, r2);
	for (int i = 0; i < srcImage.rows; i++)
	{
		for (int j = 0; j < srcImage.cols; j++)
		{
			if (B_gray.at<uchar>(i, j) == 0 && G_gray.at<uchar>(i, j) == 0 && R_gray.at<uchar>(i, j) == 0)
				continue;
			Image.at<uchar>(i, j) = B_gray.at<uchar>(i, j)*b2 + G_gray.at<uchar>(i, j)*g2 + R_gray.at<uchar>(i, j)*r2;
		}
	}

	if (G1 + R1 >= B1 / 5)
		Gray_Threshold = 255 * b2 / 3;
	else
		Gray_Threshold = 255 * (1 - b2) / 3;
	outImage = Image.clone();
	Image.release();
	return false;
}