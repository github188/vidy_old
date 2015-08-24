//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:heatmap2db.cc
//Author: Yun Luo(lauren.luo@extremevision.mo)

#include "heatmap2db.h"

namespace vidy{

Heatmap2DB::Heatmap2DB(const int cid){

}

Heatmap2DB::~Heatmap2DB(){

}

//TODO:read file with time infomation.
//TODO:save the data to database as a multipoint format and value 
//as a set.
//Table in database:
//hmid cid data time point(MULTIPOINT) values(SET)
void Heatmap2DB::saveheatmap(){
  std::ifstream inFile("~/ExtremeVision-data/cid1/heatmap.dat");
  while(!inFile.eof()){
    int x,y,value;
    inFile>>x>>y>>value;
  }
  inFile.close();
}

} //namespace vidy
