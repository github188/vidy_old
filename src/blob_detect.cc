//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_detect.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "blob_detect.h"
#include <math.h>
#include <iostream>

#define HMAX 150
#define HMIN 10
#define WMAX 150
#define WMIN 10

namespace vidy{

CBlobDetect::CBlobDetect(){
  this->Init();
  background=cv::imread("background.png");
  //cv::cvtColor(background,background_gray,CV_RGB2GRAY);
}

CBlobDetect::~CBlobDetect(){

}

void CBlobDetect::DetectHeadsFromUpperside(const cv::Mat frame,const cv::Mat mask){

}



BlobNodeList CBlobDetect::DetectFace(const cv::Mat frame){
#ifdef DEBUG
  cv::Mat showframe=frame.clone();
#endif //DEBUG
  std::vector<cv::Rect> faces;
  cv::Mat frame_gray=frame.clone();
  cv::cvtColor(frame_gray,frame_gray,CV_RGB2GRAY);
  face_cascade.detectMultiScale(frame_gray,faces,1.1,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(30,30));
  BlobNodeList currentBlobNodeList;
  for(unsigned int i=0;i<faces.size();i++){
    Blob blob;
    blob.x=faces[i].x;
    blob.y=faces[i].y;
    blob.w=faces[i].width;
    blob.h=faces[i].height;
    blob.image=frame(faces[i]).clone();
    cv::cvtColor(blob.image,blob.image,CV_RGB2GRAY);
    BlobNode blobnode(blob);
    currentBlobNodeList.push_back(blobnode);
#ifdef DEBUG
    cv::rectangle(showframe,faces[i],cv::Scalar(0,255,0),3);
#endif //DEBUG
  }
#ifdef DEBUG
  cv::imshow("detect face",showframe);
#endif //DEBUG
  return currentBlobNodeList;
}

BlobNodeList CBlobDetect::DetectUpperBody(const cv::Mat frame){
#ifdef DEBUG
  cv::Mat showframe2=frame.clone();
#endif //DEBUG
  std::vector<cv::Rect> upperbodies;
  cv::Mat frame_gray=frame.clone();
  cv::cvtColor(frame_gray,frame_gray,CV_RGB2GRAY);
  upperbody_cascade.detectMultiScale(frame_gray,upperbodies,1.1,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(30,30));
  BlobNodeList currentBlobNodeList;
  for(unsigned int i=0;i<upperbodies.size();i++){
    Blob blob;
    blob.x=upperbodies[i].x;
    blob.y=upperbodies[i].y;
    blob.w=upperbodies[i].width;
    blob.h=upperbodies[i].height;
    blob.image=frame(upperbodies[i]).clone();
    cv::cvtColor(blob.image,blob.image,CV_RGB2GRAY);
    BlobNode blobnode(blob);
    currentBlobNodeList.push_back(blobnode);
#ifdef DEBUG
    cv::rectangle(showframe2,upperbodies[i],cv::Scalar(0,255,0),3);
#endif //DEBUG
  }
#ifdef DEBUG
  cv::imshow("detect upperbody",showframe2);
#endif //DEBUG
  return currentBlobNodeList;
}

PreBlob CBlobDetect::FindPreBlobByCurrentPreBlob(PreBlob current_preblob){
  PreBlob preblob;
  //find the nearest preblob.
  int index=-1;
  float distance=50.00;
  std::vector<PreBlob>::iterator it=pre_preblob_list.begin();
  int i=-1;
  while(it!=pre_preblob_list.end()){
    i++;
    float dis_x=abs(it->blob.x+0.5*it->blob.w-current_preblob.blob.x-0.5*current_preblob.blob.w);
    float dis_y=abs(it->blob.y+0.5*it->blob.w-current_preblob.blob.y-0.5*current_preblob.blob.h);
    float dis=sqrt(dis_x*dis_x+dis_y*dis_y);
    if(dis<distance){
      distance=dis;
      index=i;
    }
    ++it;
  }
  if(index!=-1){
    preblob=pre_preblob_list[index];
    //pre_preblob_list.erase(pre_preblob_list.begin()+index);
    //std::cout<<preblob.pre_index<<std::endl;
    return preblob;
  }else{
    preblob.pre_index=-1;
    return preblob;
  }
}

BlobNodeList CBlobDetect::DetectUpperBody2(const cv::Mat frame){
  preblob_list.clear();
#ifdef DEBUG
  cv::Mat showframe3=frame.clone();
#endif //DEBUG
  std::vector<cv::Rect> upperbodies;
  cv::Mat frame_gray=frame.clone();
  cv::cvtColor(frame_gray,frame_gray,CV_RGB2GRAY);
  upperbody_cascade.detectMultiScale(frame_gray,upperbodies,1.1,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(30,30));
  BlobNodeList currentBlobNodeList;
  for(unsigned int i=0;i<upperbodies.size();i++){
    //inorge too big or too small
    if(upperbodies[i].width>HMAX||
      upperbodies[i].width<WMIN||
      upperbodies[i].height>HMAX||
      upperbodies[i].height<HMIN){
      continue;
    }
    //detect face.
    /*std::vector<cv::Rect> faces;
    cv::Mat face_gray(frame_gray,upperbodies[i]);
    face_cascade.detectMultiScale(face_gray,faces,1.1,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(30,30));
    if(faces.size()<1){
      //continue;
    }else{
#ifdef DEBUG
        cv::rectangle(showframe3,cv::Rect(faces[0].x+upperbodies[i].x,faces[0].y+upperbodies[i].y,faces[0].width,faces[0].height),cv::Scalar(0,0,255),3);
#endif
    }*/
    PreBlob preblob;
    preblob.blob.x=upperbodies[i].x;
    preblob.blob.y=upperbodies[i].y;
    preblob.blob.w=upperbodies[i].width;
    preblob.blob.h=upperbodies[i].height;
    preblob.blob.image=frame(upperbodies[i]).clone();
    //change pre_index.
    preblob.pre_index=0;
    PreBlob previous_preblob=this->FindPreBlobByCurrentPreBlob(preblob);
    if(previous_preblob.pre_index==PREBLOBNUM+1){
      preblob.pre_index=PREBLOBNUM+1;
    }else{
      preblob.pre_index=(previous_preblob.pre_index)+1;
    }
#ifdef DEBUG
    std::cout<<preblob.pre_index<<std::endl;
#endif
    preblob_list.push_back(preblob);
#ifdef DEBUG
    if(preblob.pre_index==PREBLOBNUM){
      cv::rectangle(showframe3,upperbodies[i],cv::Scalar(0,255,0),3);
    }else{
      //putText(showframe3,"Male",Point(upperbodies[i].x,upperbodies[i].y-5),FONT_HERSHEY_SIMPLEX,0.5,Scalar(255,0,0),1);
      cv::rectangle(showframe3,upperbodies[i],cv::Scalar(255,0,0),3);
    }
#endif //DEBUG
    if(preblob.pre_index==PREBLOBNUM){
      cv::cvtColor(preblob.blob.image,preblob.blob.image,CV_RGB2GRAY);
      BlobNode blobnode(preblob.blob);
      currentBlobNodeList.push_back(blobnode);
    }
  }
  pre_preblob_list=preblob_list; //update pre_preblob_list.
#ifdef DEBUG
  cv::imshow("detect upperbody2",showframe3);
#endif //DEBUG
  return currentBlobNodeList;
}

void CBlobDetect::Init(){
#ifdef SERVER
  //init HOG detect.
  people_detect_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
  //init face detect.
  std::string face_cascade_name = "/root/vidy/data/haarcascades/haarcascade_frontalface_alt.xml";
  if( !face_cascade.load( face_cascade_name ) ){
    printf("--(!)Error loading\n");
  };
  //init upper body detect.
  std::string upperbody_cascade_name = "/root/vidy/data/haarcascades/haarcascade_mcs_upperbody.xml";
  if( !upperbody_cascade.load(upperbody_cascade_name ) ){
    printf("--(!)Error loading\n");
  };
#else
  //init HOG detect.
  people_detect_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
  //init face detect.
  std::string face_cascade_name = "../data/haarcascades/haarcascade_frontalface_alt.xml";
  if( !face_cascade.load( face_cascade_name ) ){ 
    printf("--(!)Error loading\n");
  };
  //init upper body detect.
  std::string upperbody_cascade_name = "../data/haarcascades/haarcascade_mcs_upperbody.xml";
  if( !upperbody_cascade.load(upperbody_cascade_name ) ){
    printf("--(!)Error loading\n");
  };
#endif
}

int CBlobDetect::DetectPeople(const cv::Mat roi_frame){
  std::vector<cv::Rect> found;
  people_detect_hog.detectMultiScale(roi_frame,found,0,cv::Size(8,8),cv::Size(32,32),1.05,2);
  for(unsigned int i=0;i<found.size();i++){
    //cv::rectangle(roi_frame,found[i].tl(),found[i].br(),cv::Scalar(0,255,0),3);
  }
  return found.size();
}

} //namespace vidy
