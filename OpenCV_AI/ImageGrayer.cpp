#include "ImageGrayer.h"
// ���캯��������һ����ɫͼ����ļ�����Ϊ��������ȡ������ͼ��
ImageGrayer::ImageGrayer(string filename)
{
	src = imread(filename);
	if (src.empty())
	{
		// �׳�һ���쳣����ʾ�޷���ȡͼ���ļ�
		throw runtime_error("Cannot read image file.");
	}
	// Ĭ��ʹ�ø����㷨
	algorithm = FLOAT;
}

// �����������ͷ�ͼ����Դ
ImageGrayer::~ImageGrayer()
{
	src.release();
	dst.release();
}

// ���õ�ǰ���㷨ѡ�񣬽���һ�� Algorithm ���͵Ĳ���
void ImageGrayer::setAlgorithm(Algorithm alg)
{
	algorithm = alg;
}

// ��ȡ��ǰ���㷨ѡ�񣬷���һ�� Algorithm ���͵�ֵ
ImageGrayer::Algorithm ImageGrayer::getAlgorithm()
{
	return algorithm;
}

// ���ݵ�ǰ���㷨ѡ��ʵ�ֲ�ɫͼ��ĻҶȻ�������һ���Ҷ�ͼ��
Mat ImageGrayer::gray()
{
	// ����һ����ԭʼͼ���С��ͬ�ĻҶ�ͼ��
	dst = Mat(src.rows, src.cols, CV_8UC1);

	// ����ÿ�����أ�����Ҷ�ֵ
	for (int i = 0; i < src.rows; i++)
	{

		//uchar *src_data = src.ptr<uchar>(i);// ��ȡԭʼͼ��ĵ� i �е�����ָ��

		//uchar *dst_data = dst.ptr<uchar>(i);// ��ȡ�Ҷ�ͼ��ĵ� i �е�����ָ��

		for (int j = 0; j < src.cols; j++)

		{
			auto *src_data = src.ptr<Vec3b>(i, j);
			auto *dst_data = dst.ptr<uchar>(i, j);

			//int src_offset = j * 3;//����ԭʼͼ��ĵ� i �е� j �е����ص�ƫ����

			//int dst_offset = j;// ����Ҷ�ͼ��ĵ� i �е� j �е����ص�ƫ����

			// ��ȡԭʼͼ���ÿ�����ص�������ɫ����
			uchar b = src_data->val[0];// src_data[src_offset];
			uchar g = src_data->val[1];// src_data[src_offset + 1];
			uchar r = src_data->val[2];// src_data[src_offset + 2];

			// ���ݵ�ǰ���㷨ѡ�����Ҷ�ֵ
			int gray = 0;
			switch (algorithm)
			{
			case FLOAT: // �����㷨
				gray = 0.299 * r + 0.587 * g + 0.114 * b;
				break;
			case INT: // �����㷨
				gray = (r * 30 + g * 59 + b * 11) / 100;
				break;
			case SHIFT: // ��λ�㷨
				gray = (r * 28 + g * 151 + b * 77) >> 8;
				break;
			default:
				// �׳�һ���쳣����ʾ��Ч���㷨ѡ��
				throw invalid_argument("Invalid algorithm choice.");
			}

			// ���Ҷ�ֵ�����Ҷ�ͼ��Ķ�Ӧ����
			*dst_data = gray;
		}
	}

	// ���ػҶ�ͼ��
	return dst;
}
