//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:autorun.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "autorun.h"
#include "global.h"

namespace vidy{

CAutoRun::CAutoRun(){
  this->Init();
}

CAutoRun::~CAutoRun(){
  delete blobdetect;
  delete blobtrack;
  delete blobgenerate;
}

void CAutoRun::Process(const cv::Mat frame){
 
  cv::Mat roi_detect(frame,roi_rect);
  cv::Mat roi=roi_detect.clone();

  //--use upper body. --
  //currentBlobNodeList=blobdetect->DetectUpperBody2(roi);
  //--use face.--
  currentBlobNodeList=blobdetect->DetectFace2(roi);
 
  blobtrack->GetFrame(roi);
  //--use upper body. --
  //blobtrack->Track2(&existBlobNodeList,currentBlobNodeList);
  //--use face --
  blobtrack->TrackFace(&existBlobNodeList,currentBlobNodeList);
  endBlobNodeList=blobtrack->GetEndBlobNodeList();

  if(endBlobNodeList.size()>0){
    blobgenerate->Generate2(endBlobNodeList);
  }
  return ;
}

void CAutoRun::GetROI(){
  //order the two points.(from left to right)
  if(g_calibrate[1].x-g_calibrate[0].x<0){
    cv::Point tmp = g_calibrate[0];
    g_calibrate[0] = g_calibrate[1];
    g_calibrate[1] = tmp;
  }

  int r_x = g_calibrate[0].x-50<0?0:(g_calibrate[0].x-50);
  int r_y = g_calibrate[0].y-280<0?0:(g_calibrate[0].y-280);
  int r_w = (g_calibrate[1].x-g_calibrate[0].x+100)>WIDTH?WIDTH:(g_calibrate[1].x-g_calibrate[0].x+100);                                      

  cv::Rect r(r_x,r_y,r_w,400);

  roi_rect = r;

}

void CAutoRun::GetPathways(){

}

void CAutoRun::Init(){
  blobdetect=new CBlobDetect();
#ifdef DEBUG
  std::cout<<"CBlobDetect() finish.."<<std::endl;
#endif
  blobtrack=new CBlobTrack();
#ifdef DEBUG
  std::cout<<"CBlobTrack() finish.."<<std::endl;
#endif
  blobgenerate=new CBlobGenerate();
#ifdef DEBUG
  std::cout<<"CBlobGenerate) finish.."<<std::endl;
#endif
  this->GetROI();
}

} //namespace vidy
