#include "imgprog.hpp"

using namespace ip;
int main(int argc, char* argv[]) {
	using std::cout;
	using std::endl;
	//cv::Mat img = cv::imread("D:\\vsWS\\CVPT\\ConsoleApplication2\\test.jpg", -1);
	cv::Mat img = cv::imread(argv[1], -1);//argv�������ж�ȡ�ַ���������������->����->�������������
	if (img.empty())return -1;
	show(img, std::string(argv[1]), re_Title);
	cout << re_Title(string(argv[1]));

}