//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:main.cc
//Author:Yun Luo(lauren.luo@extremevision.mo)

#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

#include "configs.h"
#include "global.h"
#include "save2db_interface.h"
#include "db_mysql.h"

//@argv[1]:database name.
//@argv[2]:camera id.
//@argv[3]:camera type.
int main(int argc,char* argv[]){
  g_dbname=argv[1];
  g_cid=atoi(argv[2]);

#ifdef DEBUG
  //print database info.
  std::cout<<"database address:"<<g_dbaddr<<std::endl;
  std::cout<<"database user:"<<g_dbuser<<std::endl;
  std::cout<<"database pswd:"<<g_dbpswd<<std::endl;
  std::cout<<"database name:"<<g_dbname<<std::endl;
  std::cout<<"camera id:"<<g_cid<<std::endl;
#endif // DEBUG

  //different type of camera use different saving strategy.
  vidy::ISave2DB* save2db;
  switch(atoi(argv[3])){
    case 1:{
      save2db = new vidy::CDBMySQL1();
      break;
    }
    case 2:{
      save2db = new vidy::CDBMySQL2();
      break;
    }
    case 3:{
      save2db = new vidy::CDBMySQL3();
      break;
    }
    default:{
      std::cout<<"please input number of camera type."<<std::endl;
      return 0;
    }
  }

#ifdef DEBUG
  printf("begin save data.\n");
#endif // DEBUG

  time_t t;
  int current_hour=0;
  int previous_hour=0;

  while(1){
#ifdef DEBUG
    printf("begin loop\n");
#endif // DEBUG
    t=time(0);
    char cdate[20];
    char cdate2[20];
    char ctime[20];
    strftime(cdate,sizeof(cdate),"%Y-%m-%d",localtime(&t));
    g_date=cdate;
    strftime(cdate2,sizeof(cdate2),"%Y%m%d",localtime(&t));
    g_date2=cdate2;
    strftime(ctime,sizeof(ctime),"%H",localtime(&t));
    g_time=ctime;
    current_hour=atoi(ctime);
   
    //save data in every hour. 
#ifdef SERVER
    if(current_hour!=previous_hour&&previous_hour!=0&&current_hour>8&&current_hour<22){ 
#else
    if(current_hour!=previous_hour){
#endif // SERVER
      //read data of last hour.
      sprintf(g_time,"%02d",previous_hour);
#ifdef DEBUG
      printf("save2db->Save2DB()\n");
#endif //DEBUG
      //--save data
      save2db->Save2DB();
    }

    previous_hour=current_hour;

    usleep(10000000);
#ifdef DEBUG
    printf("end\n");
#endif // DEBUG
  }
  return 1;
} 
