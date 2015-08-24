//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File: blob.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_BLOB_H_
#define VIDY_INC_BLOB_H_

#include <string>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

#include <opencv2/core/core.hpp>

#include "gender.h"
#include "direction.h"
#include "CompressiveTracker.h"

namespace vidy{

struct Blob{
  std::string id;
  int x,y;
  int w,h;
  cv::Mat image;
};
class BlobNode;
typedef BlobNode* PtrBlobNode;

class BlobNode{

public:
  BlobNode(){
    prev=NULL;
    next=NULL;
  };

  explicit BlobNode(const Blob mblob){
    blob=mblob;
    std::string id=GetId(mblob.x,mblob.y,mblob.w,mblob.h);
    blob.id=id;
    prev=NULL;
    next=NULL;
    //init box.
    box.x=mblob.x;
    box.y=mblob.y;
    box.width=mblob.w;
    box.height=mblob.h;

    //init gender.
    gender=2;

    //init direction.
    direction=0;

    //init age.
    age=0;
  };

  ~BlobNode(){
  };

  Blob blob;

  PtrBlobNode prev;

  PtrBlobNode next;

  inline void AddAfterByBlobNode(BlobNode& blobnode){
    (&blobnode)->next=this;
    this->prev=&blobnode;
  };

  //Compressive Tracker.
  CompressiveTracker* ct;
  cv::Rect box;

  //Face Image.
  cv::Mat face;

  //Gender.
  int gender; //1:male 0:female 2:N/A

  //Age.
  int age;

  //Direction.
  int direction;

  //Trajectory.
  std::vector<cv::Rect> trajectory;

protected:
  //Id consists of year,month,day,hour,minute,second,milisecond,blob's x,blob's y,blob's width and blob's height.
  std::string GetId(const int x,const int y,const int w,const int h){
    char buf[28];
    struct timeval tv;
    struct tm tm;
    size_t len=28;
    gettimeofday(&tv,NULL);
    localtime_r(&tv.tv_sec,&tm);
    strftime(buf,len,"%Y%m%d%H%M%S",&tm);
    len=strlen(buf);
    sprintf(buf+len,"%06d",(int)(tv.tv_usec));
    std::string id=buf;
    char buf2[16];
    sprintf(buf2,"%03d%03d%03d%03d",x,y,w,h);
    id += buf2;
    return id;
  };

}; //class BlobNode

typedef std::vector<BlobNode> BlobNodeList;

} //namepsace vidy

#endif //VIDY_INC_BLOB_H_
