//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File: age_estimate.h
//Author: Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_AGE_ESTIMATE_H_
#define VIDY_INC_AGE_ESTIMATE_H_

#include <fstream>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "svm.h"

namespace vidy{

class AgeEstimate{

public:
  AgeEstimate();
  ~AgeEstimate();

  //@return: age.
  int EstimateByFace(cv::Mat face_original);

protected:
  void LoadLDA(double *LDAMatrix);

  void LDAFeature(int *graydata, double *LDAMatrix, svm_node *svm_node);

private:
  int* graydata;
  svm_node* node;
  svm_model* model;
  double* LDAMatrix;

}; //Age Estimate. 

} //namespace vidy.

#endif

