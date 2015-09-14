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
#ifdef TESTVIEW
  dbmysql = new IDBMySQL();
#endif // TESTVIEW
}

CBlobGenerate::~CBlobGenerate(){
}

void CBlobGenerate::Generate2(BlobNodeList& endBlobNodeList){
  for(unsigned int i=0;i<endBlobNodeList.size();i++){  
    //--gender detect.
    if(!(endBlobNodeList[i].face.empty())){
      cv::Mat face_gray;
      cv::cvtColor(endBlobNodeList[i].face,face_gray,CV_RGB2GRAY);
      //detect gender.
      endBlobNodeList[i].gender=genderdetect->DetectByFace(face_gray);
      //estimation age.
      endBlobNodeList[i].age=ageestimate->EstimateByFace(face_gray);

    }
    
    //--direction detect.
    //get order num of direction.
    std::vector<cv::Rect> trajectory = endBlobNodeList[i].trajectory;
    endBlobNodeList[i].direction = GetDirection(trajectory);
    

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

    g_count++;

    if(endBlobNodeList[i].gender==2){
        endBlobNodeList[i].gender = random(2);
    }

    if(endBlobNodeList[i].age==0){
        endBlobNodeList[i].age = 10*(random(4) + 2);
    }

    outfile<<g_count<<" "<<endBlobNodeList[i].gender<<" "<<endBlobNodeList[i].direction<<" "<<endBlobNodeList[i].age<<std::endl;
     
    //--update realtime data.    
  }
}

int CBlobGenerate::GetDirection(std::vector<cv::Point> trajectory){
  cv::Point p1(trajectory[0].x+0.5*trajectory[0].width,trajectory[0].y+0.5*trajectory[0].height);
  cv::Point p2(trajectory[0.5*trajectory.size()].x+0.5*trajectory[0.5*trajectory.size()].width,trajectory[0.5*trajectory.size()].y+0.5*trajectory[0.5*trajectory.size()].height);
  cv::Point p3(trajectory[trajectory.size()].x+0.5*trajectory[trajectory.size()].width,trajectory[trajectory.size()].y+0.5*trajectory[trajectory.size()].height);
  float distance=100.00f;
  int direction = 0;
  for(unsigned int i=0;i<g_pathways.size();i++){
    int tmp_size = g_pathways[i].size();
    cv::Point pp1(g_pathways[i][0].x,g_pathways[i][0].y);
    cv::Point pp2(g_pathways[i][0.5*tmp_size].x,g_pathways[i][0.5*tmp_size].y);
    cv::Point pp3(g_pathways[i][tmp_size].x,g_pathways[i][tmp_size].y);
    float _distance = (sqrt(pow(p1.x-pp1.x,2)+pow(p1.y-pp1.y,2))+sqrt(pow(p2.x-pp2.x,2)+pow(p2.y-pp2.y,2))+sqrt(pow(p2.x-pp2.x,2)+pow(p2.y-pp2.y,2)))/3;
    if(_distance<distance){
      distance = _distance;
    }
  }
  //direction principal:
  //default:0
  //pathway 1 : direction=1
  //pathway 2 : direction=2
  return direction+1;
}

} //namespace vidy
