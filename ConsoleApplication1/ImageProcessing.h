#pragma once
#include "PreprocessingAlgorithm.h"
#include "CenterLineAlgorithm.h"
typedef PreprocessingAlgorithm* PA;
typedef CenterLineAlgorithm* CA;
typedef PreprocessingAlgorithm_A PA_A;
class Preprocess {
public:
	Preprocess(PA _p=nullptr, CA _q= nullptr) : ptr_PA(_p), ptr_CA(_q){};
	void setPA(PA _p, CA _q) {
		ptr_PA = _p;
		ptr_CA = _q;
	}
	cv::Mat execute() {
		return ptr_PA->process();
		//cv::Mat preprocessed = ptr_PA->process();
		//cv::Mat result = ptr_CA->process();
		//return result;
	}
private:
	PA ptr_PA;
	CA ptr_CA;

};