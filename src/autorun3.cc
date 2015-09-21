//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:autorun3.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "autorun.h"

namespace vidy{

CAutoRun3::CAutoRun3(){
  this->Init();  
}

CAutoRun3::~CAutoRun3(){
  delete blobdetect;
  delete blobtrack;
  delete blobgenerate;
}

void CAutoRun3::Process(const cv::Mat frame){
  //-- use hog detect. --
  currentBlobNodeList=blobdetect->DetectPedestrian(frame);

  blobtrack->GetFrame(frame);
  blobtrack->Track2(&existBlobNodeList,currentBlobNodeList); 

  endBlobNodeList=blobtrack->GetEndBlobNodeList(); 

  if(endBlobNodeList.size()>0){
    blobgenerate->Generate3(endBlobNodeList);
  }

}

void CAutoRun3::Init(){
  blobdetect=new CBlobDetect();
  blobtrack=new CBlobTrack();
  blobgenerate=new CBlobGenerate();
}

} //namespace vidy
