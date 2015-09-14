//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_generate.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_BLOB_GENERATE_H_
#define VIDY_INC_BLOB_GENERATE_H_

#include <iostream>
#include "blob.h"
#include "gender_detect.h"
#include "age_estimate.h"

namespace vidy{

class CBlobGenerate{
public:
  CBlobGenerate();
  ~CBlobGenerate();
  void Generate2(BlobNodeList& endBlobNodeList);
protected:
  int GetDirection(std::vector<cv::Rect> trajectory);
private:
  GenderDetect* genderdetect;
  AgeEstimate* ageestimate;
}; //class CBlobGenerate

} //namespace vidy

#endif //VIDY_INC_BLOB_GENERATE_H_
