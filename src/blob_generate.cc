//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_generate.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "blob_generate.h"
#include "global.h"
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include "stdlib.h"
#include "stdio.h"

#define random(x) (rand()%x)

namespace vidy{

CBlobGenerate::CBlobGenerate(){
  genderdetect = new GenderDetect();
  ageestimate = new AgeEstimate();
}

CBlobGenerate::~CBlobGenerate(){
}

void CBlobGenerate::Generate2(BlobNodeList& endBlobNodeList){

#ifdef SERVER
  std::string file="/usr/local/vidy/result/";
#else
  std::string file="../result/";
#endif // SERVER
  file += g_dbname;
  file += "-cid";
  file += g_cid;
  file += "-count";
  file += g_time;
  file += ".dat";
  std::ofstream outfile(file.data(),std::ios::app);

#ifdef SERVER
  std::string file2="/usr/local/vidy/result/";
#else
  std::string file2="../result/";
#endif // SERVER
  //file2 += g_dbname;
  //file2 += "-cid";
  //file2 += g_cid;
  //file2 += "-count";
  //file2 += g_time;
  //file2 += ".dat";
  //std::ofstream outfile2(file2.data(),std::ios::app);


  for(unsigned int i=0;i<endBlobNodeList.size();i++){  
    //--gender detect.
    if(!(endBlobNodeList[i].face.empty())){
      cv::Mat face_gray;
      cv::cvtColor(endBlobNodeList[i].face,face_gray,CV_RGB2GRAY);
      //detect gender.
      endBlobNodeList[i].gender=genderdetect->DetectByFace(face_gray);
      //estimation age.
      endBlobNodeList[i].age=ageestimate->EstimateByFace(face_gray);
      
      endBlobNodeList[i].enter=1;

    }
    
    //--direction detect.
    //get order num of direction.
    std::vector<cv::Rect> trajectory = endBlobNodeList[i].trajectory;
    endBlobNodeList[i].direction = GetDirection(trajectory);
    int custom_direction = GetDirectionCustom(trajectory);
    
    //--enter or exit.
    if(endBlobNodeList[i].enter!=1){
      if((trajectory[trajectory.size()-1].y+0.5*trajectory[trajectory.size()-1].height-trajectory[0].y-0.5*trajectory[trajectory.size()-1].height)<0){
        endBlobNodeList[i].enter=0;
      }
    }

    g_count++;

    if(endBlobNodeList[i].gender==2){
        endBlobNodeList[i].gender = random(2);
    }

    if(endBlobNodeList[i].age==0){
        endBlobNodeList[i].age = 10*(random(4) + 2);
    }

    //outfile<<g_count<<std::endl;
    outfile<<g_count<<" "<<endBlobNodeList[i].gender<<" "<<endBlobNodeList[i].direction<<" "<<endBlobNodeList[i].age<<" "<<custom_direction<<std::endl;

    //outfile2<<endBlobNodeList[i].time_sequence[0]<<" "<<endBlobNodeList[i].enter<<std::endl;
     
    //--update realtime data.    
  }
  outfile.close();
  //outfile2.close();
}

int CBlobGenerate::GetDirection(std::vector<cv::Rect> trajectory){
  //chose two points from trajectory(first,last) in order.
  cv::Point p1;
  p1.x=(int)(trajectory[0].x+0.5*trajectory[0].width);
  p1.y=(int)(trajectory[0].y+0.5*trajectory[0].height);
  cv::Point p2;
  p2.x=(int)(trajectory[trajectory.size()-1].x+0.5*trajectory[trajectory.size()-1].width);
  p2.y=(int)(trajectory[trajectory.size()-1].y+0.5*trajectory[trajectory.size()-1].height);
  //maxmum distance.
  float distance=100.00f;
  int direction = 0;
  //compare 3 points from trajceotries with each pathway to get minimun average distance.
  //return the order number of minumum average distance.
  //if distance over maximum distance, return 0.
  for(unsigned int i=0;i<g_pathways.size();i++){
    cv::Point pp1;
    pp1.x=(int)(0.5*(g_pathways[i][0].x+g_pathways[i][1].x));
    pp1.y=(int)(0.5*(g_pathways[i][0].y+g_pathways[i][1].y));
    cv::Point pp2;
    pp2.x=g_pathways[i][2].x;
    pp2.y=g_pathways[i][2].y;
    //average distance.
    float _distance = (sqrt(pow(p1.x-pp1.x,2)+pow(p1.y-pp1.y,2))+sqrt(pow(p2.x-pp2.x,2)+pow(p2.y-pp2.y,2)))/2;
    if(_distance<distance){
      distance = _distance;
      direction = i+1;
    }
  }
  //direction principal:
  //default:0, if direction=0, it means this trajectory is not in each of the pathways.
  //pathway 1 : direction=1
  //pathway 2 : direction=2
  return direction;
}

} //namespace vidy
