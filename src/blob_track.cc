//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_track.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "blob_track.h"
#ifdef DEBUG
#include <iostream>
#endif //DEBUG

#define MAXCOUNT 100

#define MINCOUNT 3

namespace vidy{

CBlobTrack::CBlobTrack(){
  existBlobNodeList2=NULL;
  blobcompare=new BlobCompare();
  //init face detect
#ifdef SERVER
  std::string face_cascade_name = "/root/vidy/data/haarcascades/haarcascade_frontalface_alt.xml";
#else
  std::string face_cascade_name = "../data/haarcascades/haarcascade_frontalface_alt.xml";
#endif // SERVER
  if(!face_cascade.load(face_cascade_name)){
    printf("--(!)Error loading(face detect)\n");
  };
}

CBlobTrack::~CBlobTrack(){
  delete blobcompare;
}

void CBlobTrack::Track(BlobNodeList* existBlobNodeList,BlobNodeList& currentBlobNodeList){
  endBlobNodeList.clear();

  //update track.
  for(unsigned int j=0;j<currentBlobNodeList.size();j++){
    for(unsigned int i=0;i<existBlobNodeList->size();i++){
      //track.
      if((*existBlobNodeList)[i].next==NULL){
        if(this->CompareBlobNode(currentBlobNodeList[j],(*existBlobNodeList)[i])){
          currentBlobNodeList[j].AddAfterByBlobNode((*existBlobNodeList)[i]);
          break;
        }
      }
    }
  }
   
  //get endBlobNodeList.
  BlobNodeList::iterator it3=(*existBlobNodeList).begin();
  while(it3!=(*existBlobNodeList).end()){
    if(it3->next==NULL){
      endBlobNodeList.push_back(*it3);
    }
    ++it3;
  }

  //Deep copy from currentBlobNodeList to existBlobNodeList.
  for(unsigned int i=0;i<currentBlobNodeList.size();i++){
    BlobNode blobn;
    blobn.blob=currentBlobNodeList[i].blob;
    blobn.prev=currentBlobNodeList[i].prev;
    blobn.next=currentBlobNodeList[i].next;
    (*existBlobNodeList).push_back(blobn);
  }
}

void CBlobTrack::Track2(BlobNodeList* existBlobNodeList,BlobNodeList& currentBlobNodeList){

  endBlobNodeList.clear();

  //--compressive tracking for each exist blobnode.
  for(unsigned int i=0;i<existBlobNodeList->size();i++){
    (*existBlobNodeList)[i].ct->processFrame(gray_frame,(*existBlobNodeList)[i].box);
    //draw rectangle.
#ifdef DEBUG
    cv::rectangle(_frame,(*existBlobNodeList)[i].box,cv::Scalar(0,255,0),3);
#endif //DEBUG

    //--find end blobnodes , delete from existBlobNodeList and add to endBlobNodeList.
    if((*existBlobNodeList)[i].box.width==0){
      endBlobNodeList.push_back((*existBlobNodeList)[i]);
    }

    //face detect.
    std::vector<cv::Rect> faces;
    cv::Mat face_gray(_frame,(*existBlobNodeList)[i].box);
    face_cascade.detectMultiScale(face_gray,faces,1.1,2,0|CV_HAAR_SCALE_IMAGE,cv::Size(30,30));
    if(faces.size()>0){
      //Get face image.
      cv::Rect face_rect((*existBlobNodeList)[i].box.x+faces[0].x,(*existBlobNodeList)[i].box.y+faces[0].y,faces[0].width,faces[0].height);
      cv::Mat face_tmp(_frame,face_rect);
      if(((*existBlobNodeList)[i].face).empty()){
        (*existBlobNodeList)[i].face=face_tmp.clone();
      }
#ifdef DEBUG
      cv::rectangle(_frame,face_rect,cv::Scalar(0,0,255),3);
#endif //DEBUG
    }
    //update trajectory.
    (*existBlobNodeList)[i].trajectory.push_back((*existBlobNodeList)[i].box);
  }  

  //--find new blobnodes and add to existBlobNodeList.
  for(unsigned int j=0;j<currentBlobNodeList.size();j++){
    bool b_new=true;
    for(unsigned int i=0;i<existBlobNodeList->size();i++){
      if(this->CompareBlobNode(currentBlobNodeList[j],(*existBlobNodeList)[i])){
        b_new=false;
        break;
      }
    }
    if(b_new){
      currentBlobNodeList[j].ct = new CompressiveTracker();
      currentBlobNodeList[j].ct->init(gray_frame,currentBlobNodeList[j].box);
      (*existBlobNodeList).push_back(currentBlobNodeList[j]);
    }
  }

  //--remove end blobnodes from exist blobnodes.
  std::vector<BlobNode>::iterator it=existBlobNodeList->begin();
  while(it!=existBlobNodeList->end()){
    if(((it->box.x+it->box.width)>_frame.cols-10)||
       it->box.x<10||
       ((it->box.y+it->box.height)>_frame.rows-10)||
       it->box.y<10||
       (it->trajectory).size()>MAXCOUNT){
      if((it->trajectory).size()>MINCOUNT){
        endBlobNodeList.push_back(*it);
      }
      it=existBlobNodeList->erase(it);
      //++it;
    }else{
      ++it;
    }
  }
#ifdef DEBUG
  cv::imshow("tracking",_frame);
#endif //DEBUG
} 

void CBlobTrack::TrackFace(BlobNodeList* existBlobNodeList,BlobNodeList& currentBlobNodeList){
  endBlobNodeList.clear();

  //--compressive tracking for each exist blobnode.
  for(unsigned int i=0;i<existBlobNodeList->size();i++){
    (*existBlobNodeList)[i].ct->processFrame(gray_frame,(*existBlobNodeList)[i].box);
    //draw rectangle.
#ifdef DEBUG
    cv::rectangle(_frame,(*existBlobNodeList)[i].box,cv::Scalar(0,255,0),3);
#endif //DEBUG

    //--find end blobnodes , delete from existBlobNodeList and add to endBlobNodeList.
    if((*existBlobNodeList)[i].box.width==0){
      endBlobNodeList.push_back((*existBlobNodeList)[i]);
    }

    //face.
    cv::Mat face_gray(_frame,(*existBlobNodeList)[i].box);
    if(((*existBlobNodeList)[i].face).empty()){
      (*existBlobNodeList)[i].face=face_gray.clone();
    }

    //update trajectory.
    (*existBlobNodeList)[i].trajectory.push_back((*existBlobNodeList)[i].box);
  }


  //--find new blobnodes and add to existBlobNodeList.
  for(unsigned int j=0;j<currentBlobNodeList.size();j++){
    bool b_new=true;
    for(unsigned int i=0;i<existBlobNodeList->size();i++){
      if(this->CompareBlobNode(currentBlobNodeList[j],(*existBlobNodeList)[i])){
        b_new=false;
        break;
      }
    }
    if(b_new){
      currentBlobNodeList[j].ct = new CompressiveTracker();
      currentBlobNodeList[j].ct->init(gray_frame,currentBlobNodeList[j].box);
      (*existBlobNodeList).push_back(currentBlobNodeList[j]);
    }
  }

  //--remove end blobnodes from exist blobnodes.
  std::vector<BlobNode>::iterator it=existBlobNodeList->begin();
  while(it!=existBlobNodeList->end()){
    if(((it->box.x+it->box.width)>_frame.cols-10)||
       it->box.x<10||
       ((it->box.y+it->box.height)>_frame.rows-10)||
       it->box.y<10||
       (it->trajectory).size()>MAXCOUNT){
      if((it->trajectory).size()>MINCOUNT){
        endBlobNodeList.push_back(*it);
      }
      it=existBlobNodeList->erase(it);
      //++it;
    }else{
      ++it;
    }
  }
#ifdef DEBUG
  cv::imshow("tracking",_frame);
#endif //DEBUG

}

//Compare two blobs for tracking.
//Step 1 : Compare by distance.
//Step 2 : Compare by haar feature.
int CBlobTrack::CompareBlobNode(BlobNode blobnode,BlobNode compareblobnode){
  //Step 1--//
  if(blobcompare->CompareDistance(blobnode,compareblobnode,30.00f)){
    //TODO:Step 2--// 
    return 1;
  }else{
    return 0;
  }
}

} //namespace vidy
