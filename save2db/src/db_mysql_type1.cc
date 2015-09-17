//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevisin.mo)
//File:db_mysql_type1.cc

#include "db_mysql.h"
#include <stdio.h>
#include <iostream>

namespace vidy{

CDBMySQL1::CDBMySQL1(){
  //--get pathway data from database/
  char sql[100];
  sprintf(sql,"select calibration_data from t_calibration where cid='%d' && typeid='2' && path_type='default'",g_cid);
  std::vector<std::vector<std::string> > res = this->GetData(sql);
  for(unsigned int i=0; i<res.size();i++){
    std::vector<cv::Point> pathway;
    char path_str[100]="";
    sscanf(res[i][0].data(),"{%[^}]}",path_str);
#ifdef DEBUG
    std::cout<<path_str<<std::endl;
#endif // DEBUG
    const char* split=":,";
    char* p;
    p=strtok(path_str,split);
    int _count=0;
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
          pathway.push_back(point);
          break;
        }
      }
      p=strtok(NULL,split);
    }
    pathways.push_back(pathway);
  }
#ifdef DEBUG
  for(unsigned int k=0;k<pathways.size();k++){
    for(unsigned int n=0;n<pathways[k].size();n++){
      std::cout<<pathways[k][n].x<<" "<<pathways[k][n].y<<" ";
    }
    std::cout<<std::endl;
  }
#endif // DEBUG

  //--get custom pathway data from database/
  //char sql[100];
  sprintf(sql,"select calibration_data from t_calibration where cid='%d' and typeid='2' and path_type='custom'",g_cid);
  res = this->GetData(sql);
  for(unsigned int i=0; i<res.size();i++){
    std::vector<cv::Point> pathway;
    char path_str[100]="";
    sscanf(res[i][0].data(),"{%[^}]}",path_str);
    const char* split=":,";
    char* p;
    p=strtok(path_str,split);
    int _count=0;
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
          pathway.push_back(point);
          break;
        }
      }
      p=strtok(NULL,split);
    }
    custom_pathways.push_back(pathway);
  }
#ifdef DEBUG
  for(unsigned int k=0;k<custom_pathways.size();k++){
    for(unsigned int n=0;n<custom_pathways[k].size();n++){
      std::cout<<custom_pathways[k][n].x<<" "<<custom_pathways[k][n].y<<" ";
    }
    std::cout<<std::endl;
  }
#endif // DEBUG

}

CDBMySQL1::~CDBMySQL1(){

}

void CDBMySQL1::Save2DB(){  
#ifdef DEBUG
  printf("begin type1 save2db \n");
#endif // DEBUG

  //get result data.
  char g_filename_c[200];
#ifdef SERVER
  sprintf(g_filename_c,"/usr/local/vidy/result/%s-cid%d-count%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#else
  sprintf(g_filename_c,"../../result/%s-cid%d-count%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#endif // SERVER

  g_filename = g_filename_c;
#ifdef DEBUG
  printf("%s\n",g_filename);
#endif

  //--get calibrate roi data.
  std::ifstream inFile(g_filename,std::ios::in);
  count=0;
  female=0;
  male=0;
  std::vector<int> _directions(pathways.size()+1);
  for(unsigned int i=0;i<pathways.size()+1;i++){
    _directions[i]=0;
  }
  directions = _directions;
  std::vector<int> _custom_directions(custom_pathways.size()+1);
  for(unsigned int i=0;i<custom_pathways.size()+1;i++){
    _custom_directions[i]=0;
  }
  custom_directions = _custom_directions;
  age_1=0; //<20
  age_2=0; //21-30
  age_3=0; //31-40
  age_4=0; //41-50
  age_5=0; //51-60
  age_6=0; //>60

  //if no data, insert 0.
  if(!inFile.is_open()){
    printf("no file at %s %s:00:00..\n",g_date,g_time);
    //return;
  }
 
  while(!inFile.eof()){
    //get result data.
    int _count,_gender,_direction,_age,_custom_direction;
    inFile>>_count>>_gender>>_direction>>_age>>_custom_direction;
#ifdef DEBUG
    printf("%d %d %d %d %d\n",_count,_gender,_direction,_age,_custom_direction);
#endif
    //--count.
    count++;
    //--gender.
    if(_gender==1){
      male++;
    }else if(_gender==0){
      female++;
    }
    //--direciton.
    //direction==0 means no in each of the selected pathways.
    (directions[_direction])++;
    (custom_directions[_custom_direction])++;
    //--age.
    if(_age==20){
     _age = random(40)+20;
    }
    switch((int)(_age/10)){
      case 0:
        age_1++;
        break;
      case 1:
        age_2++;
        break;
      case 2:
        age_3++;
        break;
      case 3:
        age_4++;
        break;
      default:
        age_5++;
        break;
    }
  }
  inFile.close();

#ifdef DEBUG
  printf("SaveCount()..\n");
#endif // DEBUG  
  this->SaveCount();

#ifdef DEBUG
  printf("\nSavePathway()..\n");
#endif // DEBUG
  
  this->SavePathway();

}

void CDBMySQL1::SaveCount(){
  //--save to t_data_count.
  sprintf(sql,"insert into t_data_count(uuid,cid,datetime,date,time,count,male,female,age_group1,age_group2,age_group3,age_group4,age_group5) values(UUID(),'%d','%s %s:00:00','%s','%s:00:00','%d','%d','%d','%d','%d','%d','%d','%d')",g_cid,g_date,g_time,g_date,g_time,count,male,female,age_1,age_2,age_3,age_4,age_5);

#ifdef DEBUG
  printf("%s",sql);
#endif

  this->InsertData(sql);

}

void CDBMySQL1::SavePathway(){

  for(unsigned int i=0;i<pathways.size();i++){

    std::string sql = "insert into t_data_path_result(cid,date,time,path,num) values('";
    char ccid[10];
    sprintf(ccid,"%d",g_cid);
    sql += ccid;
    sql += "','";
    sql += g_date;
    sql += "','";
    sql += g_time;
    sql += ":00:00','{";

    std::string points;
    for(unsigned int j=0;j<pathways[i].size();j++){
      char point[50];
      sprintf(point,"%sx%d%s:%s%d%s,%sy%d%s:%s%d%s","\"",j+1,"\"","\"",pathways[i][j].x,"\"","\"",j+1,"\"","\"",pathways[i][j].y,"\"");
      points += point;
      if(j!=pathways[i].size()-1){
        points += ",";
      }
    }    

    sql += points;
    sql += "}','";

    char cdirection[10];
    sprintf(cdirection,"%d",directions[i+1]);    
    sql += cdirection;
    sql += "')";

#ifdef DEBUG
    printf("%s\n",sql.data());
#endif //DEBUG
    this->InsertData(sql.data());
  }

  for(unsigned int i=0;i<custom_pathways.size();i++){

    std::string sql = "insert into t_data_path_result(cid,date,time,path,num) values('";
    char ccid[10];
    sprintf(ccid,"%d",g_cid);
    sql += ccid;
    sql += "','";
    sql += g_date;
    sql += "','";
    sql += g_time;
    sql += ":00:00','{";

    std::string points;
    for(unsigned int j=0;j<custom_pathways[i].size();j++){
      char point[50];
      sprintf(point,"%sx%d%s:%s%d%s,%sy%d%s:%s%d%s","\"",j+1,"\"","\"",custom_pathways[i][j].x,"\"","\"",j+1,"\"","\"",custom_pathways[i][j].y,"\"");
      points += point;
      if(j!=custom_pathways[i].size()-1){
        points += ",";
      }
    }

    sql += points;
    sql += "}','";

    char cdirection[10];
    sprintf(cdirection,"%d",custom_directions[i+1]);
    sql += cdirection;
    sql += "')";

#ifdef DEBUG
    printf("%s\n",sql.data());
#endif //DEBUG
    this->InsertData(sql.data());
  }

}

} //namespace vidy
