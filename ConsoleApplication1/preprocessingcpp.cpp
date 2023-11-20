#include "preprocessing.h"
//��RGBͼ��ת��Ϊ�Ҷ�ͼ��ĺ���ʵ��
cv::Mat preprocessing::bgr2gray() {
	cv::Mat output; //����ĻҶ�ͼ��
	convertColor(input, output); //ת��
	return output; //����
}

//ת��ͼ���ɫ�ʿռ��ɫ����ȵĺ���ʵ��
void preprocessing::convertColor(cv::Mat& input, cv::Mat& output) {
	//���ͼ������ͺ�ͨ����
	if (input.type() != CV_8UC3) {
		throw std::invalid_argument("The input image must be of type CV_8UC3");
	}

	//�ջҶ�ͼ��
	output = cv::Mat::zeros(input.rows, input.cols, CV_8UC1);

	//����ͼ��ͨ��
	cv::Mat channel[3];//0->B,1->G,2->R
	cv::split(input, channel);

	//����ͼ��Ĳ���Ȩ��
	double weight[3] = { 0,0,0 };
	calculateWeight(channel, weight);

	//���ݲ���Ȩ�ؼ���Ҷ�ֵ
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

//����ͼ�񲹳�Ȩ�غ�����ʵ��
void preprocessing::calculateWeight(cv::Mat channel[3], double weight[3]) {
	//һЩͳ��������
	proportion_t proportion[3];//��ͨ���Ҷ�ռ��ͳ������eg proportion[B]Ϊ��ͼ������Bͨ���Ҷ�ռ��ͨ���Ҷ�֮�͵ı�
	proportion[B] = { B,0 };
	proportion[G] = { G,0 };
	proportion[R] = { R,0 };
	int count[3] = { 0,0,0 };//��ͨ��ͳ������eg count[0]Ϊ�õ��Bͨ��ǿ�����
	double compensation[3] = { 0,0,0 };//��һ�β������Ȩ��
	double normalization[3] = { 0,0,0 };//��һ����һ�β���Ȩ��
	double templateWeight[3] = { 0.114,0.299,0.587 };//ģ��Ȩ��
	double re_tmpW[3] = { 0,0,0 };//�ط�����ģ��Ȩ��

	//ͳ��ԭʼ��ͨ����������ͨ���Ҷ�ռ��
	for (int i = 0; i < input.rows; i++)
	{	//��ʱָ�룬�����ͨ������ָ��
		auto intensity_B = channel[B].ptr<uchar>(i);
		auto intensity_G = channel[G].ptr<uchar>(i);
		auto intensity_R = channel[R].ptr<uchar>(i);
		for (int j = 0; j < input.cols; j++)
		{	//���ͼ��ǿ��С����ֵ������ͳ��
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

	//---------------------------------------------һ��Ȩ�ز���------------------------------------------------//

	//�ԻҶȽ���һ�β���
	(count[G] >= count[R]) ? count[G] *= 2 : count[R] *= 2;

	//��������ͼ��Ŀ�������ع�ϸ�
	if (count[G] + count[R] >= count[B] / 5)
	{
		//�ԻҶȽ��ж��β���
		(count[G] >= count[R]) ? count[R] *= 2 : count[G] *= 2;

		//��һ��Ȩ�ز�����������ɫͨ��Ȩ�أ�����������ͨ��Ȩ��
		compensation[B] = (count[B] - count[G] - count[R]) / 4;
		compensation[G] = proportion[G].weight + (proportion[B].weight - compensation[B]) / 2;
		compensation[R] = proportion[R].weight + (proportion[B].weight - compensation[B]) / 2;
		//fan_switch = false;
	}
	//��������ͼ��Ŀ�������ع�ϵ�
	else
	{
		//Ȩ�ز���
		compensation[B] = proportion[B].weight;
		compensation[G] = proportion[G].weight;
		compensation[R] = proportion[R].weight;
	}

	//��һ����һ��Ȩ�ز���
	double compensation_sum = std::accumulate(compensation, compensation + 3, 0);
	for (int i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}

	//---------------------------------------------����Ȩ�ز���------------------------------------------------//
	//��proportion���н�������
	auto cmp = [](proportion_t a, proportion_t b)-> bool {
		// ���a����b������true����ʾaӦ������bǰ�棬������
		return a.weight > b.weight;
	};
	std::sort(proportion, proportion + 3, cmp);//��������

	//���·���ģ��Ȩ��
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

	//����Ȩ�ز���
	weight[B] = (re_tmpW[B] + normalization[B]) / 2;
	weight[G] = (re_tmpW[G] + normalization[G]) / 2;
	weight[R] = (re_tmpW[R] + normalization[R]) / 2;
}