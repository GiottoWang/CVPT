#include <iostream>
#include "Algorithm.h"
#include<stack>

#if _MSC_VER < 1600 // Visual Studio 2008 and older doesn't have stdint.h...
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
#else
#include <stdint.h>
#endif
using namespace cv;
using namespace std;

//int Gray_Threshold;//背景去噪阈值
bool fan_switch = true;//是否取反标志量
bool Gray_Weight_Line(const Mat &srcImage, Mat &outImage);
bool Gray_Weight(const Mat &srcImage, Mat &outImage); //灰度化
bool Area_Search(const Mat &srcImage, Mat &outImage);//去噪
bool Inversion(const Mat &srcImage, Mat &outImage);//优化低曝光图像
bool Center_Line(const Mat &srcImage, Mat &outImage,int modelLength);//提线
int main(int argc, char** argv)
{
	for (int i = 1; i <= 6; i++)
	{
		string str_in,str_out;
		//str_in = format("Desktop/123.jpg");
		str_in = format("D:\\vsWS\\大机器代码\\GrayWeight\\%d.jpg", i);
		Mat src = imread(str_in);
		Mat Weight_outimg = Mat::zeros(src.rows, src.cols, CV_8UC1);
		Gray_Weight_Line( src,Weight_outimg);
		//imwrite(str_out, Weight_outimg);
		src.release();
		Weight_outimg.release();
	}
}

bool Gray_Weight_Line(const Mat & srcImage, Mat & outImage)
{
	Mat Gray_Image = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	Mat Gray_Image_NoNoise = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	Mat Gray_Image_Final = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	Mat CenterLine = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	Gray_Weight(srcImage, Gray_Image);
	Area_Search(Gray_Image, Gray_Image_NoNoise);
	Inversion(Gray_Image_NoNoise, Gray_Image_Final);
	Center_Line(Gray_Image_Final, CenterLine,5);
	outImage = CenterLine.clone();
	Gray_Image.release();
	Gray_Image_NoNoise.release();
	Gray_Image_Final.release();
	CenterLine.release();
	return false;
}

bool Gray_Weight(const Mat & srcImage, Mat &outImage)
{
	float a[3] = { 0.11,0.3,0.59 };
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

	//if (G1 + R1 >= B1 / 5)
	//	Gray_Threshold = 255 * b2 / 3;
	//else
	//	Gray_Threshold = 255 * (1 - b2) / 3;
	outImage = Image.clone();
	Image.release();
	return false;
}

bool Area_Search(const Mat & srcImage, Mat & outImage)
{
	Mat Image_C = srcImage.clone();
	Mat flags = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);  //区域生长标记
	Mat flags_1 = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	Mat flags_2 = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	AreaGrowData* pot = new AreaGrowData[srcImage.rows * srcImage.cols]();
	int k_num = 0;
	int num = 0;
	int around;
	stack<Point2f> seeds;
	stack<Point2f> seeds_end;
	Point2f seed;
	Point2f waitSeed;    //待生长种子点
	float direct[8][2] = { {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} };  //定义8邻域
	//float direct_4[4][2] = { {0,-1} ,{-1,0} , {1,0} , {0,1} };//定义4邻域
	float direct_9[9][2] = { {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {0,0} };  //定义九宫格
	int flag, flag_1, flag_2;
	for (int i = 0; i < srcImage.rows; i++)
	{
		for (int j = 0; j < srcImage.cols; j++)
		{
			if (srcImage.at<uchar>(i, j) != 0 && flags.at<uchar>(i, j) == 0)
			{
				seed.x = float(i);
				seed.y = float(j);
				seeds.push(seed);
			}
			while (!seeds.empty())
			{
				Point2f seed = seeds.top();     //取出最后一个元素
				seeds.pop();         //删除栈中最后一个元素,防止重复扫描
				around = 8;			//以8邻域去除孤立点
				for (int ii = 0; ii < 8; ii++)    //遍历种子点的8邻域
				{
					waitSeed.x = seed.x + direct[ii][0];    //第i个坐标0行，即x坐标值
					waitSeed.y = seed.y + direct[ii][1];    //第i个坐标1行，即y坐标值
					if (waitSeed.x < 0 || waitSeed.y < 0 ||
						(waitSeed.x >= srcImage.rows) || (waitSeed.y >= srcImage.cols))
					{
						around--;
						continue;
					}

					flag = flags.at<uchar>(waitSeed.x, waitSeed.y);
					if (flag == 0)
					{
						if (srcImage.at<uchar>(waitSeed.x, waitSeed.y) != 0)
						{
							num++;
							flags.at<uchar>(waitSeed.x, waitSeed.y) = 255; //flag=255表示该点已经是区域生长中的点，不再做判断
							seeds.push(waitSeed);    //将满足生长条件的待生长种子点放入种子栈中
						}
						else                            //******************/
						{								//*			       */
							around--;					//*	去除单个孤立点 */
						}								//*			       */
						if (around == 0)				//*			       */
						{								//*			       */
							Image_C.at<uchar>(i, j) = 0;//******************/
						}
					}
				}
				if (seeds.empty() && num != 0)
				{
					pot[k_num].i = i;
					pot[k_num].j = j;
					pot[k_num].num = num;
					k_num++;
				}
			}
			num = 0;
		}
	}

	qsort(pot, k_num, sizeof(pot[0]), cmp_flag);
	int m = k_num;
	for (int n = 2; n < k_num; n++)
	{
		if (pot[n].num < pot[0].num * 0.03)//光斑去不干净可以将0.03加大
		{
			seed.x = pot[n].i;
			seed.y = pot[n].j;
			seeds_end.push(seed);
			m--;
		}
		while (!seeds_end.empty())
		{
			Point2f seed = seeds_end.top();     //取出最后一个元素
			//currentseed = srcImage.at<Vec3b>(seed);
			seeds_end.pop();         //删除栈中最后一个元素,防止重复扫描
			for (int ii = 0; ii < 9; ii++)    //遍历种子点的8邻域
			{
				waitSeed.x = seed.x + direct_9[ii][0];    //第i个坐标0行，即x坐标值
				waitSeed.y = seed.y + direct_9[ii][1];    //第i个坐标1行，即y坐标值
				if (waitSeed.x < 0 || waitSeed.y < 0 ||
					(waitSeed.x >= srcImage.rows) || (waitSeed.y >= srcImage.cols))
					continue;
				flag_2 = flags_2.at<uchar>(waitSeed.x, waitSeed.y);
				if (flag_2 == 0)
				{
					if (srcImage.at<uchar>(waitSeed.x, waitSeed.y) != 0)
					{
						Image_C.at<uchar>(waitSeed.x, waitSeed.y) = 0;
						flags_2.at<uchar>(waitSeed.x, waitSeed.y) = 255; //flag=1表示该点已经是区域生长中的点，不再做判断
						seeds_end.push(waitSeed);    //将满足生长条件的待生长种子点放入种子栈中
					}
				}
			}
		}

	}
	delete[] pot;
	outImage = Image_C.clone();
	return false;
}

bool Inversion(const Mat & srcImage, Mat & outImage)
{

	Mat Image_C = srcImage.clone();
	//Mat Image_Show = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
	Mat Image_Show = Image_C.clone();

	int num;
	int a[4] = { -2,-1,1,2 };
	int j_min, j_max;
	if (fan_switch == true)
	{
		AreaGrowData *k = new AreaGrowData[srcImage.rows]();
		for (int i = 0; i < Image_C.rows; i++)
		{
			j_min = Image_C.cols + 1;
			j_max = -1;
			for (int j = 0; j < Image_C.cols; j++)
			{
				if (srcImage.at<uchar>(i, j) == 0)
					continue;
				if (j_min > j)
					j_min = j;
				if (j_max < j)
					j_max = j;
				Image_C.at<uchar>(i, j) = 255 - srcImage.at<uchar>(i, j);
			}
			if (j_min != Image_C.cols + 1 || j_max != -1)
			{
				k[i].i = i;
				k[i].j_min = j_min;
				k[i].j_max = j_max;
			}
		}

		for (int i = 0; i < srcImage.rows; i++)
		{
			for (int j = k[i].j_min + 2; j <= k[i].j_max - 2; j++)
			{
				if (Image_C.at<uchar>(k[i].i, j) == 0 && Image_Show.at<uchar>(k[i].i, j) == 0)
				{
					int T = 0;
					int asize = 4;
					for (int t = 0; t < 4; t++)
					{
						if (j + a[t] < k[i].j_min || j + a[t] > k[i].j_max)
						{
							asize--;
							continue;
						}

						T += Image_C.at<uchar>(k[i].i, j + a[t]);
					}
					Image_Show.at<uchar>(k[i].i, j) = T / asize;
				}
				if (Image_C.at<uchar>(k[i].i, j) != 0)
					Image_Show.at<uchar>(k[i].i, j) = Image_C.at<uchar>(k[i].i, j);
			}
		}
		delete[] k;
	}
	else
		fan_switch = true;
	outImage = Image_Show.clone();
	Image_C.release();
	Image_Show.release();
	return false;
}

void smooth(std::vector<Point>& rawData, int smoothNum);

bool Center_Line(const Mat & srcImage, Mat & outImage, int modelLength)
{
	bool flag = false;
	if (srcImage.empty() || outImage.empty())
		return flag;
	Mat dstImage = Mat::zeros(srcImage.size(), CV_8UC1);//反转图像
	Mat tempData = Mat::zeros(1, srcImage.cols, CV_64FC1);
	vector<MyPoint> pts;
	Mat Image_A = srcImage.clone();
	MyPoint tempPoint;
	int Th = 0;
	int L = 5;
/////////////**********************************************////////////
	//ROI搜索
	int* k = new int[srcImage.rows]();//中心点数组
	k[0] = -1;
	int j_min = -1, j_max = -1, j_num = 0, j_0_num = 0;//j_min left j_max right
	int n_index;//标志量
	for (int i = 0; i < srcImage.rows; i++)//求左右边界
	{
		//左边界
		//首行操作
		if (i == 0)
		{
			for (int j = 0; j < srcImage.cols - modelLength; j++)
			{
				if (srcImage.at<uchar>(i, j) > Th) {
					j_min = j - modelLength;
					break;
				}
			}

			if (j_min == -1)
			{
				k[i] = -1;
				continue;
			}
		}
		else//其他行
		{
			if (k[i - 1] == -1)
			{
				for (int j = 0; j < srcImage.cols - modelLength; j++)
				{
					if (srcImage.at<uchar>(i, j) > Th) {
						j_min = j - modelLength;
						break;
					}
				}

				if (j_min == -1)
				{
					k[i] = -1;
					continue;
				}
			}
			else
			{
				if (srcImage.at<uchar>(i, k[i - 1]) > Th)
				{
					for (int j = k[i - 1] - 1; j >= 0; j--)
					{
						if (srcImage.at<uchar>(i, j) <= Th)
							j_0_num++;
						if (srcImage.at<uchar>(i, j) > Th)
						{
							j_0_num = 0;
							continue;
						}
						if (j_0_num == L)
						{
							j_min = j + L - modelLength;
							j_0_num = 0;
							break;
						}
						if (j == 0 && j_0_num != L)
							j_min = 0;
					}
				}
				else
				{
					n_index = k[i - 1];//上一行中心点列坐标
					for (int j = k[i - 1] - 1; j >= 0; j--)
					{
						if (srcImage.at<uchar>(i, j) > Th)
						{
							j_0_num = 0;
							for (int jj = j - 1; jj >= 0; jj--)
							{
								if (srcImage.at<uchar>(i, jj) <= Th)
									j_0_num++;
								if (srcImage.at<uchar>(i, jj) > Th)
								{
									j_0_num = 0;
									continue;
								}
								if (j_0_num == L)
								{
									j_min = jj + L - modelLength;
									j_0_num = 0;
									break;
								}
								if (jj == 0 && j_0_num != L)
									j_min = 0;
							}
							break;
						}
						n_index--;//until j==0
					}
					if (n_index == 0)
					{
						n_index = k[i - 1] + 1;
						for (int j = k[i - 1] + 1; j < srcImage.cols - modelLength; j++)
						{
							if (srcImage.at<uchar>(i, j) > Th)
							{
								j_min = j - modelLength;
								break;
							}
							n_index++;
						}
						if (n_index == srcImage.cols - modelLength)
						{
							k[i] = -1;
							j_min = -1;
						}
					}
				}
			}

		}
		//左边界
		if (j_min != -1)
		{
			for (int j = j_min + modelLength + 1; j < srcImage.cols - modelLength; j++)
			{
				if (srcImage.at<uchar>(i, j) <= Th) {
					j_num++;
				}
				if (srcImage.at<uchar>(i, j) > Th) {
					j_num = 0;
					continue;
				}
				if (j_num == L) {
					j_max = j - L;
					j_num = 0;
					break;
				}
			}
			if (j_min <= 0)
				j_min = 0;
			////////////差分计算/////////
			for (int j = j_min; j <= j_max; j++)
			{
				tempData.at<double>(0, j) = modelLength * Image_A.at<uchar>(i, j);
				for (int count = 1; count <= modelLength; count++)
				{
					tempData.at<double>(0, j) -= Image_A.at<uchar>(i, j + count);
				}
			}

			
			double max = tempData.at<double>(0, 0);//val
			int max_j = -1;//index
			double min = tempData.at<double>(0, 0);
			int min_j = -1;
			for (int j = j_min; j <= j_max; j++)
			{
				if (max < tempData.at<double>(0, j))
				{
					max = tempData.at<double>(0, j);
					max_j = j;
				}
				if (min > tempData.at<double>(0, j))
				{
					min = tempData.at<double>(0, j);
					min_j = j;
				}
			}
			j_min = -1;
			j_max = -1;

			//if (max_j - min_j < 60)//初步提取激光线，条件1
			int j1 = floor((max_j + min_j) / 2);
			k[i] = j1;
			MyPoint tempPoint;
			tempPoint.i = i;
			tempPoint.j = j1;
			tempPoint.flag = 1;
			if (j1 != -1 && max_j != -1 && min_j != -1)
			{
				pts.push_back(tempPoint);
			}
		}
	}
	/////////////********************************************//////////////////////////




	vector<Point> points;
	for (int i = 0; i < pts.size(); i++)
	{
		if (pts[i].flag == 1)
		{
			Point temp;
			temp.y = pts[i].i;
			temp.x = pts[i].j;
			points.push_back(temp);
		}
	}


	smooth(points, 1);//平滑

	//判断连续的两个点差多少个像素
	vector<Point> insert_vec;
	for (int i = 1; i < points.size(); i++)
	{
		int count = points[i].y - points[i - 1].y;
		if (count > 1 && count < 15)
		{
			double x_rad = (points[i].x - points[i - 1].x) * 1.0 / (count);
			for (int j = 0; j < count - 1; j++)
			{
				Point point1;
				point1.y = points[i - 1].y + 1 + j;
				point1.x = points[i - 1].x + x_rad * (1 + j);
				insert_vec.push_back(point1);
			}

		}
	}
	points.insert(points.end(), insert_vec.begin(), insert_vec.end());//将insert_vec容器里面的数据直接添加到points容器后面.

	for (int i = 0; i < points.size(); i++)
	{
		outImage.at<uchar>(points[i].y, points[i].x) = 255;
	}
	if (points.size() != 0)
		flag = true;

	vector<Point>().swap(points);
	dstImage.release();
	tempData.release();
	return flag;
}

void smooth(std::vector<Point>& rawData, int smoothNum)
{
	if (rawData.size() < 10)
	{
		return;
	}
	std::vector<Point> processData(rawData);
	int i = 0;
	while (i++ < smoothNum)//平滑次数
	{
		processData[0].x = 0.65 * rawData[0].x + 0.2 * rawData[1].x + 0.1 * rawData[2].x + 0.05 * rawData[3].x;
		processData[1].x = 0.2 * rawData[0].x + 0.45 * rawData[1].x + 0.2 * rawData[2].x + 0.1 * rawData[3].x + 0.05 * rawData[4].x;
		processData[2].x = 0.1 * rawData[0].x + 0.2 * rawData[1].x + 0.35 * rawData[2].x + 0.2 * rawData[3].x + 0.1 * rawData[4].x + 0.05 * rawData[5].x;

		int N = rawData.size();
		for (int i = 3; i < N - 3; i++)
		{
			processData[i].x = 0.05 * rawData[i - 3].x + 0.1 * rawData[i - 2].x + 0.2 * rawData[i - 1].x + 0.3 * rawData[i].x + 0.2 * rawData[i + 1].x + 0.1 * rawData[i + 2].x + 0.05 * rawData[i + 3].x;
		}
		processData[N - 3].x = 0.1 * rawData[N - 1].x + 0.2 * rawData[N - 2].x + 0.35 * rawData[N - 3].x + 0.2 * rawData[N - 4].x + 0.1 * rawData[N - 5].x + 0.05 * rawData[N - 6].x;
		processData[N - 2].x = 0.2 * rawData[N - 1].x + 0.45 * rawData[N - 2].x + 0.2 * rawData[N - 3].x + 0.1 * rawData[N - 4].x + 0.05 * rawData[N - 5].x;
		processData[N - 1].x = 0.65 * rawData[N - 1].x + 0.2 * rawData[N - 2].x + 0.1 * rawData[N - 3].x + 0.05 * rawData[N - 4].x;

		rawData = processData;
	}
}
