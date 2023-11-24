#include "ExtractAlgorithm.h"



int main(int argc, char** argv)
{
	for (int i =1; i < 600; i++)
	{
		string str_srcImg = format("E:\\lyle\\Desktop\\zhuanli\\pic1101\\%d.jpg",i);//图片编号4位用 \\%04d
		//string str_srcImg = format("D:\\ExtractStripeCenterline\\OriginalLaserStripe\\yaoshi\\%d.jpg",i);//图片编号4位用 \\%04d

		/*str_dstImg_chafen = format("D:\\ExtractStripe\\OutputCenterline\\chafen\\%d.jpg",i);
		Hui_Feng_WANG_yuchuli= format("D:\\ExtractStripe\\OutputCenterline\\Hui_Feng_WANG_yuchuli\\%d.jpg",i);

		lrh_tuxaingRonghe= format("D:\\ExtractStripe\\OutputCenterline\\tuxaingRonghe\\%d.jpg",i);
		lrh_Ronghechafen= format("D:\\ExtractStripe\\OutputCenterline\\rongHechafen\\%d.jpg",i);

		lrh_xinsuanfachafen= format("D:\\ExtractStripe\\OutputCenterline\\xinsuanfa9_12\\%d.jpg",i);
	    lrh_xinsuanfachafen2= format("D:\\ExtractStripe\\OutputCenterline\\xinsuanfa9_13\\%d.jpg",i);
		lrh_color= format("D:\\ExtractStripe\\OutputCenterline\\lrh_color\\%d.jpg",i);
	    lrh_xinsuanfachafen3= format("D:\\ExtractStripe\\OutputCenterline\\xinsuanfa9_14\\%d.jpg",i);
		lrh_xinsuanfachafen4= format("D:\\ExtractStripe\\OutputCenterline\\xinsuanfa9_14_1\\%d.jpg",i);

		lrh_9_15_hebingchefen= format("D:\\ExtractStripe\\OutputCenterline\\lrh_9_15_hebingchefen\\%d.jpg",i);
		lrh_9_15_lvbohuidu= format("D:\\ExtractStripe\\OutputCenterline\\lrh_9_15_lvbohuidu\\%d.jpg",i);
		lrh_9_15_hebinghuidu= format("D:\\ExtractStripe\\OutputCenterline\\lrh_9_15_hebinghuidu\\%d.jpg",i);


		lrh_9_15_jingbihuidu= format("D:\\ExtractStripe\\OutputCenterline\\lrh_9_15_jingbihuidu\\%d.jpg",i);
		lrh_9_15_jingbichafen= format("D:\\ExtractStripe\\OutputCenterline\\lrh_9_15_jingbichafen\\%d.jpg",i);
*/
		string lrh_ceshi_1 = format("D:\\\ExtractStripeCenterline\\OutputCenterline\\lrh_ceshi_1\\%d.jpg",i);
		string lrh_ceshi_2 = format("D:\\\ExtractStripeCenterline\\OutputCenterline\\lrh_ceshi_2\\%d.jpg",i);
		string lrh_jinshu_1 = format("D:\\\ExtractStripeCenterline\\OutputCenterline\\lrh_jinshu_1\\%d.jpg",i);
		string lrh_jinshu_2 = format("D:\\\ExtractStripeCenterline\\OutputCenterline\\lrh_jinshu_2\\%d.jpg",i);

		string lrh_yaoshi_1 = format("D:\\\ExtractStripeCenterline\\OutputCenterline\\lrh_yaoshi_1\\%d.jpg",i);
		string lrh_yaoshi_2 = format("D:\\\ExtractStripeCenterline\\OutputCenterline\\lrh_yaoshi_2\\%d.jpg",i);
		string lrh_yaoshi_3 = format("D:\\\ExtractStripeCenterline\\OutputCenterline\\lrh_yaoshi_3\\%d.jpg",i);

		string lrh_gray1 = format("E:\\lyle\\Desktop\\ppt\\image\\lrh\\gray2\\%d.jpg",i);
		string lrh_gray2 = format("E:\\lyle\\Desktop\\ppt\\image\\lrh\\gray3\\%d.jpg",i);
		string lrh_DST1 = format("E:\\lyle\\Desktop\\ppt\\image\\lrh\\lrh_DST1\\%d.jpg",i);
		string lrh_zhijiehuidu = format("E:\\lyle\\Desktop\\ppt\\image\\lrh\\lrh_zhijiehuidu\\%d.jpg",i);


		string zhijiehuidu = format("E:\\lyle\\Desktop\\zhuanli\\zhijiehuidu\\%d.jpg",i);
		

		string lvbo1 = format("E:\\lyle\\Desktop\\zhuanli\\lvbo1\\%d.jpg",i);
		string yuzhi = format("E:\\lyle\\Desktop\\zhuanli\\yuzhi\\%d.jpg",i);
		string lvbo2 = format("E:\\lyle\\Desktop\\zhuanli\\lvbo2\\%d.jpg",i);
		string chafen = format("E:\\lyle\\Desktop\\zhuanli\\chafen\\%d.jpg",i);
		string dst_ = format("E:\\lyle\\Desktop\\zhuanli\\dst_\\%d.jpg",i);
		string dst1101 = format("E:\\lyle\\Desktop\\zhuanli\\dst1101\\%d.jpg",i);
	
		Mat srcColorImage = imread(str_srcImg);

		Mat SingleChannelImage = Mat::zeros(srcColorImage.rows, srcColorImage.cols, CV_8UC1);

		Mat dst = Mat::zeros(srcColorImage.rows, srcColorImage.cols, CV_8UC1);

		CExtractAlgorithm m_LaserAlgorithm;


		//////////////////////************纯差分**************/////////////////
		/*Mat srcColorImage1 = Mat::zeros(srcColorImage.rows, srcColorImage.cols, CV_8UC1);
		cvtColor(srcColorImage, srcColorImage1, COLOR_BGR2GRAY);
		imwrite(zhijiehuidu,srcColorImage1);*/
		//m_LaserAlgorithm.detectResult_sjw_vertical(srcColorImage1,dst,3); 
		//imwrite(chafen,dst);
		//////////////////////********************************/////////////////

		//1.处理反射光，提取ROI区域，阈值t取值范围t*0.01；金属1；塑料不反光50；
		m_LaserAlgorithm.splitChannelExtractROI(srcColorImage,SingleChannelImage);
	//	imwrite(lvbo1,SingleChannelImage);

		//2.弱化光斑，分割
		medianBlur(SingleChannelImage, SingleChannelImage, 7);
		threshold(SingleChannelImage, SingleChannelImage, 8, 255, cv::THRESH_TOZERO);
	//	imwrite(yuzhi,SingleChannelImage);
		
	
		//3.去噪，二次8连通域面积去除大面积漫反射
		m_LaserAlgorithm.connectedComponentFilter_8(SingleChannelImage,SingleChannelImage);//5,10
	//	imwrite(lvbo2,SingleChannelImage);

		//4.差分并平滑曲线
		m_LaserAlgorithm.detectResult_sjw_vertical(SingleChannelImage,dst,5); 
		imwrite(dst1101,dst);


		cout << i <<endl;

		srcColorImage.release();

		SingleChannelImage.release();

		dst.release();
	}

	waitKey(0);
	return 0;
}

