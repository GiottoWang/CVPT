#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// ����һ�� ImageGrayer �࣬����ʵ�ֲ�ɫͼ��ĻҶȻ�
class ImageGrayer
{
public:
	// ����һ�� enum ���ͣ���ʾ���ֻҶȻ����㷨
	enum Algorithm
	{
		FLOAT, // �����㷨
		INT,   // �����㷨
		SHIFT  // ��λ�㷨
	};
	ImageGrayer(string filename);
	~ImageGrayer();

	// ���õ�ǰ���㷨ѡ�񣬽���һ�� Algorithm ���͵Ĳ���
	void setAlgorithm(Algorithm alg);

	// ��ȡ��ǰ���㷨ѡ�񣬷���һ�� Algorithm ���͵�ֵ
	Algorithm getAlgorithm();

	// ���ݵ�ǰ���㷨ѡ��ʵ�ֲ�ɫͼ��ĻҶȻ�������һ���Ҷ�ͼ��
	Mat gray();

private:
	// ԭʼ��ɫͼ��
	Mat src;
	// �Ҷ�ͼ��
	Mat dst;
	// �㷨ѡ��
	Algorithm algorithm;
};
