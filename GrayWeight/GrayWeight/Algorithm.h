#pragma once
#include "opencv2/opencv.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
using namespace cv;
struct GrayWeight
{
	char channel_gray;
	int num;
};
struct AreaGrowData
{
	int i;
	int j;
	int num;
	int i_min;
	int i_max;
	int j_min;
	int j_max;
	bool flag;
};
struct MyPoint
{
	int i;
	int j;
	int flag;

	int index_pre;
	MyPoint()
	{
		flag = -1;
	}
};
int cmp_channel(const void* a, const void* b)
{
	return ((struct GrayWeight*)b)->num - ((struct GrayWeight*)a)->num;
}
int cmp_flag(const void* a, const void* b)
{
	return ((struct AreaGrowData*)b)->num - ((struct AreaGrowData*)a)->num;
}
