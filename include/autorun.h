//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:autorun.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_AUTORUN_H_
#define VIDY_INC_AUTORUN_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "fg_detect.h"
#include "blob_detect.h"
#include "blob_track.h"
#include "blob_generate.h"

#include "global.h"

namespace vidy{

class IAutoRun{
public:
  virtual void Process(const cv::Mat frame)=0;
}; //class IAutoRun

class CAutoRun : public IAutoRun{
public:
  CAutoRun();
  ~CAutoRun();
  virtual void Process(const cv::Mat frame);
protected:
  void Init();
private:
  CBlobDetect* blobdetect;
  CBlobTrack* blobtrack;
  CBlobGenerate* blobgenerate;
  BlobNodeList existBlobNodeList;
  BlobNodeList currentBlobNodeList;
  BlobNodeList endBlobNodeList;
}; //class CAutoRun

class CAutoRun2 : public IAutoRun{
public:
  CAutoRun2();
  ~CAutoRun2();
  virtual void Process(const cv::Mat frame);
protected:
  void UpdateHeatmapData(cv::Mat result);
  void Print();
  void ClearPixel(){
    memset(staytime,0.00f,sizeof(staytime)); 
    memset(value,0,sizeof(value));
    memset(value,0,sizeof(num));
  }
private:
  std::vector<std::vector<cv::Point> > areas;
  double staytime[HEIGHT][WIDTH];
  int value[HEIGHT][WIDTH];
  int num[HEIGHT][WIDTH];
  cv::Mat preresult;
  cv::Mat background;
  cv::Mat background_gray;
  cv::Mat background_gray_roi;
  cv::Mat mask;
  int icurrenttime;
  int iprevioustime;
  int count;
}; //class CAutoRun2

class CAutoRun3 : public IAutoRun{
public:
  CAutoRun3();
  ~CAutoRun3();
  virtual void Process(const cv::Mat frame);
}; //class CAutoRun3

} //namespace vidy

#endif //VIDY_INC_AUTORUN_H_
