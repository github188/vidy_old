//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:autorun.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "autorun.h"
#include "global.h"

namespace vidy{

CAutoRunTestView1::CAutoRunTestView1(){
  this->Init();
}

CAutoRunTestView1::~CAutoRunTestView1(){
  delete blobdetect;
  delete blobtrack;
  delete blobgenerate;
}

void CAutoRunTestView1::Process(const cv::Mat frame){
  cv::Rect r(400,150,500,400);
  cv::Mat roi_detect(frame,r);
  cv::Mat roi=roi_detect.clone();

  //--use upper body. --
  //currentBlobNodeList=blobdetect->DetectUpperBody2(frame);
  //--use face.--
  currentBlobNodeList=blobdetect->DetectFace2(roi);
 
  blobtrack->GetFrame(roi);
  //--use upper body. --
  //blobtrack->Track2(&existBlobNodeList,currentBlobNodeList);
  //--use face --
  blobtrack->TrackFace(&existBlobNodeList,currentBlobNodeList);
  endBlobNodeList=blobtrack->GetEndBlobNodeList();

#ifdef TESTVIEW
  if(endBlobNodeList.size()>0){
    blobgenerate->GenerateTestView(endBlobNodeList);
  }
#endif // TESTVIEW
  return ;
}

void CAutoRunTestView1::Init(){
  blobdetect=new CBlobDetect();
  blobtrack=new CBlobTrack();
  blobgenerate=new CBlobGenerate();
}

} //namespace vidy
