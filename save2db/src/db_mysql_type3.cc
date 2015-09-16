//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevisin.mo)
//File:db_mysql_type1.cc

#include "db_mysql.h"

namespace vidy{

CDBMySQL3::CDBMySQL3(){
  srand((unsigned)time(NULL)); 
  //--get pathway data from database/
  char sql[100];
  sprintf(sql,"select calibration_data from t_calibration where cid='%d' and typeid='2' and path_type='default'",g_cid);
  std::vector<std::vector<std::string> > res = this->GetData(sql);
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
        case 4:{
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
  char sql[100];
  sprintf(sql,"select calibration_data from t_calibration where cid='%d' and typeid='2' and path_type='custom'",g_cid);
  std::vector<std::vector<std::string> > res = this->GetData(sql);
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
        case 4:{
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

}

CDBMySQL3::~CDBMySQL3(){

}

void CDBMySQL3::Save2DB(){
#ifdef DEBUG
  printf("begin type3 save2db \n");
#endif // DEBUG

  this->SavePathway();
  this->SavePathwayCustom():
  this->SaveStaytime();
  this->SaveCount();
  this->SaveHeatmap();
}

void CDBMySQL3::SaveStaytime(){
  //get result data.
  char g_filename_c[200];
#ifdef SERVER
  sprintf(g_filename_c,"/usr/local/vidy/result/%s-cid%d-pathway%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#else
  sprintf(g_filename_c,"../../result/%s-cid%d-pathway%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#endif // SERVER

  g_filename = g_filename_c;
#ifdef DEBUG
  printf("%s\n",g_filename);
#endif
  std::ifstream inFile(g_filename,std::ios::in);

  //if no data, insert 0.
  if(!inFile.is_open()){
    printf("no file at %s %s:00:00..\n",g_date,g_time);
    //return;
  }

  int staytime_sec=0;
  
  while(!inFile.eof()){
    count++;
    int _staytime_sec;
    inFile>>_staytime_sec;
    staytime_sec = (staytime_sec*(count-1)+_staytime_sec)/count;
  }
  inFile.close();

  //save data.
  float staytime_min = staytime_sec/60;
  sprintf(sql,"insert into t_data_staytime(cid,datetime,date,time,avgstay) values('%d','%s %s:00:00','%s','%s:00:00','%.2f')",g_cid,g_date,g_time,g_date,g_time,staytime_min);

#ifdef DEBUG
  printf("%s",sql);
#endif

  this->InsertData(sql);


}

void CDBMySQL3::SaveCount(){
  //save data.
  sprintf(sql,"insert into t_data_count(cid,datetime,date,time,count) values('%d','%s %s:00:00','%s','%s:00:00','%.2f')",g_cid,g_date,g_time,g_date,g_time,count);

#ifdef DEBUG
  printf("%s",sql);
#endif

  this->InsertData(sql);

}

void CDBMySQL3::SaveHeatmap(){
  //get result data.
  char g_filename_c[200];
#ifdef SERVER
  sprintf(g_filename_c,"/usr/local/vidy/result/%s-cid%d-heatmap%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#else
  sprintf(g_filename_c,"../../result/%s-cid%d-heatmap%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#endif // SERVER

  g_filename = g_filename_c;
#ifdef DEBUG
  printf("%s\n",g_filename);
#endif

  if(!inFile.is_open()){
    printf("no file at %s %s:00:00..\n",g_date,g_time);
    //return;
  }

  //save to t_data_heatmap
  std::string sql="insert into t_data_heatmap(cid,date,time,heatmapdata) values('";
  char cid[10];
  sprintf(cid,"%d",g_cid);
  sql += cid;
  sql += "','";
  sql += g_date;
  sql += "','";
  sql += g_time;
  sql += ":00:00',GeomFromText('MULTIPOINT(";
  while(!inFile.eof()){
    int x,y;
    float valuei,radius;
    inFile>>x>>y>>value>>radius;
    char _sql[50];
    sprintf(_sql,"%d %d,%d %d,",x,y,(int)value,radius);
    sql += _sql;
  }
  sql +="0 0,0 0)',0))";
#ifdef DEBUG
  printf("%s\n",sql.data());
#endif
  inFile.close();
  this->InsertData(sql.data());
}

void CDBMySQL3::SavePathway(){
  //get result data.
  char g_filename_c[200];
#ifdef SERVER
  sprintf(g_filename_c,"/usr/local/vidy/result/%s-cid%d-pathway%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#else
  sprintf(g_filename_c,"../../result/%s-cid%d-pathway%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#endif // SERVER

  g_filename = g_filename_c;
#ifdef DEBUG
  printf("%s\n",g_filename);
#endif

  std::vector<int> _directions(pathways.size()+1);
  directions = _directions;

  std::ifstream inFile(g_filename,std::ios::in);
  
  //if no data, insert 0.
  if(!inFile.is_open()){
    printf("no file at %s %s:00:00..\n",g_date,g_time);
    //return;
  }
  
  while(!inFile.eof()){
    int _direction;
    inFile>>_direction;
    (directions[_direction])++;
  }

  inFile.close();

  //save data.
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
      sprintf(point,"%sx%d%s:%s%d%s,%sy%d%s:%s%d%s","\"",i,"\""."\"",pathways[i][j].x,"\"","\"",i,"\"","\"",pathways[i][j].y,"\"");
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
}

void CDBMySQL3::SavePathwayCustom(){
  //get result data.
  char g_filename_c[200];
#ifdef SERVER
  sprintf(g_filename_c,"/usr/local/vidy/result/%s-cid%d-custom_pathway%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#else
  sprintf(g_filename_c,"../../result/%s-cid%d-custom_pathway%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#endif // SERVER

  g_filename = g_filename_c;
#ifdef DEBUG
  printf("%s\n",g_filename);
#endif

  std::vector<int> _directions(custom_pathways.size()+1);
  directions = _directions;

  std::ifstream inFile(g_filename,std::ios::in);

  //if no data, insert 0.
  if(!inFile.is_open()){
    printf("no file at %s %s:00:00..\n",g_date,g_time);
    //return;
  }

  while(!inFile.eof()){
    int _direction;
    inFile>>_direction;
    (directions[_direction])++;
  }

  inFile.close();

  //save data.
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
      sprintf(point,"%sx%d%s:%s%d%s,%sy%d%s:%s%d%s","\"",i,"\""."\"",pathways[i][j].x,"\"","\"",i,"\"","\"",pathways[i][j].y,"\"");
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
}

} //namespace vidy
