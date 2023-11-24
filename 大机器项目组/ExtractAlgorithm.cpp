#include "ExtractAlgorithm.h"

///////////////////////////////
//��ɫͨ���Ĳο��㣬��¼���У��У��Ҷ�ֵ
struct ReferencePoint
{
	int row;
	int col;
	int pixelValue;
};


//�˽ṹ���bgrÿ�е�ƽ���Ҷ�ֵ�ͱ���
struct RowAverageGray
{
	double bRowAverageGrayValue;
	double gRowAverageGrayValue;
	double rRowAverageGrayValue;
	double ratio_RG_divided_B;

};


//����λ������ɫͨ����������ṩ
struct grChannelpixelValue
{
	int gPixelValue;
	int rPixelValue;
	int gRowPixelValueSum;
	int rRowPixelValueSum;
};


struct ConnectedComponent
{
	int label;  // ��ͨ���ǩ
	int count;  // ��ͨ�����ؼ���
	std::vector<cv::Point> points;  // ��ͨ���еĵ�λ��
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

	
	// ����ɫͼ�����������Ҷ�ͼ��
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

	ReferencePoint BlueDemandPixels = {-1,-1,0};//��ɫͨ���������Ϣ

	vector<ReferencePoint> pixelPositions; //���������Ϣn��

	RowAverageGray AverageGray = {0.0,0.0,0.0,0.0};//��ͨ�����д��ƽ���Ҷ�

	vector<RowAverageGray> ChannelAveragesGray;

	double minRatio = std::numeric_limits<double>::max();

	for(int i = 0;i<blueChannel.rows;i++)
	{
		int bChannelRowSum=0;
		int gChannelRowSum=0;
		int rChannelRowSum=0;


		grChannelpixelValue grPixelValue = {0,0,0,0};//�ֱ��¼��ɫ����ɫ ͨ�������ĻҶ�ֵ��λ������ɫͨ���������Ϣ�ṩ
		//�ֱ��¼ͬλ�õĺ�ɫͨ������ɫͨ��������ֵ�ĺ�

		//grPixelValue.gPixelValue=0;
		//grPixelValue.rPixelValue=0;
		int  count=1;

		for (int j = 0;j<blueChannel.cols;j++)
		{

			//������ɫͨ��Ϊ��׼���Ҷ�ֵ����t�������ۣ���������
			if (blueChannel.at<uchar>(i,j)<50)
			{
				int NoNum=0;
				NoNum++;
				continue;
			}
			else 
			{
				//��¼�Ҷ�ֵ����t�����ص�����//
				BlueDemandPixels.row = i;
				BlueDemandPixels.col = j;
				BlueDemandPixels.pixelValue=blueChannel.at<uchar>(i,j);
				pixelPositions.push_back(BlueDemandPixels);

			
				//��ɫ ��ɫͨ��ÿ�����������غ�
				bChannelRowSum+=blueChannel.at<uchar>(i,j);
				gChannelRowSum+=greenChannel.at<uchar>(i,j);
				rChannelRowSum+=redChannel.at<uchar>(i,j);
				
				count++;

				//grPixelValue.gRowPixelValueSum += greenChannel.at<int>(i,j);
				//grPixelValue.rRowPixelValueSum += redChannel.at<int>(i,j);

			}

		}

		//��ȡ��ͬλ�õģ�bgrͨ��ÿ��������ƽ���Ҷ�ֵ,����������
		double bRowAverageGrayValue = 0.0;
		double gRowAverageGrayValue = 0.0;
		double rRowAverageGrayValue = 0.0;

		bRowAverageGrayValue=bChannelRowSum/count;
		gRowAverageGrayValue=gChannelRowSum/count;
		rRowAverageGrayValue=rChannelRowSum/count;

		AverageGray.bRowAverageGrayValue=bRowAverageGrayValue;
		AverageGray.gRowAverageGrayValue=gRowAverageGrayValue;
		AverageGray.rRowAverageGrayValue=rRowAverageGrayValue;

		//��ȡÿ�У�r+g��/b��������������
		AverageGray.ratio_RG_divided_B=(AverageGray.rRowAverageGrayValue+AverageGray.gRowAverageGrayValue)/(AverageGray.bRowAverageGrayValue+1);//

		/*************************************************************************************/
		//AverageGray.ratio_RG_divided_B=(gChannelRowSum + rChannelRowSum) / (bChannelRowSum+1);
		//�����޷���ȷ��ӳ����ɫ����ɫͨ���������ɫͨ������ʵǿ�ȱ�����ϵ��//


		ChannelAveragesGray.push_back(AverageGray);

		//��ֵ����Сֵ
		for (int i = 0; i < ChannelAveragesGray.size(); i++)
		{
			double Ratio_RG_divided_B = ChannelAveragesGray[i].ratio_RG_divided_B;
			if (Ratio_RG_divided_B > 0.001 && Ratio_RG_divided_B <minRatio) 
			{
				minRatio = Ratio_RG_divided_B;
			}
		}


	}
	

		// ��ÿһ�е����ؽ��д����ͳ��
		for (int i = 0; i < blueChannel.rows; i++)
		{
			// �ж�ÿһ�еı����Ƿ������ֵ,���������
			if (ChannelAveragesGray[i].ratio_RG_divided_B >=1*minRatio)//����ֵ��row�ĳ�0�����Сֵ�������ֵ̫�������������߶��ѣ���ɢ
			{
				// �����������������е�����
				for (int j = 0; j < blueChannel.cols; j++)
				{
				    // �����е���������Ϊ (redChannel.at<uchar>(i, j)+greenChannel.at<uchar>(i, j))/4
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
			// �ж�ÿһ�еı����Ƿ�С������ֵ
			if (ChannelAveragesGray[i].ratio_RG_divided_B <15*minRatio)
			{
				 count = cv::countNonZero(blueChannel.row(i) > 50); // ͳ�ƴ���50�����ص����
			}
			if (count<20&ChannelAveragesGray[i].ratio_RG_divided_B <15*minRatio)//���������Ᵽ��
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

		

	   ////////////////������������䣬�򷵻أ���������˲��������������������/////////////////

		return SelectChannelImage;

/**********************************************************************************************/



		////////////////////////�Ż�forѭ�� 9.22///////////////////////

		//for (int i = 0; i < blueChannel.rows; i++)
		//{
		//	int count = 0;
		//	uchar* bluePtr = blueChannel.ptr<uchar>(i);
		//	uchar* redPtr = redChannel.ptr<uchar>(i);
		//	uchar* greenPtr = greenChannel.ptr<uchar>(i);
		//	uchar* adjustBluePtr = adjustBlueChannel.ptr<uchar>(i);

		//	// �ж�ÿһ�еı����Ƿ������ֵ
		//	if (ChannelAveragesGray[i].ratio_RG_divided_B > t * minRatio)
		//	{
		//		// �����������������е�����
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
		//			if (bluePtr[j] > 50)// ͳ�ƴ���50�����ص����
		//			{
		//				count++;
		//			}
		//		}
		//		if (count < 30)
		//		{
		//			// ���������Ᵽ��
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

	// ����ѷ��ʹ�������
	std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
	dstImage = srcImage.clone();

	// �������ӵ�
	std::vector<cv::Point> seedPoints;

	// �������ӵ�
	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			if (srcImage.at<uchar>(y, x) < threshold) 
			{
				// ���� grayImage Ϊ�յ����
				// ���Դ�ӡ������Ϣ�򷵻ش������
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

	// ������ͨ������
	std::vector<ConnectedComponent> components;

	int nextLabel = 1;

	// �������ӵ�
	for (const auto& seedPoint : seedPoints) 
	{
		if (visited[seedPoint.y][seedPoint.x]) {
			continue;
		}

		// �����µ���ͨ��
		ConnectedComponent component;
		component.label = nextLabel++;
		component.count = 0;

		std::vector<cv::Point> points;
		points.emplace_back(seedPoint.x, seedPoint.y);

		// ��ͨ������
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

				// ��¼��λ��
				component.points.emplace_back(px, py);

				// ����4����
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

		// �����ͨ������
		components.push_back(component);
	}

	// ����������
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
			// ���С����������1/3������Ӧ������Ϊ0
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

				// ����8����
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
	while(i++ < smoothNum)//ƽ������
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
	Mat dstImage = Mat::zeros(srcImage.size(), CV_8UC1);//��תͼ��,CV_8UCI char

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

		//�ж�������Ѱ����С���ֵλ�õľ�ֵ�����о������ֵҪ����60
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

		//if (max_j - min_j < 60)//������ȡ�����ߣ�����1
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

	//�ж����������������ٸ�����
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
	points.insert(points.end(), insert_vec.begin(), insert_vec.end());//��insert_vec�������������ֱ����ӵ�points��������.

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
