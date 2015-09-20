//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:autorun.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "autorun.h"
#include "global.h"
#include "opencv2/imgproc/imgproc.hpp"

namespace vidy{

CAutoRun::CAutoRun(){
  this->Init();
  control_frame=0;
}

CAutoRun::~CAutoRun(){
  delete blobdetect;
  delete blobtrack;
  delete blobgenerate;
}

void CAutoRun::Process(const cv::Mat frame){
  control_frame++;

  currentBlobNodeList.clear();

  //cv::Mat showframe = frame.clone();
 
  cv::Mat roi_detect(frame,roi_rect);

  cv::Mat showframe = roi_detect.clone();
  
  cv::Mat mask = cv::Mat::zeros(frame.size(),CV_8UC1);
  fgdetect->ProcessMOG(frame,&mask);
  
  cv::Mat roi = mask(g_roi);

  int nonzero = cv::countNonZero(roi);
  
  if(nonzero>3000){
  std::vector<std::vector<cv::Point> > contours;
  cv::findContours(roi,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
  cv::vector<std::vector<cv::Point> >::iterator it=contours.begin();
  int r_min_x = 1000;
  int r_max_x = 0;
  int r_min_y = 1000;
  int r_max_y = 0;
  while(it!=contours.end()){
    cv::Rect r0 = boundingRect(*it);
    if(r0.x<r_min_x){
      r_min_x = r0.x;
    }
    if(r0.y<r_min_y){
      r_min_y = r0.y;
    }
    if(r0.x+r0.width>r_max_x){
      r_max_x = r0.x+r0.width;
    }
    if(r0.y+r0.height>r_max_y){
      r_max_y = r0.y+r0.height;
    }
    ++it;
  }
  cv::Rect rect(r_min_x,r_min_y,r_max_x-r_min_x,r_max_y-r_min_y);
  if(rect.width<rect.height){
    cv::rectangle(showframe,rect,cv::Scalar(0,255,0),2);
    Blob blob;
    blob.x = rect.x;
    blob.y = rect.y;
    blob.w = rect.width;
    blob.h = rect.height;
    blob.image = frame(rect).clone();
    BlobNode blobnode(blob);
    currentBlobNodeList.push_back(blobnode);
  }
  }
#ifdef DEBUG
  cv::imshow("showframe",showframe);
#endif // DEBUG

  //--use upper body. --
  //currentBlobNodeList=blobdetect->DetectUpperBody2(roi);
  //--use face. --
  //currentBlobNodeList=blobdetect->DetectFace2(roi);
  //--use hog. --
  //currentBlobNodeList=blobdetect->DetectPedestrian(roi_detect);
 
  blobtrack->GetFrame(roi_detect);
  //--use upper body. --
  blobtrack->Track2(&existBlobNodeList,currentBlobNodeList);
  //--use face --
  //blobtrack->TrackFace(&existBlobNodeList,currentBlobNodeList);

  endBlobNodeList=blobtrack->GetEndBlobNodeList();

  if(endBlobNodeList.size()>0){
    if(control_frame>5){
      blobgenerate->Generate2(endBlobNodeList);
    }
    control_frame=0;
  }
  return ;
}

void CAutoRun::GetROI(){

  //order two points.(from left to right)
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
  
  g_roi = r;

  //roi_rect = cv::Rect(0,0,1280,720);
  //g_roi = roi_rect;

}

void CAutoRun::Init(){
  fgdetect = new CFGDetect();

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
  std::cout<<"CBlobGeneratei() finish.."<<std::endl;
#endif

  //get roi area for analysis.
  this->GetROI();

}

} //namespace vidy
