//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_generate3.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "blob_generate.h"
#include "global.h"

namespace vidy{

//TODO: improve produtivity.
int CBlobGenerate::GetDirection2(std::vector<cv::Rect> trajectory){
  //init maximum distance.
  float max_distance = 100.00f;
  int direction = 0;

  //find nearest points to each pathways.
  std::vector<std::vector<cv::Point> > near_points;

  //loop each pathways.
  for(unsigned int n=0;n<g_pathways.size();n++){
    cv::Point pp1 = cv::Point((int)(0.5*(g_pathways[n][0].x+g_pathways[n][1].x)),(int)(0.5*(g_pathways[n][0].y+g_pathways[n][1].x)));
    cv::Point pp2 = cv::Point((int)g_pathways[n][1].x,(int)g_pathways[n][1].y);
    float max_distance1 = 100.00f;
    float max_distance2 = 100.00f;
    //loop each point in this trajectory;
    //find the nearest points to the first and last point of this pathway;
    //save points to near_points[n][0] and near_points[n][1];
    for(unsigned int i=0;i<trajectory.size();i++){
      cv::Point p = cv::Point((int)(0.5*(trajectory[i].x+trajectory[i].width)),(int)(0.5*(trajectory[i].y+trajectory[i].height)));
      //distance to pp1.
      float _distance1 = sqrt(pow(p.x-pp1.x,2)+pow(p.y-pp1.y,2));
      //distance to pp2.
      float _distance2 = sqrt(pow(p.x-pp2.x,2)+pow(p.y-pp2.y,2));
      //compare and get nearest points.
      if(_distance1<max_distance1){
        max_distance1 = _distance1;
        near_points[n][0] = p;
      }
      if(_distance2<max_distance2){
        max_distance2 = _distance2;
        near_points[n][1] = p;
      }
    }
    //compare and get the "direction" of minimun distance.
    float _distance = (max_distance1+max_distance2)/2;
    if(_distance<max_distance){
      max_distance = _distance;
      direction = n+1;
    } 
  }
  return direction;
}

void CBlobGenerate::Generate3(BlobNodeList& endBlobNodeList){
  for(unsigned int i=0;i<endBlobNodeList.size();i++){
    int direction = this->GetDirection2(endBlobNodeList[i].trajectory);
#ifdef SERVER
    std::string file="/usr/local/vidy/result/";
#else
    std::string file="../result/";
#endif // SERVER
    file += g_dbname;
    file += "-cid";
    file += g_cid;
    file += "-pathway";
    file += g_time;
    file += ".dat";
    std::ofstream outfile(file.data(),std::ios::app);
 
    //output the result.
    outfile<<direction<<std::endl; 
  }
}


} // namespace vidy
