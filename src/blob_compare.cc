//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_compare.cc
//Author: Yun Luo(lauren.luo@extremevision.mo)

#include <iostream>
#include "blob_compare.h"
#include "math.h"

namespace vidy{

BlobCompare::BlobCompare(){
}

BlobCompare::~BlobCompare(){
}

int BlobCompare::CompareDistance(const BlobNode current_blobnode,const BlobNode compare_blobnode,float distance){
  float dis_x=fabs(current_blobnode.blob.x+0.5*current_blobnode.blob.w-compare_blobnode.blob.x-0.5*compare_blobnode.blob.w);
  float dis_y=fabs(current_blobnode.blob.y+0.5*current_blobnode.blob.h-compare_blobnode.blob.y-0.5*compare_blobnode.blob.h);
  float dis=sqrt(dis_x*dis_x+dis_y*dis_y);
#ifdef DEBUG
  std::cout<<"dis:"<<dis<<std::endl;
#endif // DEBUG<
  if(dis<distance){
    return 1;
  }else{
    return 0;
  }
}

int BlobCompare::CompareFeature(const BlobNode current_blobnode,const BlobNode compare_blobnode) const{
  return 1;
}

} //namespace vidy
