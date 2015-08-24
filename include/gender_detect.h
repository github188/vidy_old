//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File: label_detect.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_GENDER_DETECT_H_
#define VIDY_INC_GENDER_DETECT_H_

#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/contrib/contrib.hpp>
namespace vidy{

class GenderDetect{

public:
  GenderDetect();
  ~GenderDetect();

  //Note. Detect gender by facial recognition.
  int DetectByFace(cv::Mat face_gray); //1:male 0:female 2:N/A

private:
  cv::Ptr<cv::FaceRecognizer> model;
  cv::Ptr<cv::FaceRecognizer> fishermodel;
  cv::Mat eigenvalues;
  cv::Mat W;
  int xth;
  cv::Mat evs;
  cv::Mat mean;
  cv::Mat sample;

}; //Gender Detect

} //namespace vidy

#endif // VIDY_INC_Gender_DETECT_H_
