//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_track.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_BLOB_TRACK_H_
#define VIDY_INC_BLOB_TRACK_H_

#include "blob.h"
#include "blob_compare.h"
#include "gender_detect.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#ifdef DEBUG
#include <opencv2/highgui/highgui.hpp>
#endif //DEBUG

namespace vidy{

class CBlobTrack{

public:
  CBlobTrack();
  ~CBlobTrack();
  void Track(BlobNodeList* existBlobNodeList,BlobNodeList& currentBlobNodeList);

  //Note. Use compressive tracking.
  void Track2(BlobNodeList* existBlobNodeList,BlobNodeList& currentBlobNodeList);

  BlobNodeList GetEndBlobNodeList(){
    return endBlobNodeList;
  };

  void GetFrame(cv::Mat frame){
    _frame=frame.clone();
    cv::cvtColor(frame,gray_frame,CV_RGB2GRAY);
  };
protected:
  int CompareBlobNode(BlobNode blobnode,BlobNode compareblobnode);
private:
  BlobNodeList endBlobNodeList;
  BlobNodeList* existBlobNodeList2;
  cv::Mat _frame;
  cv::Mat gray_frame;

  //blob compare.
  BlobCompare* blobcompare;

  //face detect.
  cv::CascadeClassifier face_cascade;

}; //class CBlobTrack

} //namespace vidy

#endif //VIDY_INC_BLOB_TRACK_H_
