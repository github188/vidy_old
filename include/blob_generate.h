//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_generate.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_BLOB_GENERATE_H_
#define VIDY_INC_BLOB_GENERATE_H_

#include <iostream>
#include <string>
#include "blob.h"
#include "gender_detect.h"
#include "age_estimate.h"

#include “heatmap.h"

namespace vidy{

class CBlobGenerate{
public:
  CBlobGenerate();
  ~CBlobGenerate();
  void Generate2(BlobNodeList& endBlobNodeList);

  //generate result for type 3 camera.
  void Generate3(BlobNodeList& endBlobNodeList);

protected:
  int GetDirection(std::vector<cv::Rect> trajectory);

  //get pathway order result for type 2&&3 camera.
  int GetDirection2(std::vector<cv::Rect> trajectory);

  //get staytime result for type 2&&3 camera.
  int GetStayTime(std::vector<std::string> time_sequence);

  //get heatmap(x,y,stay_time_value) result for type 2&&3 camera.
  std::vector<heatmap> GetHeatmapResult(std::vector<std::string> time_sequence,std::vector<cv::Rect> trajectory);
  

private:
  GenderDetect* genderdetect;
  AgeEstimate* ageestimate;
}; //class CBlobGenerate

} //namespace vidy

#endif //VIDY_INC_BLOB_GENERATE_H_
