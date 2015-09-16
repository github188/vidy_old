//Copyright(c) 2015 ExtremeVision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevision.mo)
//File:main.cc

#include <iostream>
#include <fstream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "global.h"
#include "autorun.h"
#include "db_mysql.h"

void help(){
  printf("please enter database name, camera id, read address and camera type.\n");
  printf("type 1 : entrance type. \n");
  printf("type 2 : fullview/birdview type. \n");
  printf("type 3 : key area type. \n");
}

void get_calibration_data(){
  //-- data from database ---//
  vidy::IDBMySQL* dbmysql = new vidy::IDBMySQL();

  //note.  only g_type==1 needs g_calibration.
  //note2. only 1 door situation.
  //-- get door calibration data.--/
  char sql_calibration[100];
  if(g_type){
    sprintf(sql_calibration,"select calibration_data from t_calibration where cid='%s' and typeid='1' limit 0,1",g_cid);
    std::vector<std::vector<std::string> > res_calibration = dbmysql->GetData(sql_calibration);
    if(res_calibration.size()>0){
      char str[100]="";
      sscanf(res_calibration[0][0].data(),"{%[^}]}",str);
      const char* split=":,";
      char* p;
      p=strtok(str,split);
      int _count = 0;
      cv::Point point;
      while(p!=NULL){
        _count++;
        switch(_count%4){
          case 1:{
            break;
          }
          case 2:{
            int len = sizeof(p);
            char* x = new char(len-3);
            x = p+1;
            point.x = atoi(x);
            break;
          }
          case 3:{
           break;
          }
          case 0:{
            int len = sizeof(p);
            char* y = new char(len-3);
            y = p+1;
            point.y = atoi(y);
            g_calibrate.push_back(point);
            break;
          }
        }
        p=strtok(NULL,split);
      }
#ifdef DEBUG
      std::cout<<"G_Door:";
      for(unsigned int j=0;j<g_calibrate.size();j++){
        std::cout<<g_calibrate[j].x<<" "<<g_calibrate[j].y<<" | ";
      }
      std::cout<<std::endl;
#endif
    }
  }

  //-- get default pathway data --/
  char sql_pathway[100];
  sprintf(sql_pathway,"select calibration_data from t_calibration where cid='%s' and typeid='2' and path_type='default'",g_cid);
  std::vector<std::vector<std::string> > res_pathway = dbmysql->GetData(sql_pathway);
  for(unsigned int i=0;i<res_pathway.size();i++){
    char str[100]="";
    sscanf(res_pathway[i][0].data(),"{%[^}]}",str);
    const char* split=":,";
    char* p;
    p=strtok(str,split);
    int _count = 0;
    cv::Point point;
    Pathway _pathway;
    while(p!=NULL){
      _count++;
      switch(_count%4){
        case 1:{
          break;
        }
        case 2:{
          int len = sizeof(p);
          char* x = new char(len-3);
          x = p+1;
          point.x = atoi(x);
          break;
        }
        case 3:{
          break;
        }
        case 0:{
          int len = sizeof(p);
          char* y = new char(len-3);
          y = p+1;
          point.y = atoi(y);
          _pathway.push_back(point);
          break;
        }
      }      
      p=strtok(NULL,split);
    }
    g_pathways.push_back(_pathway);
#ifdef DEBUG
    std::cout<<"G_Pathways:";
    for(unsigned int j=0;j<g_pathways[i].size();j++){
      std::cout<<g_pathways[i][j].x<<" "<<g_pathways[i][j].y<<" | ";
    }
    std::cout<<std::endl;
#endif

  }

  //-- get custom pathway data --/
  //char sql_pathway[100];
  sprintf(sql_pathway,"select calibration_data from t_calibration where cid='%s' and typeid='2' and path_type='custom'",g_cid);
  res_pathway = dbmysql->GetData(sql_pathway);
  for(unsigned int i=0;i<res_pathway.size();i++){
    char str[100]="";
    sscanf(res_pathway[i][0].data(),"{%[^}]}",str);
    const char* split=":,";
    char* p;
    p=strtok(str,split);
    int _count = 0;
    cv::Point point;
    Pathway _pathway;
    while(p!=NULL){
      _count++;
      switch(_count%4){
        case 1:{
          break;
        }
        case 2:{
          int len = sizeof(p);
          char* x = new char(len-3);
          x = p+1;
          point.x = atoi(x);
          break;
        }
        case 3:{
          break;
        }
        case 0:{
          int len = sizeof(p);
          char* y = new char(len-3);
          y = p+1;
          point.y = atoi(y);
          _pathway.push_back(point);
          break;
        }
      }
      p=strtok(NULL,split);
    }
    g_pathways_custom.push_back(_pathway);
#ifdef DEBUG
    std::cout<<"G_Pathways:";
    for(unsigned int j=0;j<g_pathways_custom[i].size();j++){
      std::cout<<g_pathways_custom[i][j].x<<" "<<g_pathways_custom[i][j].y<<" | ";
    }
    std::cout<<std::endl;
#endif

  }


  //note. only 1 area situation.
  //-- get area data --/
  char sql_area[100];
  sprintf(sql_area,"select calibration_data from t_calibration where cid='%s' and typeid='3' limit 0,1",g_cid);
  std::vector<std::vector<std::string> > res_area = dbmysql->GetData(sql_area);
  if(res_area.size()>0){
    char str[100]="";
    sscanf(res_area[0][0].data(),"{%[^}]}",str);
    const char* split=":,";
    char* p;
    p=strtok(str,split);
    int _count = 0;
    cv::Point point;
    while(p!=NULL){
      _count++;
      switch(_count%4){
        case 1:{
          break;
        }
        case 2:{
          int len = sizeof(p);
          char* x = new char(len-3);
          x = p+1;
          point.x = atoi(x);
          break;
        }
        case 3:{
          break;
        }
        case 0:{
          int len = sizeof(p);
          char* y = new char(len-3);
          y = p+1;
          point.y = atoi(y);
          g_area.push_back(point);
          break;
        }
      }
      p=strtok(NULL,split);
    }
#ifdef DEBUG
    std::cout<<"G_Areas:";
    for(unsigned int j=0;j<g_area.size();j++){
      std::cout<<g_area[j].x<<" "<<g_area[j].y<<" | ";;
    }
    std::cout<<std::endl;
#endif
  }


  delete dbmysql;

}

//@param argv[1]:database name.
//@param argv[2]:camera id.
//@param argv[3]:read address.
//@param argv[4]:camera type.
int main(int argc,char* argv[]){

  if(argc<5){
    help();
    return 0;
  }

  //set global variable.
  g_dbname=argv[1];
  g_cid=argv[2];
  g_type=atoi(argv[4]);

  //--set database configs--
  g_dbname2=argv[1];

  //-- data from database ---//
  get_calibration_data();
  
  //--Init processing class.
  vidy::IAutoRun* pAutoRun;
  
  switch(g_type){

    //type 1 : for entrance.
    case 1:
      if(g_calibrate.size()!=2){
        printf("type 1 calibrated data must be two points.\n");
        return 0;
      }
      pAutoRun = new vidy::CAutoRun();
      break;

    //type 2 : for birdview or fullview.
    case 2:
      pAutoRun = new vidy::CAutoRun2();
      break;

    //type 3 : for key area.
    case 3:
      pAutoRun = new vidy::CAutoRun3();
      break;

    default:
      printf("camera type haven't exist\n");
      return 0;
  }
  cv::VideoCapture capture(argv[3]);

  if(!capture.isOpened()){
    std::cout<<"remote address error!"<<std::endl;
  }

  char key;
  cv::Mat frame; 
  time_t t; //localtime.

#ifdef DEBUG
  int count_frame=0;
#endif // DEBUG
  
  while(key!=27){
      double begin_time=clock();

      capture.read(frame);

      if(frame.empty()){
        usleep(1000000);
        std::cout<<"no frame"<<std::endl;
        continue;
      }

      //--get current time--
      t=time(0);
      char ctime[20];
      strftime(ctime,sizeof(ctime),"%Y%m%d%H",localtime(&t));
      g_time=ctime;
      
      int hour;
      char chour[20];
      strftime(chour,sizeof(chour),"%H",localtime(&t));
      hour = atoi(chour);
 
#ifdef DEBUG
      //cv::line(frame,cv::Point(g_door_x1,g_door_y1),cv::Point(g_door_x2,g_door_y2),cv::Scalar(255,0,0),3);
      cv::imshow("frame",frame);
#endif //DEBUG

      if(hour>=8 && hour<23){
        //--process runs.
        pAutoRun->Process(frame);
      }

      double finish_time=clock();

      double duration=(finish_time-begin_time)/CLOCKS_PER_SEC;
#ifdef DEBUG
      std::cout<<duration<<std::endl;
#endif
      //int wait_time=(double)(1000.00f/FPS)-duration*1000.00f;
      
      //-- time control --/
      key=cv::waitKey(1);

#ifdef DEBUG
      if(key=='s'){
        char pic[50];
        sprintf(pic,"%s/test%d.jpg",g_cid,count_frame);
        cv::imwrite(pic,frame);
        count_frame++;
      }
      if(key=='b'){
        cv::imwrite("background.jpg",frame);
      }
#endif
  }
  delete pAutoRun;

  return 1;
} // main

