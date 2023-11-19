#include "preprocessing.h"
#include <algorithm>
#include <numeric>
const double preprocessing::templateChannelWeight[3] = { 0.114,0.299,0.587 };

void preprocessing::calculateWeightAndCompensation(cv::Mat img_channels[3], double weight[3], double compensation[3]) {
	//��ͨ���Ҷ�ռ��ͳ������eg proportion['B']Ϊ��ͼ������Bͨ���Ҷ�ռ��ͨ���Ҷ�֮�͵ı�
	std::map<_channel_t, double> proportion;
	proportion.insert(std::pair<_channel_t, double>{B, 0});
	proportion.insert(std::pair<_channel_t, double>{G, 0});
	proportion.insert(std::pair<_channel_t, double>{R, 0});
	auto &proportion_B = proportion[B];
	auto &proportion_G = proportion[G];
	auto &proportion_R = proportion[R];

	//��ͨ��ͳ������eg count[0]Ϊ�õ��Bͨ��ǿ�����
	int count[3];//0->B,1->G,2->R
	auto &count_B = count[B];
	auto &count_G = count[G];
	auto &count_R = count[R];

	//��һ��
	double normalization[3];
	auto &normalized_B = normalization[B];
	auto &normalized_G = normalization[G];
	auto &normalized_R = normalization[R];

	//
	double re_tmpcw[3];
	auto &re_tmpcw_B = re_tmpcw[B];
	auto &re_tmpcw_G = re_tmpcw[G];
	auto &re_tmpcw_R = re_tmpcw[R];

	//ͳ��ԭʼ��ͨ����������ͨ���Ҷ�ռ��
	//ʹ��һ������ѭ��������ͼ����������أ�����ʱ�临�Ӷ�
	//ʹ��ptr����������ͼ�������ֵ����ߴ����Ч�ʺ�����
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

	//�ԻҶ�ֵ����һ�β���
	(count_G >= count_R) ? count_G *= 2 : count_R *= 2;

	//��������ͼ��Ŀ�������ع�ϸ�
	if (count_G + count_R >= count_B / 5)
	{
		//�ԻҶȽ��ж��β���
		(count_G >= count_R) ? count_R *= 2 : count_G *= 2;

		//������ɫͨ��Ȩ�أ�����������ͨ��Ȩ��
		compensation[B] = (count_B - count_G - count_R) / 4;
		compensation[G] = proportion_G + (proportion_B - compensation[B]) / 2;
		compensation[R] = proportion_R + (proportion_B - compensation[B]) / 2;
		//fan_switch = false;
	}
	//��������ͼ��Ŀ�������ع�ϵ�
	else
	{
		//Ȩ�ز���
		compensation[B] = proportion_B;
		compensation[G] = proportion_G;
		compensation[R] = proportion_R;
	}

	//compensation��һ��
	double compensation_sum = std::accumulate(compensation, compensation + 3, 0);
	for (auto i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}

	auto cmp = [](std::pair<_channel_t, double> a, std::pair<_channel_t, double> b)-> bool {
		// ���a����b������true����ʾaӦ������bǰ�棬������
		return a.second > b.second;
	};
	std::sort(proportion.begin(), proportion.end(), cmp);//��������

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

//ת��ͼ���ɫ�ʿռ��ɫ����ȵĺ���ʵ��
void preprocessing::convertColorSpaceAndDepth(cv::Mat& img, cv::Mat& output) {
	//���ͼ������ͺ�ͨ����
	if (img.type() != CV_8UC3) {
		throw std::invalid_argument("The input image must be of type CV_8UC3");
	}

	//����һ���յĻҶ�ͼ��
	output = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);

	//����ͼ�������ͨ��
	cv::Mat channels[3];//0->B,1->G,2->R
	cv::split(img, channels);

	//����ͼ���Ȩ�غͲ���
	double weight[3];
	double compensation[3];
	calculateWeightAndCompensation(channels, weight, compensation);

	//����Ȩ�غͲ�������Ҷ�ֵ
	//ʹ��һ������ѭ��������ͼ����������أ�����ʱ�临�Ӷ�
	//ʹ��ptr����������ͼ�������ֵ����ߴ����Ч�ʺ�����
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

//��RGBͼ��ת��Ϊ�Ҷ�ͼ��ĺ���ʵ��
cv::Mat preprocessing::bgr2Gray() {
	cv::Mat output; //����ĻҶ�ͼ��
	convertColorSpaceAndDepth(img, output); //ת��
	return output; //����
}














cv::Mat preprocessing::bgr2Gray() {
	cv::Mat output = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);

	cv::Mat img_channels[3];//0->B,1->G,2->R
	cv::split(img, img_channels);
	//��ͨ���Ҷ�ռ��ͳ������eg proportion_BΪ��ͼ������Bͨ���Ҷ�ռ��ͨ���Ҷ�֮�͵ı�
	std::map<char, double> proportion;
	proportion.insert(std::pair<char, double>{'B', 0});
	proportion.insert(std::pair<char, double>{'G', 0});
	proportion.insert(std::pair<char, double>{'R', 0});
	auto &proportion_B = proportion['B'];
	auto &proportion_G = proportion['G'];
	auto &proportion_R = proportion['R'];
	//��ͨ��ͳ������eg count_BΪ�õ��Bͨ��ǿ�����
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
	//ͳ��ԭʼ��ͨ����������ͨ���Ҷ�ռ��
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

	//�ԻҶ�ֵ����һ�β���
	(count_G >= count_R) ? count_G *= 2 : count_R *= 2;
	//��������ͼ��Ŀ�������ع�ϸ�
	if (count_G + count_R >= count_B / 5)
	{	//�ԻҶȽ��ж��β���
	    (count_G >= count_R) ? count_R *= 2 : count_G *= 2;
		//������ɫͨ��Ȩ�أ�����������ͨ��Ȩ��
		compensation_B = (count_B - count_G - count_R) / 4;
		compensation_G = proportion_G + (proportion_B - compensation_B) / 2;
		compensation_R = proportion_R + (proportion_B - compensation_B) / 2;
		//fan_switch = false;
	}
	//��������ͼ��Ŀ�������ع�ϵ�
	else
	{	//Ȩ�ز���
		compensation_B =proportion_B;
		compensation_G =proportion_G;
		compensation_R = proportion_R;
	}
	//compensation��һ��
	double compensation_sum=std::accumulate(compensation, compensation + 3, 0);
	for (auto i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}
	///////////////////////////////////////////////////////////////////

	auto cmp = [](std::pair<char, double> a, std::pair<char, double> b)-> bool{
		// ���a����b������true����ʾaӦ������bǰ�棬������
		return a.second > b.second;
	};
	std::sort(proportion.begin(), proportion.end(), cmp);//��������
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