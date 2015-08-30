//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_detect.h
//Author: Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_BLOB_DETECT_H_
#define VIDY_INC_BLOB_DETECT_H_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <objdetect/objdetect.hpp>

#include "blob.h"
#include "preblob.h"

namespace vidy{

class CBlobDetect{
public:
  CBlobDetect();
  ~CBlobDetect();
  
  //TODO
  //Note: After getting the foreground as a mask
  //inorder to smaller the detect area.
  //Mask can be got from fg_detect.h for example.
  void DetectHeadsFromUpperside(const cv::Mat frame,const cv::Mat mask);

  //Note: Use the original frame for detecting faces.
  //@return: blobnodelist of this frame.
  BlobNodeList DetectFace(const cv::Mat frame);

  //Note: Use the original frame for detecting upper body.
  //@return: blobnodelist of this frame.
  BlobNodeList DetectUpperBody(const cv::Mat frame);

  //Note: Based on function DetectUpperBody, improving detection
  //accurecy. After 3 times of detection, a new blobnode will be set.
  //!Important: It's better not to use both DetectUpperBody and DetectUpperBody2 
  //at the same time.
  //@return : new blobnode list.
  BlobNodeList DetectUpperBody2(const cv::Mat frame);

  BlobNodeList DetectFace2(const cv::Mat frame);
  
protected:
  void Init();
  
  //TODO
  int DetectPeople(const cv::Mat roi_frame);

  //@return: the nearest and most similar preblob.
  PreBlob FindPreBlobByCurrentPreBlob(PreBlob current_preblob);
  
  //update current_preblob.pre_index.
  int UpdatePreBlob(PreBlob* current_preblob){
    PreBlob preblob=this->FindPreBlobByCurrentPreBlob(*current_preblob);
    if(preblob.pre_index!=0&&preblob.pre_index!=PREBLOBNUM){
      (*current_preblob).pre_index=(preblob.pre_index)++;
      return 1;
    }else if(preblob.pre_index==PREBLOBNUM){
      (*current_preblob).pre_index=PREBLOBNUM;
      return 1;
    }else{
      return 0;
    }
  };

private:
  //TODO
  std::vector<cv::Rect> people;
  cv::HOGDescriptor people_detect_hog;

  //Face Detector.
  cv::CascadeClassifier face_cascade;

  //Upper body Detector.
  cv::CascadeClassifier upperbody_cascade;

  //previous preblob list.
  std::vector<PreBlob> pre_preblob_list;

  //current preblob list.
  std::vector<PreBlob> preblob_list;

  //background.
  cv::Mat background;
  cv::Mat background_gray;

}; //class CBlobDetect.

}

#endif
