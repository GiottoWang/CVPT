#include "PreprocessingAlgorithm.h"
//��RGBͼ��ת��Ϊ�Ҷ�ͼ��ĺ���ʵ��
cv::Mat PreprocessingAlgorithm_A::process() {
	convertColor(); //ת��
	deNoise();
	invertion();
	return this->output; //����
}

//�Ե��ع�ͼ�����ȡ��
void PreprocessingAlgorithm_A::invertion() {

	cv::Mat inverted = output.clone();
	cv::Mat result = output.clone();
	int direct[4] = { -2,-1,1,2 };
	int y_left, y_right;
	const int BEGIN = -1;
	const int END = output.cols + 1;
	if (FLAG_INVERT == true)
	{
		std::vector<effective_t> effective;
		//AreaGrowData* effective = new AreaGrowDataoutput.rows;
		for (int i = 0; i < inverted.rows; i++)
		{
			y_left = END;
			y_right = BEGIN;
			auto intensity_I = inverted.ptr<uchar>(i); // ��ȡ��i�е�ָ��
			auto intensity_O = output.ptr<uchar>(i); // ��ȡ��i�е�ָ��
			for (int j = 0; j < inverted.cols; j++)
			{
				if (intensity_O[j] == 0)
					continue;
				if (y_left > j)
					y_left = j;
				if (y_right < j)
					y_right = j;
				intensity_I[j] = 255 ^ intensity_O[j]; // ʹ��λ����������
			}
			if (y_left != END || y_right != BEGIN)
			{
				//effective[i].x = i;
				//effective[i].y_left = y_left;
				//effective[i].y_right = y_right;
				effective.emplace_back(effective_t{ i,y_left,y_right });
			}
		}

		for (int i = 0; i < output.rows; i++)
		{
			auto intensity_I = inverted.ptr<uchar>(effective[i].x); // ��ȡ��effective[i].x�е�ָ��
			auto intensity_R = result.ptr<uchar>(effective[i].x); // ��ȡ��effective[i].x�е�ָ��
			for (int j = effective[i].y_left + 2; j <= effective[i].y_right - 2; j++)
			{
				if (intensity_I[j] == 0 && intensity_R[j] == 0)
				{
					int T = 0;
					int around = 4;
					for (int k = 0; k < 4; k++)
					{
						if (j + direct[k] < effective[i].y_left || j + direct[k] > effective[i].y_right)
						{
							around--;
							continue;
						}

						T += intensity_I[j + direct[k]];
					}
					intensity_R[j] = T >> 2; // ʹ�����ƴ������
				}
				else
					intensity_R[j] = intensity_I[j];
			}
		}
	}
	else
		FLAG_INVERT = true;
	output = result.clone();
}

//�ԻҶ�ͼ�����ȥ��
void PreprocessingAlgorithm_A::deNoise() {
	cv::Mat result = output.clone();
	cv::Mat mark = cv::Mat::zeros(output.rows, output.cols, CV_8UC1);  //�����������
	std::vector<spot_t> spot;
	int num = 0;
	int around;
	std::vector<cv::Point2f> seeds;
	cv::Point2f seed;
	cv::Point2f seed_wait; //���������ӵ�
	constexpr float direct[9][2] = { {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {0,0} };  //9����
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
				seed = seeds.back();     //ȡ�����һ��Ԫ��
				seeds.pop_back();         //ɾ��ջ�����һ��Ԫ��,��ֹ�ظ�ɨ��
				around = 8;			//��8����ȥ��������
				for (int k = 0; k < 8; k++)    //�������ӵ��8����
				{
					seed_wait.x = seed.x + direct[k][0];    //��i������0�У���x����ֵ
					seed_wait.y = seed.y + direct[k][1];    //��i������1�У���y����ֵ
					if (seed_wait.x < 0 || seed_wait.y < 0 ||
						(seed_wait.x >= output.rows) || (seed_wait.y >= output.cols))
					{
						around--;
						continue;
					}

					flag = mark.at<uchar>(seed_wait.x, seed_wait.y);
					if (flag == 0)
					{
						if (output.at<uchar>(seed_wait.x, seed_wait.y) != 0)
						{
							num++;
							mark.at<uchar>(seed_wait.x, seed_wait.y) = 255; //flag=255��ʾ�õ��Ѿ������������еĵ㣬�������ж�
							seeds.emplace_back(seed_wait);    //���������������Ĵ��������ӵ��������ջ��
						}
						else                            //******************/
						{								//*			       */
							around--;					//*	 ȥ������������   */
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
		// ���a����b������true����ʾaӦ������bǰ�棬������
		return a.num > b.num;
	};
	std::sort(spot.begin(), spot.end(), cmp);
	mark = 0;
	for (int n = 2; n < spot.size(); n++)
	{
		if (spot[n].num < spot[0].num * THRESHOLD_NOISE)//���ȥ���ɾ����Խ�0.03�Ӵ�
		{
			seed.x = spot[n].x;
			seed.y = spot[n].y;
			seeds.emplace_back(seed);
		}
		while (!seeds.empty())
		{
			seed = seeds.back();     //ȡ�����һ��Ԫ��
			seeds.pop_back();         //ɾ��ջ�����һ��Ԫ��,��ֹ�ظ�ɨ��
			for (int k = 0; k < 9; k++)    //�������ӵ��8����
			{
				seed_wait.x = seed.x + direct[k][0];    //��i������0�У���x����ֵ
				seed_wait.y = seed.y + direct[k][1];    //��i������1�У���y����ֵ
				if (seed_wait.x < 0 || seed_wait.y < 0 ||
					(seed_wait.x >= output.rows) || (seed_wait.y >= output.cols))
					continue;
				flag = mark.at<uchar>(seed_wait.x, seed_wait.y);
				if (flag == 0)
				{
					if (output.at<uchar>(seed_wait.x, seed_wait.y) != 0)
					{
						result.at<uchar>(seed_wait.x, seed_wait.y) = 0;
						mark.at<uchar>(seed_wait.x, seed_wait.y) = 255; //flag=1��ʾ�õ��Ѿ������������еĵ㣬�������ж�
						seeds.emplace_back(seed_wait);    //���������������Ĵ��������ӵ��������ջ��
					}
				}
			}
		}
	}
	output = result.clone();
}

//ת��ͼ���ɫ�ʿռ��ɫ����ȵĺ���ʵ��
void PreprocessingAlgorithm_A::convertColor() {
	//���ͼ������ͺ�ͨ����
	if (data.type() != CV_8UC3) {
		throw std::invalid_argument("The input image must be of type CV_8UC3");
	}

	//�ջҶ�ͼ��
	cv::Mat result = cv::Mat::zeros(data.rows, data.cols, CV_8UC1);

	//����ͼ��ͨ��
	cv::Mat channel[3];//0->B,1->G,2->R
	cv::split(data, channel);

	//����ͼ��Ĳ���Ȩ��
	double weight[3] = { 0,0,0 };
	calculateWeight(channel, weight);

	//���ݲ���Ȩ�ؼ���Ҷ�ֵ
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
	output = result.clone();
}

//����ͼ�񲹳�Ȩ�غ�����ʵ��
void PreprocessingAlgorithm_A::calculateWeight(cv::Mat channel[3], double weight[3]) {
	//һЩͳ��������
	count_t count[3];//��ͨ���Ҷ�ռ��ͳ������eg count[B]Ϊ��ͼ������Bͨ���Ҷ�ռ��ͨ���Ҷ�֮�͵ı�
	count[B] = { B,0 };
	count[G] = { G,0 };
	count[R] = { R,0 };
	int light[3] = { 0,0,0 };//��ͨ��ͳ������eg light[0]Ϊ�õ��Bͨ��ǿ�����
	double compensation[3] = { 0,0,0 };//��һ�β��������ͨ��Ȩ��ͳ����
	double normalization[3] = { 0,0,0 };//��һ����һ�β���Ȩ��
	double templateWeight[3] = { 0.114,0.299,0.587 };//ģ��Ȩ��
	double re_tmpW[3] = { 0,0,0 };//�ط�����ģ��Ȩ��

	//ͳ��ԭʼ��ͨ����������ͨ���Ҷ�ռ��
	for (int i = 0; i < data.rows; i++)
	{	//��ʱָ�룬�����ͨ������ָ��
		auto intensity_B = channel[B].ptr<uchar>(i);
		auto intensity_G = channel[G].ptr<uchar>(i);
		auto intensity_R = channel[R].ptr<uchar>(i);
		for (int j = 0; j < data.cols; j++)
		{	//���ͼ��ǿ��С����ֵ������ͳ��
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

	//---------------------------------------------һ��Ȩ�ز���------------------------------------------------//

	//�ԻҶȽ���һ�β���
	(light[G] >= light[R]) ? light[G] *= 2 : light[R] *= 2;

	//��������ͼ��Ŀ�������ع�ϸ�
	if (light[G] + light[R] >= light[B] / 5)
	{
		//�ԻҶȽ��ж��β���
		(light[G] >= light[R]) ? light[R] *= 2 : light[G] *= 2;

		//��һ��Ȩ�ز�����������ɫͨ��Ȩ�أ�����������ͨ��Ȩ��
		compensation[B] = (light[B] - light[G] - light[R]) / 4;
		compensation[G] = count[G].weight + (count[B].weight - compensation[B]) / 2;
		compensation[R] = count[R].weight + (count[B].weight - compensation[B]) / 2;
		FLAG_INVERT = false;
	}
	//��������ͼ��Ŀ�������ع�ϵ�
	else
	{
		//Ȩ�ز���
		compensation[B] = count[B].weight;
		compensation[G] = count[G].weight;
		compensation[R] = count[R].weight;
	}

	//��һ����һ��Ȩ�ز���
	double compensation_sum = std::accumulate(compensation, compensation + 3, 0);
	for (int i = 0; i < 3; i++) {
		normalization[i] = compensation[i] / compensation_sum;
	}

	//---------------------------------------------����Ȩ�ز���------------------------------------------------//
	//��count���н�������
	auto cmp = [](count_t a, count_t b)-> bool {
		// ���a����b������true����ʾaӦ������bǰ�棬������
		return a.weight > b.weight;
	};
	std::sort(count, count + 3, cmp);//��������

	//���·���ģ��Ȩ��
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

	//����Ȩ�ز���
	weight[B] = (re_tmpW[B] + normalization[B]) / 2;
	weight[G] = (re_tmpW[G] + normalization[G]) / 2;
	weight[R] = (re_tmpW[R] + normalization[R]) / 2;
}