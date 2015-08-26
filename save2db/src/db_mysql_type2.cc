//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevisin.mo)
//File:db_mysql_type2.cc

#include "db_mysql.h"
#include <iostream>
#include <fstream>

namespace vidy{

CDBMySQL2::CDBMySQL2(){

}

CDBMySQL2::~CDBMySQL2(){

}

void CDBMySQL2::Save2DB(){
  //get result data.
  char g_filename_c[200];
#ifdef SERVER
  sprintf(g_filename_c,"/root/vidy/result/%s-cid%d-heatmap%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#else
  sprintf(g_filename_c,"../../result/%s-cid%d-heatmap%s%s.dat",g_dbname,g_cid,g_date2,g_time);
#endif // SERVER
  
  g_filename = g_filename_c;

#ifdef DEBUG
  printf("%s",g_filename_c);
#endif //DEBUG


  //get heatmap data file.
  std::ifstream inFile(g_filename,std::ios::in);

  if(!inFile.is_open()){
    printf("no file at %s %s:00:00..\n",g_date,g_time);
    return;
  }

  //average stay time.
  int staytime_sec=0;
  
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
    float value;
    inFile>>x>>y>>value;
    char _sql[50];
    sprintf(_sql,"%d %d,%d %d,",x,y,(int)value,5);
    sql += _sql;
    staytime_sec += (int)value;
  }  
  sql +="0 0,0 0)',0))";
#ifdef DEBUG
  printf("%s\n",sql.data());
#endif
  inFile.close();
  this->InsertData(sql.data());
}

} //namespace vidy
