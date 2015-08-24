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
  //get heatmap data file.
  std::string filename;
  std::ifstream inFile(filename.data(),std::ios::in);
  std::vector<std::vector<int> > heatmapdata;
  while(!inFile.eof()){
    int x,y,value,radius;
    inFile>>x>>y>>value>>radius;
    std::vector<int> heatmap;
    heatmap.push_back(x);
    heatmap.push_back(y);
    heatmap.push_back(value);
    heatmap.push_back(radius);
    heatmapdata.push_back(heatmap);
  }
  heatmapdata.erase(heatmapdata.end());
  inFile.close();
  
  //save to t_data_heatmap
  std::string sql="insert into t_data_heatmap(cid,date,time,heatmapdata) values('"; 
  char cid[10];
  sprintf(cid,"%d",g_cid);
  sql += cid;
  sql += "','";
  sql += g_date;
  sql += "','";
  sql += g_time;
  sql += ":00:00','MULTIPOINTS(";
  for(unsigned int i=0;i<heatmapdata.size();i++){
    char _sql[50];
    sprintf(_sql,"%d,%d %d,%d ",heatmapdata[i][0],heatmapdata[i][1],heatmapdata[i][2],0);
    sql += _sql;
  }  
  sql +="')";
  this->InsertData(sql.data());
}

} //namespace vidy
