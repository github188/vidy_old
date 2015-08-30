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

  cv::Rect r(g_calibrate[0].x-50,g_calibrate[0].y-280,g_calibrate[1].x-g_calibrate[0].x+100,400);
  cv::Mat roi_detect(frame,r);
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

void CAutoRun::Init(){
  blobdetect=new CBlobDetect();
  blobtrack=new CBlobTrack();
  blobgenerate=new CBlobGenerate();
}

} //namespace vidy
