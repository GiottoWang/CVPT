#include "ExtractAlgorithm.h"

///////////////////////////////
//蓝色通道的参考点，记录，行，列，灰度值
struct ReferencePoint
{
	int row;
	int col;
	int pixelValue;
};


//此结构体存bgr每行的平均灰度值和比例
struct RowAverageGray
{
	double bRowAverageGrayValue;
	double gRowAverageGrayValue;
	double rRowAverageGrayValue;
	double ratio_RG_divided_B;

};


//具体位置由蓝色通道的需求点提供
struct grChannelpixelValue
{
	int gPixelValue;
	int rPixelValue;
	int gRowPixelValueSum;
	int rRowPixelValueSum;
};


struct ConnectedComponent
{
	int label;  // 连通域标签
	int count;  // 连通域像素计数
	std::vector<cv::Point> points;  // 连通域中的点位置
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

Mat CExtractAlgorithm::splitChannelExtractROI_1(Mat& srcImg, Mat& SelectChannelImage)
{
	vector<Mat> channels(3);
	cv::split(srcImg, channels);
	Mat blueChannel = channels[0];
	Mat greenChannel = channels[1];
	Mat redChannel = channels[2];

	Mat adjustBlueChannel = blueChannel.clone();

	double t = 1;

	struct ReferencePoint
	{
		int row;
		int col;
		uchar pixelValue;
	};

	vector<ReferencePoint> pixelPositions;

	struct RowAverageGray
	{
		double bRowAverageGrayValue;
		double gRowAverageGrayValue;
		double rRowAverageGrayValue;
		double ratio_RG_divided_B;
	};

	vector<RowAverageGray> ChannelAveragesGray;

	double minRatio = std::numeric_limits<double>::max();

	for (int i = 0; i < blueChannel.rows; i++)
	{
		int bChannelRowSum = 0;
		int gChannelRowSum = 0;
		int rChannelRowSum = 0;

		for (int j = 0; j < blueChannel.cols; j++)
		{
			if (blueChannel.at<uchar>(i, j) < 50)
			{
				continue;
			}
			else
			{
				ReferencePoint BlueDemandPixels;
				BlueDemandPixels.row = i;
				BlueDemandPixels.col = j;
				BlueDemandPixels.pixelValue = blueChannel.at<uchar>(i, j);
				pixelPositions.push_back(BlueDemandPixels);

				bChannelRowSum += blueChannel.at<uchar>(i, j);
				gChannelRowSum += greenChannel.at<uchar>(i, j);
				rChannelRowSum += redChannel.at<uchar>(i, j);
			}
		}

		double bRowAverageGrayValue = bChannelRowSum / pixelPositions.size();
		double gRowAverageGrayValue = gChannelRowSum / pixelPositions.size();
		double rRowAverageGrayValue = rChannelRowSum / pixelPositions.size();

		RowAverageGray AverageGray;
		AverageGray.bRowAverageGrayValue = bRowAverageGrayValue;
		AverageGray.gRowAverageGrayValue = gRowAverageGrayValue;
		AverageGray.rRowAverageGrayValue = rRowAverageGrayValue;
		AverageGray.ratio_RG_divided_B = (rRowAverageGrayValue + gRowAverageGrayValue) / (bRowAverageGrayValue + 1);

		ChannelAveragesGray.push_back(AverageGray);

		if (AverageGray.ratio_RG_divided_B > 0.01 && AverageGray.ratio_RG_divided_B < minRatio)
		{
			minRatio = AverageGray.ratio_RG_divided_B;
		}
	}

	for (int i = 0; i < blueChannel.rows; i++)
	{
		if (ChannelAveragesGray[i].ratio_RG_divided_B >= t * minRatio)
		{
			for (int j = 0; j < blueChannel.cols; j++)
			{
				adjustBlueChannel.at<uchar>(i, j) = (redChannel.at<uchar>(i, j) + greenChannel.at<uchar>(i, j)) / 4;
			}
		}
	}

	connectedComponentFilter_4(redChannel, redChannel);
	medianBlur(redChannel, redChannel, 5);

	for (int i = 0; i < blueChannel.rows; i++)
	{
		if (ChannelAveragesGray[i].ratio_RG_divided_B < 3 * t * minRatio)
		{
			int count = cv::countNonZero(blueChannel.row(i) > 50);
			if (count < 30)
			{
				for (int j = 0; j < blueChannel.cols; j++)
				{
					adjustBlueChannel.at<uchar>(i, j) = blueChannel.at<uchar>(i, j);
				}
			}
		}
	}

	bitwise_or(redChannel, adjustBlueChannel, SelectChannelImage);

	return SelectChannelImage;
}




Mat CExtractAlgorithm::splitChannelExtractROI(Mat&srcImg,Mat &SelectChannelImage )
{

	
	// 将彩色图像分离成三个灰度图像
	vector<Mat> channels(3);
	cv::split(srcImg, channels);
	Mat blueChannel = channels[0];
	Mat greenChannel = channels[1];
	Mat redChannel = channels[2];
	
	//string RGB_R = format("E:\\lyle\\Desktop\\zhuanli\\RGB\\R\\%d.jpg");
	//string RGB_G = format("E:\\lyle\\Desktop\\zhuanli\\RGB\\G\\%d.jpg");
	//string RGB_B = format("E:\\lyle\\Desktop\\zhuanli\\RGB\\B\\%d.jpg");

	//imwrite(RGB_R,redChannel);
	//imwrite(RGB_G,greenChannel);
	//imwrite(RGB_B,blueChannel);


	double t = 1;

	Mat adjustBlueChannel = blueChannel.clone();

	ReferencePoint BlueDemandPixels = {-1,-1,0};//蓝色通道需求点信息

	vector<ReferencePoint> pixelPositions; //存需求点信息n个

	RowAverageGray AverageGray = {0.0,0.0,0.0,0.0};//三通道按行存的平均灰度

	vector<RowAverageGray> ChannelAveragesGray;

	double minRatio = std::numeric_limits<double>::max();

	for(int i = 0;i<blueChannel.rows;i++)
	{
		int bChannelRowSum=0;
		int gChannelRowSum=0;
		int rChannelRowSum=0;


		grChannelpixelValue grPixelValue = {0,0,0,0};//分别记录绿色，红色 通道需求点的灰度值，位置由蓝色通道需求点信息提供
		//分别记录同位置的红色通道和绿色通道的像素值的和

		//grPixelValue.gPixelValue=0;
		//grPixelValue.rPixelValue=0;
		int  count=1;

		for (int j = 0;j<blueChannel.cols;j++)
		{

			//均已蓝色通道为基准，灰度值大于t进入讨论，否则跳过
			if (blueChannel.at<uchar>(i,j)<50)
			{
				int NoNum=0;
				NoNum++;
				continue;
			}
			else 
			{
				//记录灰度值大于t的像素点坐标//
				BlueDemandPixels.row = i;
				BlueDemandPixels.col = j;
				BlueDemandPixels.pixelValue=blueChannel.at<uchar>(i,j);
				pixelPositions.push_back(BlueDemandPixels);

			
				//绿色 红色通道每行需求点的像素和
				bChannelRowSum+=blueChannel.at<uchar>(i,j);
				gChannelRowSum+=greenChannel.at<uchar>(i,j);
				rChannelRowSum+=redChannel.at<uchar>(i,j);
				
				count++;

				//grPixelValue.gRowPixelValueSum += greenChannel.at<int>(i,j);
				//grPixelValue.rRowPixelValueSum += redChannel.at<int>(i,j);

			}

		}

		//求取相同位置的，bgr通道每行需求点的平均灰度值,并保存起来
		double bRowAverageGrayValue = 0.0;
		double gRowAverageGrayValue = 0.0;
		double rRowAverageGrayValue = 0.0;

		bRowAverageGrayValue=bChannelRowSum/count;
		gRowAverageGrayValue=gChannelRowSum/count;
		rRowAverageGrayValue=rChannelRowSum/count;

		AverageGray.bRowAverageGrayValue=bRowAverageGrayValue;
		AverageGray.gRowAverageGrayValue=gRowAverageGrayValue;
		AverageGray.rRowAverageGrayValue=rRowAverageGrayValue;

		//求取每行（r+g）/b比例，并存起来
		AverageGray.ratio_RG_divided_B=(AverageGray.rRowAverageGrayValue+AverageGray.gRowAverageGrayValue)/(AverageGray.bRowAverageGrayValue+1);//

		/*************************************************************************************/
		//AverageGray.ratio_RG_divided_B=(gChannelRowSum + rChannelRowSum) / (bChannelRowSum+1);
		//可能无法正确反映出红色和绿色通道相对于蓝色通道的真实强度比例关系。//


		ChannelAveragesGray.push_back(AverageGray);

		//比值的最小值
		for (int i = 0; i < ChannelAveragesGray.size(); i++)
		{
			double Ratio_RG_divided_B = ChannelAveragesGray[i].ratio_RG_divided_B;
			if (Ratio_RG_divided_B > 0.001 && Ratio_RG_divided_B <minRatio) 
			{
				minRatio = Ratio_RG_divided_B;
			}
		}


	}
	

		// 对每一行的像素进行处理和统计
		for (int i = 0; i < blueChannel.rows; i++)
		{
			// 判断每一行的比例是否大于阈值,正反射情况
			if (ChannelAveragesGray[i].ratio_RG_divided_B >=1*minRatio)//此阈值是row的除0外的最小值，如果阈值太高则会出现中心线断裂，会散
			{
				// 正反射情况，处理该行的像素
				for (int j = 0; j < blueChannel.cols; j++)
				{
				    // 将该行的像素设置为 (redChannel.at<uchar>(i, j)+greenChannel.at<uchar>(i, j))/4
					adjustBlueChannel.at<uchar>(i, j) = (2*redChannel.at<uchar>(i, j) + 1*greenChannel.at<uchar>(i, j))/3.1 +blueChannel.at<uchar>(i, j)*0.03;
				}
			}
		}

		connectedComponentFilter_4(redChannel,redChannel);
		medianBlur(redChannel, redChannel, 5);
	   //bitwise_or(redChannel, adjustBlueChannel, SelectChannelImage);

		int count=0;
		for (int i = 0; i < blueChannel.rows; i++)
		{
			// 判断每一行的比例是否小于于阈值
			if (ChannelAveragesGray[i].ratio_RG_divided_B <15*minRatio)
			{
				 count = cv::countNonZero(blueChannel.row(i) > 50); // 统计大于50的像素点个数
			}
			if (count<20&ChannelAveragesGray[i].ratio_RG_divided_B <15*minRatio)//优良蓝激光保留
			{
				for (int j = 0; j < blueChannel.cols; j++)
				{
					adjustBlueChannel.at<uchar>(i, j) = blueChannel.at<uchar>(i, j);
				}

				//bitwise_or(redChannel, adjustBlueChannel, SelectChannelImage);
				bitwise_or(redChannel, adjustBlueChannel, adjustBlueChannel);
			}	
		}

		
		bitwise_or(redChannel, adjustBlueChannel, SelectChannelImage);

		

	   ////////////////如果含有漫反射，则返回，二次面积滤波，消除大量漫反射面积/////////////////

		return SelectChannelImage;

/**********************************************************************************************/



		////////////////////////优化for循环 9.22///////////////////////

		//for (int i = 0; i < blueChannel.rows; i++)
		//{
		//	int count = 0;
		//	uchar* bluePtr = blueChannel.ptr<uchar>(i);
		//	uchar* redPtr = redChannel.ptr<uchar>(i);
		//	uchar* greenPtr = greenChannel.ptr<uchar>(i);
		//	uchar* adjustBluePtr = adjustBlueChannel.ptr<uchar>(i);

		//	// 判断每一行的比例是否大于阈值
		//	if (ChannelAveragesGray[i].ratio_RG_divided_B > t * minRatio)
		//	{
		//		// 正反射情况，处理该行的像素
		//		for (int j = 0; j < blueChannel.cols; j++)
		//		{
		//			uchar pixelValue = (redPtr[j] + greenPtr[j]) / 4;
		//			adjustBluePtr[j] = pixelValue;
		//		}
		//	}
		//	else if (ChannelAveragesGray[i].ratio_RG_divided_B < 3 * t * minRatio)
		//	{

		//		for (int j = 0; j < blueChannel.cols; j++)
		//		{
		//			if (bluePtr[j] > 50)// 统计大于50的像素点个数
		//			{
		//				count++;
		//			}
		//		}
		//		if (count < 30)
		//		{
		//			// 优良蓝激光保留
		//			for (int j = 0; j < blueChannel.cols; j++)
		//			{
		//				adjustBluePtr[j] = bluePtr[j];
		//			}
		//		}
		//	}

		//}

		//connectedComponentFilter_4(redChannel, redChannel);
		//medianBlur(redChannel, redChannel, 5);
		//bitwise_or(redChannel, adjustBlueChannel, SelectChannelImage);

		//return SelectChannelImage;


}


void CExtractAlgorithm::connectedComponentFilter_4(const cv::Mat& srcImage, cv::Mat& dstImage) 
{


	int width = srcImage.cols;
	int height = srcImage.rows;
	int threshold = 0;

	// 标记已访问过的像素
	std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
	dstImage = srcImage.clone();

	// 定义种子点
	std::vector<cv::Point> seedPoints;

	// 查找种子点
	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			if (srcImage.at<uchar>(y, x) < threshold) 
			{
				// 处理 grayImage 为空的情况
				// 可以打印错误信息或返回错误码等
				//grayImage.at<uchar>(y, x) == 0;
				int NoNum=0;
				NoNum++;
				continue;
			} 
			else if (srcImage.at<uchar>(y, x) > threshold) 
			{
				seedPoints.emplace_back(x, y);
			}
		}
	}

	// 定义连通域向量
	std::vector<ConnectedComponent> components;

	int nextLabel = 1;

	// 遍历种子点
	for (const auto& seedPoint : seedPoints) 
	{
		if (visited[seedPoint.y][seedPoint.x]) {
			continue;
		}

		// 创建新的连通域
		ConnectedComponent component;
		component.label = nextLabel++;
		component.count = 0;

		std::vector<cv::Point> points;
		points.emplace_back(seedPoint.x, seedPoint.y);

		// 连通域搜索
		while (!points.empty()) 
		{
			cv::Point p = points.back();
			points.pop_back();

			int px = p.x;
			int py = p.y;

			if (px >= 0 && px < width && py >= 0 && py < height && !visited[py][px] &&
				srcImage.at<uchar>(py, px) > threshold) 
			{
				visited[py][px] = true;
				component.count++;

				// 记录点位置
				component.points.emplace_back(px, py);

				// 搜索4邻域
				int dx[] = {0, 1, 0, -1};
				int dy[] = {-1, 0, 1, 0};
				for (int k = 0; k < 4; k++) 
				{
					int nx = px + dx[k];
					int ny = py + dy[k];

					if (nx >= 0 && nx < width && ny >= 0 && ny < height && !visited[ny][nx]) 
					{
						points.emplace_back(nx, ny);
					}
				}
			}
		}

		// 添加连通域到向量
		components.push_back(component);
	}

	// 查找最大面积
	int maxArea = 0;

	for (const auto& component : components) 
	{
		if (component.count > maxArea) 
		{
			maxArea = component.count;
		}
	}

	for (const auto& component : components) 
	{
		if (component.count < maxArea / 10) 
		{
			// 面积小于最大面积的1/3，将对应区域置为0
			for (const auto& point : component.points)
			{
				dstImage.at<uchar>(point.y, point.x) =0;
			}
		}
	}
}

void CExtractAlgorithm::connectedComponentFilter_8(const cv::Mat& srcImage, cv::Mat& dstImage) 
{
	int width = srcImage.cols;
	int height = srcImage.rows;
	int threshold = 0;

	std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));

	dstImage = srcImage.clone();

	std::vector<cv::Point> seedPoints;

	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			if (srcImage.at<uchar>(y, x) < threshold) 
			{
				int NoNum=0;
				NoNum++;
				continue;
			} 
			else if (srcImage.at<uchar>(y, x) > threshold) 
			{
				seedPoints.emplace_back(x, y);
			}
		}
	}

	std::vector<ConnectedComponent> components;
	int nextLabel = 1;

	for (const auto& seedPoint : seedPoints) 
	{
		if (visited[seedPoint.y][seedPoint.x]) 
		{
			continue;
		}

		ConnectedComponent component;
		component.label = nextLabel++;
		component.count = 0;

		std::vector<cv::Point> points;
		points.emplace_back(seedPoint.x, seedPoint.y);

		while (!points.empty()) 
		{
			cv::Point p = points.back();
			points.pop_back();

			int px = p.x;
			int py = p.y;

			if (px >= 0 && px < width && py >= 0 && py < height && !visited[py][px] &&
				srcImage.at<uchar>(py, px) > threshold) 
			{
				visited[py][px] = true;
				component.count++;
				component.points.emplace_back(px, py);

				// 搜索8邻域
				int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
				int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

				for (int k = 0; k < 8; k++) 
				{
					int nx = px + dx[k];
					int ny = py + dy[k];

					if (nx >= 0 && nx < width && ny >= 0 && ny < height && !visited[ny][nx]) 
					{
						points.emplace_back(nx, ny);
					}
				}
			}
		}

		components.push_back(component);
	}

	int maxArea = 0;

	for (const auto& component : components) 
	{
		if (component.count > maxArea) 
		{
			maxArea = component.count;
		}
	}

	for (const auto& component : components) 
	{
		if (component.count < maxArea / 5) 
		{
			for (const auto& point : component.points)
			{
				dstImage.at<uchar>(point.y, point.x) = 0;
			}
		}
	}
}

void CExtractAlgorithm::smooth(std::vector<Point>& rawData,int smoothNum)
{
	if (rawData.size() < 10)
	{
		return;
	}
	std::vector<Point> processData(rawData);
	int i=0;
	while(i++ < smoothNum)//平滑次数
	{
		processData[0].x = 0.65 * rawData[0].x + 0.2 * rawData[1].x + 0.1 * rawData[2].x + 0.05 * rawData[3].x;
		processData[1].x = 0.2 * rawData[0].x + 0.45 * rawData[1].x + 0.2 * rawData[2].x + 0.1 * rawData[3].x + 0.05 * rawData[4].x;
		processData[2].x = 0.1 * rawData[0].x + 0.2 * rawData[1].x + 0.35 * rawData[2].x + 0.2 * rawData[3].x + 0.1 * rawData[4].x + 0.05 * rawData[5].x;

		int N = rawData.size();
		for (int i = 3; i < N - 3; i++)
		{
			processData[i].x =  0.05 * rawData[i - 3].x + 0.1 * rawData[i - 2].x + 0.2 * rawData[i- 1].x + 0.3 * rawData[i].x + 0.2 * rawData[i + 1].x + 0.1 * rawData[i + 2].x + 0.05 * rawData[i + 3].x;
		}
		processData[N - 3].x = 0.1 * rawData[N - 1].x + 0.2 * rawData[N - 2].x + 0.35 * rawData[N - 3].x + 0.2 * rawData[N - 4].x + 0.1 * rawData[N - 5].x + 0.05 * rawData[N - 6].x;
		processData[N - 2].x = 0.2 * rawData[N - 1].x + 0.45 * rawData[N - 2].x + 0.2 * rawData[N - 3].x + 0.1 * rawData[N - 4].x + 0.05 * rawData[N - 5].x;
		processData[N - 1].x = 0.65 * rawData[N - 1].x + 0.2 * rawData[N - 2].x + 0.1 * rawData[N - 3].x + 0.05 * rawData[N - 4].x;

		rawData = processData;
	}
}

bool CExtractAlgorithm::detectResult_sjw_vertical(const Mat& srcImage, Mat& outImage, int modelLength)
{
	bool flag = false;
	if (srcImage.empty() || outImage.empty())
		return flag;
	Mat dstImage = Mat::zeros(srcImage.size(), CV_8UC1);//反转图像,CV_8UCI char

	Mat tempData = Mat::zeros(1, srcImage.cols, CV_64FC1);

	vector<MyPoint> pts;

	for (int i = 0; i < srcImage.rows; i++)
	{
		for (int j = 0; j < srcImage.cols - modelLength; j++)
		{
			tempData.at<double>(0, j) = modelLength * srcImage.at<uchar>(i, j);
			for (int count = 1; count <= modelLength; count++)
			{
				tempData.at<double>(0, j) -= srcImage.at<uchar>(i, j + count);
			}
		}

		//判断条件，寻找最小最大值位置的均值，还有就是最大值要大于60
		double max = tempData.at<double>(0, 0);
		int max_j = -1;
		double min = tempData.at<double>(0, 0);
		int min_j = -1;
		for (int j = 0; j < tempData.cols; j++)
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

		//if (max_j - min_j < 60)//初步提取激光线，条件1
		int j = floor((max_j + min_j) / 2);
		MyPoint tempPoint;
		tempPoint.i = i;
		tempPoint.j = j;
		tempPoint.flag = 1;
		if (j != -1 && max_j != -1 && min_j != -1)
		{
			pts.push_back(tempPoint);
		}
	}

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

	smooth(points, 1);

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
