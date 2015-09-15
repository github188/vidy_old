//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:fg_detect.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_FG_DETECT_H_
#define VIDY_INC_FG_DETECT_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/video/background_segm.hpp>

#include "blob.h"

namespace vidy{

class CFGDetect{
public:
  CFGDetect();
  ~CFGDetect();
  void Process(const cv::Mat& frame,cv::Mat* mask);
  void ProcessMOG(const cv::Mat& frame,cv::Mat* mask);
protected:
  void Init();
private:
  cv::Mat background_gray,frame_gray,foreground;
  bool getbackgroundgray;
  cv::BackgroundSubtractorMOG2 mog;

};//class FGDetect

} //namespace vidy

#endif
