//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:autorun2.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "autorun.h"

#include <time.h>
#include <string.h>
#include <iostream>
#include <fstream>


namespace vidy{

CAutoRun2::CAutoRun2(){
  //note : only one area.
  areas.push_back(g_calibrate);
  //create mask.
  mask=cv::Mat(cv::Size(WIDTH,HEIGHT),CV_8UC1);
  mask.zeros(cv::Size(WIDTH,HEIGHT),CV_8UC1);
  cv::drawContours(mask,areas,-1,cv::Scalar(255),CV_FILLED);
  
  icurrenttime=0;
  iprevioustime=0;  

  //clear pixel
  this->ClearPixel();

  count=FPS;
}

CAutoRun2::~CAutoRun2(){
  
}

void CAutoRun2::Process(const cv::Mat frame){
  count--;
  if(count==0){
    count=FPS;
    if(background.empty()){
      background=frame.clone();
      cv::cvtColor(background,background_gray,CV_RGB2GRAY);
      background_gray.copyTo(background_gray_roi,mask);
    }
    cv::Mat roi,roi_gray;
    frame.copyTo(roi,mask);
#ifdef DEBUG
    cv::imshow("roi",roi);
#endif
    cv::cvtColor(roi,roi_gray,CV_RGB2GRAY);
    cv::Mat result;
    cv::absdiff(roi_gray,background_gray_roi,result);
    cv::threshold(result,result,75,255,CV_THRESH_BINARY);

    //count 
    this->UpdateHeatmapData(result);  

    //update background.
    icurrenttime=atoi(g_time);
    if(g_time[8]=='1'&&g_time[9]=='5'){
      background=frame.clone();  
      cv::cvtColor(background,background_gray,CV_RGB2GRAY);
      background_gray.copyTo(background_gray_roi,mask);
    }

    //print data.
    if(iprevioustime!=icurrenttime){
      this->Print();
      this->ClearPixel();
    }

    //upate previous time.
    iprevioustime = icurrenttime;
  }
}

void CAutoRun2::Print(){
  //print data to local file.
  std::string file="heatmap";
  file += g_time;
  file += ".dat";
  std::ofstream outfile(file.data(),std::ios::app);
  for(unsigned int i=0;i<HEIGHT;i++){
    for(unsigned int j=0;j<WIDTH;j++){
      if(staytime[i][j]!=0){
        outfile<<i<<" "<<j<<" "<<staytime[i][j]<<std::endl;
      }
    }
  }
}

void CAutoRun2::UpdateHeatmapData(cv::Mat result){
  for(unsigned int i=0; i<result.rows;i++){
    uchar* data=result.ptr<uchar>(i);
    uchar* predata=preresult.ptr<uchar>(i);
    for(unsigned int j=0;j<result.cols;j++){
      if(data[j]!=0&&predata[j]!=0){
        value[i][j]++;
      }else{
        if(value[i][j]>LOWEST_STAYTIME){
          num[i][j]++;
          staytime[i][j]=(staytime[i][j]*(num[i][j]-1)+value[i][j])/num[i][j];
          value[i][j]=0;
        }
      }
    }
  }
  //update preresult;
  preresult=result.clone();
}





} //namespace vidy
