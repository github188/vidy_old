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
  sprintf(sql,"select astext(calibration_data) from t_calibration where cid='%d' and typeid='2'",g_cid);
  std::vector<std::vector<std::string> > res = this->GetData(sql);
  for(unsigned int i=0; i<res.size();i++){
    std::vector<cv::Point> pathway;
    char path_str[100]="";
    sscanf(res[i][0].data(),"MULTIPOINT(%[^)])",path_str);
    const char* split=", ";
    char* p;
    p=strtok(path_str,split);
    int _count=0;
    cv::Point point;
    while(p!=NULL){
      _count++;
      if(_count%2==1){
        //x
        point.x=atoi(p);
      }else{
        //y
        point.y=atoi(p);
        pathway.push_back(point);
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
  std::vector<int> _directions(pathways.size());
  directions = _directions;
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
    (directions[_direction-1])++;
    //--age.
    _age = random(40)+20;
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
  //compute total directions number.
  int direction_total = 0;
  for(unsigned int i=0;i<directions.size();i++){
    direction_total += directions[i];
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

    std::string points;
    for(unsigned int j=0;j<pathways[i].size();j++){
      char point[50];
      sprintf(point,"%d %d",pathways[i][j].x,pathways[i][j].y);
      points += point;
      if(j!=pathways[i].size()-1){
        points += ",";
      }
    }    
    sql += points;
    sql += ")',0),'";

    char cdirection[10];
    sprintf(cdirection,"%d",directions[i]);    
    sql += cdirection;
    sql += "','";
     
    char cpercentage[10];
    sprintf(cpercentage,"%.2f",100.00f*directions[i]/direction_total);
    sql += cpercentage;
    sql += "')";

#ifdef DEBUG
    printf("%s\n",sql.data());
#endif //DEBUG
    this->InsertData(sql.data());

  }
}

} //namespace vidy
