//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:fg_detect.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "fg_detect.h"

#include <vector>

#define THRESHOLD 0

namespace vidy{

CFGDetect::CFGDetect(){
}

CFGDetect::~CFGDetect(){
}

void CFGDetect::Process(const cv::Mat& frame,cv::Mat* mask){
  //get background_gray.
  if(!getbackgroundgray){
    cv::cvtColor(frame,background_gray,CV_BGR2GRAY);
    getbackgroundgray=true;
  }
  cv::cvtColor(frame,frame_gray,CV_BGR2GRAY);
  cv::absdiff(background_gray,frame_gray,foreground);
  //cv::threshold(foreground,foreground,THRESHOLD,100,CV_THRESH_BINARY);
#ifdef DEBUG
  cv::imshow("foreground",foreground);
  Mat edge;
  cv::Canny(foreground,edge,180,255,3);
  cv::imshow("canny",edge);
#endif //DEBUG
  *mask=foreground;
}

void CFGDetect::ProcessMOG(const cv::Mat& frame,cv::Mat* mask){
  cv::Mat _mask;
  mog(frame,_mask,0.1);
  for(int i=0;i<3;i++){
    cv::dilate(_mask,_mask,cv::Mat(),cv::Point(1,1),1);
    cv::erode(_mask,_mask,cv::Mat(),cv::Point(1,1),1);
  }
  std::vector<std::vector<cv::Point> > contours,contours2;
  cv::findContours(_mask,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
  std::vector<std::vector<cv::Point> >::iterator it=contours.begin();
  while(it!=contours.end()){
    cv::Rect r0=boundingRect(*it);
    if(it->size()>30){
      contours2.push_back(*it);
      ++it;
    }else{
      ++it;
    }
  }
  cv::drawContours(*mask,contours2,-1,cv::Scalar(255),CV_FILLED);
#ifdef DEBUG
  cv::imshow("foreground",*mask);
#endif //DEBUG
}

void CFGDetect::Init(){
  getbackgroundgray=false;
}

}//namespace vidy
