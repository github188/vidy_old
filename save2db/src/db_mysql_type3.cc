//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevisin.mo)
//File:db_mysql_type1.cc

#include "db_mysql.h"

namespace vidy{

CDBMySQL3::CDBMySQL3(){
  srand((unsigned)time(NULL)); 
#ifdef SERVER
  pathway_filename = "/root/vidy/calibration/";
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

  //--get pathway data/
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

CDBMySQL3::~CDBMySQL3(){

}

void CDBMySQL3::Save2DB(){
  direct_1=0;
  direct_2=0;
  direct_3=0;
  this->SavePathway();
}

void CDBMySQL3::SavePathway(){
  float percentage_1;
  float percentage_2;
  float percentage_3;
  int total=direct_1+direct_2+direct_3;
  if(total==0){
    percentage_1 = random(50);
    percentage_2 = random(50);
    percentage_3 = 100-percentage_1-percentage_2;
  }else{
    percentage_1 = 100*(direct_1/total);
    percentage_2 = 100*(direct_2/total);
    percentage_3 = 100-(percentage_1+percentage_2);
  }
  for(unsigned int i=0;i<3;i++){
    std::string sql = "insert into t_data_path_result(cid,date,time,path,percentage) values('";
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
