//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_generate3.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include "blob_generate.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define random(x) (rand()%x)

#define HEATMAP_RADIUS 50
#define HEATMAP_MIN_SEC 5

namespace vidy{

//TODO: improve productivity.
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
    direction = random(g_pathways.size())+1;
  }
  return direction;
}

//TODO: improve productivity.
int CBlobGenerate::GetDirectionCustom(std::vector<cv::Rect> trajectory){
  //init maximum distance.
  float max_distance = 100.00f;
  int direction = 0;

  //find nearest points to each pathways.
  std::vector<std::vector<cv::Point> > near_points;

  //loop each pathways.
  for(unsigned int n=0;n<g_pathways_custom.size();n++){
    cv::Point pp1 = cv::Point((int)(0.5*(g_pathways_custom[n][0].x+g_pathways_custom[n][1].x)),(int)(0.5*(g_pathways_custom[n][0].y+g_pathways_custom[n][1].x)));
    cv::Point pp2 = cv::Point((int)g_pathways_custom[n][1].x,(int)g_pathways_custom[n][1].y);
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

int CBlobGenerate::GetStayTime(std::vector<std::string> time_sequence){
#ifdef DEBUG
  std::cout<<time_sequence.size()<<std::endl;
#endif // DEBUG
if(time_sequence.size()>5){
  std::string time_start = time_sequence[0];
  std::string time_end = time_sequence[time_sequence.size()-1];
#ifdef DEBUG
  std::cout<<"time_start:"<<time_start<<std::endl;
  std::cout<<"time_end:"<<time_end<<std::endl;
#endif // DEBUG
  int hour_start,minue_start,second_start,hour_end,minue_end,second_end;
  const char* split=": ";
  char* p;
  char str[100];
  sprintf(str,"%s",time_start.data());
  p=strtok(str,split);
  int _count=0;
  while(p!=NULL){
    _count++;
    switch(_count%4){
      case 1:{
        break;
      }
      case 2:{
        hour_start = atoi(p);
        break;
      }
      case 3:{
        minue_start = atoi(p);
        break;
      }
      case 0:{
        second_start = atoi(p);
        break;
      }
    }
    p = strtok(NULL,split);
  }
  //str[100]="";
  sprintf(str,"%s",time_end.data());
  p=strtok(str,split);                                                  
  _count=0;
  while(p!=NULL){                                                                     
     _count++;
     switch(_count%4){                                                                 
       case 1:{                                                                        
         break;                                                                        
       }
       case 2:{                                                  
         hour_end = atoi(p);                                                                                                                       
         break;                                                                        
       }
       case 3:{    
         minue_end = atoi(p);                                                                                                                   
         break;                                                                        
       }
       case 0:{
         second_end = atoi(p);                                                                                                                 
         break;                                                                        
       }                                                                               
     } 
     p = strtok(NULL,split);                                                           
   }         
 
#ifdef DEBUG
  std::cout<<"hour:"<<hour_start<<" "<<hour_end<<std::endl;
  std::cout<<"minue:"<<minue_start<<" "<<minue_end<<std::endl;
  std::cout<<"second:"<<second_start<<" "<<second_end<<std::endl;
#endif // DEBUG
  //cal staytime.
  if(second_end<second_start){
    second_end += 60;
    minue_end -= 1;
  }
  if(minue_end<minue_start){
    minue_end += 60;
    hour_end -= 1;
  }
  int staytime = 3600*(hour_end-hour_start)+60*(minue_end-minue_start)+second_end-second_start;
#ifdef DEBUG
  std::cout<<"stay time:"<<staytime<<std::endl;
#endif // DEBUG
  return staytime;
}else{
  return 0;
}
}

std::vector<Heatmap> CBlobGenerate::GetHeatmapResult(std::vector<std::string> time_sequence,std::vector<cv::Rect> trajectory)
{
  Heatmap heatmap_one;
  heatmap_one.radius = HEATMAP_RADIUS;
  std::vector<Heatmap> heatmap_data;
  for(unsigned int i=0;i<trajectory.size();i++){
    float t_x = trajectory[i].x+0.5*trajectory[i].width;
    float t_y = trajectory[i].y+0.5*trajectory[i].height;
    heatmap_one.value = 0;
    for(unsigned int j=i+1;j<trajectory.size();j++){
      float t_x2 = trajectory[j].x+0.5*trajectory[j].width;
      float t_y2 = trajectory[j].y+0.5*trajectory[j].height;
      if(fabs(t_x-t_x2)<HEATMAP_RADIUS&&fabs(t_y-t_y2)<HEATMAP_RADIUS){
        heatmap_one.value++;
      }
    }
    if(heatmap_one.value>FPS*HEATMAP_MIN_SEC){
      heatmap_one.x = (int)(t_x+0.5*HEATMAP_RADIUS);
      heatmap_one.y = (int)(t_y+0.5*HEATMAP_RADIUS);
      heatmap_data.push_back(heatmap_one);
    }
  }
  return heatmap_data;
}

void CBlobGenerate::Generate3(BlobNodeList& endBlobNodeList){
  //open pathway file.
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

  //open staytime file.
#ifdef SERVER
  std::string file2="/usr/local/vidy/result/";
#else
  std::string file2="../result/";
#endif // SERVER
  file2 += g_dbname;
  file2 += "-cid";
  file2 += g_cid;
  file2 += "-staytime";
  file2 += g_time;
  file2 += ".dat";
  std::ofstream outfile2(file2.data(),std::ios::app);

  //open heatmap file.
#ifdef SERVER
  std::string file3="/usr/local/vidy/result/";
#else
  std::string file3="../result/";
#endif // SERVER
  file3 += g_dbname;
  file3 += "-cid";
  file3 += g_cid;
  file3 += "-heatmap";
  file3 += g_time;
  file3 += ".dat";
  std::ofstream outfile3(file3.data(),std::ios::app);

  //open custom-defined pathway file.
  //open pathway file.
#ifdef SERVER
  std::string file4="/usr/local/vidy/result/";
#else
  std::string file4="../result/";
#endif // SERVER
  file4 += g_dbname;
  file4 += "-cid";
  file4 += g_cid;
  file4 += "-custom_pathway";
  file4 += g_time;
  file4 += ".dat";
  std::ofstream outfile4(file.data(),std::ios::app);

  g_count++;
#ifdef DEBUG
  std::cout<<"count:"<<g_count<<std::endl;
#endif

  for(unsigned int i=0;i<endBlobNodeList.size();i++){ 
   //if(i==0){
    //get direction.
    int direction = this->GetDirection2(endBlobNodeList[i].trajectory);
    endBlobNodeList[i].direction = direction;
    //output the result.
    outfile<<direction<<std::endl;
    //get custom-defined direction.
    int direction_custom = this->GetDirectionCustom(endBlobNodeList[i].trajectory);
    //output the result.
    outfile4<<direction_custom<<std::endl;
    //get staytime.
    int staytime = this->GetStayTime(endBlobNodeList[i].time_sequence); 
    //output the result.
#ifdef DEBUG
    std::cout<<"staytime:"<<staytime<<std::endl;
#endif
    if(staytime>5){
      outfile2<<g_count<<" "<<staytime<<std::endl;
    }
   //}
    //get heatmap_one
    std::vector<Heatmap> heatmap_data = this->GetHeatmapResult(endBlobNodeList[i].time_sequence,endBlobNodeList[i].trajectory);
    //output the result.
    for(unsigned int j=0;j<heatmap_data.size();j++){
      outfile3<<heatmap_data[j].x<<" "<<heatmap_data[j].y<<" "<<heatmap_data[j].value<<" "<<heatmap_data[j].radius<<std::endl;
    }
  }
  
  //close files.
  outfile.close();
  outfile2.close();
  outfile3.close();
  outfile4.close();
}


} // namespace vidy
