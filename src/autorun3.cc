//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:autorun3.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "autorun.h"

namespace vidy{

CAutoRun3::CAutoRun3(){
  this->Init();
  control_frame++;  
}

CAutoRun3::~CAutoRun3(){
  delete blobdetect;
  delete blobtrack;
  delete blobgenerate;
}

void CAutoRun3::Process(const cv::Mat frame){
#ifdef DEBUG
  cv::Mat showframe = frame.clone();
#endif // DEBUG
  control_frame++;

  currentBlobNodeList.clear();

  //-- use hog detect. --
  //currentBlobNodeList=blobdetect->DetectPedestrian(frame);

  cv::Mat mask = cv::Mat::zeros(frame.size(),CV_8UC1);
  fgdetect->ProcessMOG(frame,&mask);

  std::vector<std::vector<cv::Point> > contours;
  cv::findContours(mask,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
  cv::vector<std::vector<cv::Point> >::iterator it=contours.begin();
  while(it!=contours.end()){
    cv::Rect r0 = boundingRect(*it);
    if(r0.width>r0.height||it->size()<500){
      it = contours.erase(it);
    }else{
      Blob blob;
      blob.x = r0.x;
      blob.y = r0.y;
      blob.w = r0.width;
      blob.h = r0.height;
      blob.image = frame(r0).clone();
      BlobNode blobnode(blob);
      currentBlobNodeList.push_back(blobnode);
#ifdef DEBUG
      cv::rectangle(showframe,r0,cv::Scalar(0,255,0),2);
#endif // DEBUG
      ++it;
    }
  }

  blobtrack->GetFrame(frame);
  blobtrack->Track2(&existBlobNodeList,currentBlobNodeList); 

  endBlobNodeList=blobtrack->GetEndBlobNodeList();
#ifdef DEBUG
  std::cout<<"endBlobNodeList.size():"<<endBlobNodeList.size()<<std::endl;
#endif 

  if(endBlobNodeList.size()>0){
    if(control_frame>5){
      blobgenerate->Generate3(endBlobNodeList);
    }
    control_frame=0;
  }
#ifdef DEBUG
  cv::imshow("showframe",showframe);
#endif // DEBUG

}

void CAutoRun3::Init(){
  fgdetect = new CFGDetect();
  blobdetect=new CBlobDetect();
  blobtrack=new CBlobTrack();
  blobgenerate=new CBlobGenerate();
}

} //namespace vidy
