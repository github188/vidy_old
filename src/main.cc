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
  g_cid=argv[2];
  g_type=atoi(argv[4]);
    
  //--get calibrated data.
#ifdef SERVER
  std::string filename="/root/vidy/calibration/";
#else
  std::string filename="../calibration/"; 
#endif // SERVER
  filename += g_dbname;
  filename += "-cid";
  filename += g_cid;
  filename += ".dat";
  std::ifstream inFile(filename.data(),std::ios::in);

  //default calibrate.
  if(!inFile.is_open()){
    cv::Point point1(0,0);
    cv::Point point2(1280,0);
    cv::Point point3(1280,720);
    cv::Point point4(0,720);
    g_calibrate.push_back(point1);
    g_calibrate.push_back(point2);
    g_calibrate.push_back(point3);
    g_calibrate.push_back(point4);
  }else{
    while(!inFile.eof()){
      cv::Point point;
      inFile>>point.x>>point.y;
      g_calibrate.push_back(point);
    }
  }
  inFile.close();

  //delete last data as it's empty.
  if(g_calibrate.size()>1){
    g_calibrate.erase(g_calibrate.end());
  }

#ifdef DEBUG
  std::cout<<g_calibrate.size()<<std::endl;
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

      if(hour>=8 && hour<21){
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

