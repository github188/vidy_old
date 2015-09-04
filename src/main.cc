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

#ifdef TESTVIEW  // for test view
void help(){
  printf("please enter video address.\n");
}

//@param argv[1]:read address.
//@param argv[2]:camera type.
int main(int argc,char* argv[]){
  if(argc==2){
    g_type=1;
  }else if(argc==1){
    help();
    return 0;  
  }else{
    g_type=atoi(argv[2]);
  }
  g_dbname2="realtime";
  
  cv::VideoCapture capture(argv[1]);
  if(!capture.isOpened()){
    std::cout<<"remote address error!"<<std::endl;
  }
  
  char key;
  cv::Mat frame;
 
  //--Init processing class.
  vidy::IAutoRun* pAutoRun;

  switch(g_type){
    case 1:
      pAutoRun = new vidy::CAutoRunTestView1();
      break;
    default:
      return 0;
  }

  while(key!=27){
    double begin_time=clock();
 
    capture.read(frame);

    if(frame.empty()){
        usleep(1000000);
        std::cout<<"no frame"<<std::endl;
        continue;
    }

    cv::imshow("frame",frame);

    pAutoRun->Process(frame);

    double finish_time=clock();

    double duration=(finish_time-begin_time)/CLOCKS_PER_SEC;

    int wait_time=(double)(1000.00f/FPS)-duration*1000.00f;

    if(wait_time<10){
      key=cv::waitKey(1);
    }else{
      key=cv::waitKey(wait_time);
    }

  }
  delete pAutoRun;
  return 0;
}

#else // TESTVIEW
void help(){
  printf("please enter database name, camera id, read address and camera type.\n");
  printf("type 1 : entrance type. \n");
  printf("type 2 : fullview/birdview type. \n");
  printf("type 3 : key area type. \n");
  printf("type 4 : real-time entrance type. \n");
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

  g_dbname=argv[1];
  g_dbname2=argv[1];
  g_cid=argv[2];
  g_type=atoi(argv[4]);

  vidy::IDBMySQL* dbmysql = new vidy::IDBMySQL();
  //--get calibration data from database/
  char sql[100];
  if(g_type){
    //--type 1 : entrance type /
    sprintf(sql,"select astext(calibration_data) from t_calibration where cid='%s' and typeid='1'",g_cid);
  }else{
    //--type 2,3 : roi area data /
    sprintf(sql,"select astext(calibration_data) from t_calibration where cid='%s' and typeid='3'",g_cid);
  }
  std::vector<std::vector<std::string> > res = dbmysql->GetData(sql);
  char str[100]="";
  sscanf(res[0][0].data(),"MULTIPOINT(%[^)])",str);
  const char* split=", ";
  char* p;
  p=strtok(str,split);
  int _count = 0;
  cv::Point point;
  while(p!=NULL){
    _count++;
    if(_count%2==1){
      //x
      point.x=atoi(p);
    }else{
      //y
      point.y=atoi(p);
      g_calibrate.push_back(point);
    }
    p=strtok(NULL,split);
  }
#ifdef DEBUG
  for(unsigned int j=0;j<g_calibrate.size();j++){
    std::cout<<"Point:"<<g_calibrate[j].x<<" "<<g_calibrate[j].y<<std::endl;
  }
#endif
  
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
      int wait_time=(double)(1000.00f/FPS)-duration*1000.00f;
      
      if(wait_time<10){
        key=cv::waitKey(1);
      }else{
        key=cv::waitKey(wait_time);
      }
  }
  delete pAutoRun;

  return 1;
} // main

#endif //NORMAL
