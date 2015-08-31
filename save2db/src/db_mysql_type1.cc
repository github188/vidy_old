//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevisin.mo)
//File:db_mysql_type1.cc

#include "db_mysql.h"
#include <stdio.h>
#include <iostream>

namespace vidy{

CDBMySQL1::CDBMySQL1(){
#ifdef SERVER
  pathway_filename = "/usr/local/vidy/calibration/";
#else
  pathway_filename = "../../calibration/";
#endif // SERVER
  pathway_filename += g_dbname;
  pathway_filename += "-cid";
  char ccid[10];
  sprintf(ccid,"%d",g_cid);
  pathway_filename += ccid;
  pathway_filename += "-pathway.dat";
#ifdef DEBUG
  printf("%s\n",pathway_filename.data());
#endif

  //--get pathway data from database/
  char sql[100];
  sprintf(sql,"select path from t_calibration where cid='%d' and type='2'",g_cid);
  std::vector<std::vector<std::string> > res = this->GetData(sql);
  for(unsigned int i=0; i<res.size();i++){
    char path_str[100];
    sscanf(res[i].data(),"GeomFromText('MULTIPOINT(%s)',0)",path_str);
    for(unsigned int j=0;j<strlen(path_str);++j){
      if(path_str[j]==','){
        
      }
    }
  }

  std::ifstream inFile2(pathway_filename.data(),std::ios::in);
  while(!inFile2.eof()){
    cv::Point p1,p2,p3;
    inFile2>>p1.x>>p1.y>>p2.x>>p2.y>>p3.x>>p3.y;
    std::vector<cv::Point> pathway;
    pathway.push_back(p1);
    pathway.push_back(p2);
    pathway.push_back(p3);
    pathways.push_back(pathway);
  }
  pathways.erase(pathways.end());
  inFile2.close();
#ifdef DEBUG
  printf("pathway size : %d\n",pathways.size());
#endif  
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
  direct_1=0;
  direct_2=0;
  direct_3=0;
  age_1=0; //<20
  age_2=0; //21-30
  age_3=0; //31-40
  age_4=0; //41-50
  age_5=0; //51-60
  age_6=0; //>60

  if(!inFile.is_open()){
    printf("no file at %s %s:00:00..\n",g_date,g_time);
    return;
  }
 
  while(!inFile.eof()){
    int _count,_gender,_direction,_age;
    inFile>>_count>>_gender>>_direction>>_age;
#ifdef DEBUG
    printf("%d %d %d %d\n",_count,_gender,_direction,_age);
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
    switch(_direction){
      case 1:
        direct_1++;
        break;
      case 2:
        direct_2++;
        break;
      case 3:
        direct_3++;
        break;
    }
    //--age.
    _age = ramdom(40)+20;
    switch((int)(_age/10)){
      case 0:
        age_1++;
        break;
      case 1:
        age_1++;
        break;
      case 2:
        age_2++;
        break;
      case 3:
        age_3++;
        break;
      case 4:
        age_4++;
        break;
      case 5:
        age_5++;
        break;
      case 6:
        age_6++;
        break;
    }
  }
  inFile.close();

#ifdef DEBUG
  printf("SaveCount()..\n");
#endif // DEBUG  
  this->SaveCount();

#ifdef DEBUG
  printf("SavePathway()..\n");
#endif // DEBUG
  
  //note. web server is not ready. 
  //this->SavePathway();

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
  float percentage_1;
  float percentage_2;
  float percentage_3;
  int total=direct_1+direct_2;
  total += direct_3;
  if(total==0){
    percentage_1 = 30;
    percentage_2 = 30;
    percentage_3 = 40;
  }else{
    percentage_1 = 100*(direct_1/total);
    percentage_2 = 100*(direct_2/total);
    percentage_3 = 100-(percentage_1+percentage_2);
  }
  for(unsigned int i=0;i<pathways.size();i++){
    std::string sql = "insert into t_data_path_result(cid,date,time,path,num,percentage) values('";
    char ccid[10];
    sprintf(ccid,"%d",g_cid);
    sql += ccid;
    sql += "','";
    sql += g_date;
    sql += "','";
    sql += g_time;
    sql += ":00:00',GeomFromText('MULTIPOINT(";
    char points[100];
    sprintf(points,"%d %d,%d %d,%d %d",pathways[i][0].x,pathways[i][0].y,pathways[i][1].x,pathways[i][1].y,pathways[i][2].x,pathways[i][2].y);
    sql += points;
    sql += ")',0),'";

    char cpercentage[10];
    switch(i){
      case 0:
        sprintf(cpercentage,"%f",percentage_1);
        break;
      case 1:
        sprintf(cpercentage,"%f",percentage_2);
        break;
      case 2:
        sprintf(cpercentage,"%f",percentage_3);
        break;
    }
    sql += cpercentage;
    sql += "')";
#ifdef DEBUG
    printf("%s\n",sql.data());
#endif //DEBUG
    this->InsertData(sql.data());
  }
}

} //namespace vidy
