//Copyright(c) 2015 ExtremeVision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevision.mo)
//File:db_mysql_interface.cc

#include "db_mysql.h"
#include "configs.h"
#include "stdio.h"

#define PORT 3306

namespace vidy{

IDBMySQL::IDBMySQL(){
  this->InitMySQL();
}

IDBMySQL::~IDBMySQL(){
  Close();
}

int IDBMySQL::InsertData(const char* sql){
  if(!mysql_query(&mysql,sql)){  //return 1 for insert success.
    return 1;
  }else{
    printf("[error] mysql insert failed.\n");
    return 0;
  }
}

std::vector<std::vector<std::string> > IDBMySQL::GetData(const char* sql){
  std::vector<std::vector<std::string> > vec_result;
  MYSQL_ROW sql_row;
  MYSQL_RES* result;
  MYSQL_FIELD* fd;
  int res;
  res=mysql_query(&mysql,sql);
  if(!res){
    result=mysql_store_result(&mysql);
    if(result){
      int field_num = mysql_num_fields(result);
      while(sql_row=mysql_fetch_row(result)){
        std::vector<std::string> _vec_result;
        for(unsigned int i=0;i<field_num;i++){
          _vec_result.push_back(sql_row[i]);
        }
        vec_result.push_back(_vec_result);
      }
    }
  }
  mysql_free_result(result);
  return vec_result;
}

int IDBMySQL::InitMySQL(){
  if(!mysql_init(&mysql)){
    printf("[error] Memory insufficient!\n");
    return 0;
  }else{
    printf("mysql init success!\n");
  }
  
  if(!mysql_real_connect(&mysql,g_dbaddr,g_dbuser,g_dbpswd,g_dbname2,PORT,NULL,0)){
    fprintf(stderr,"[error] Failed to connect to database: Error: %s\n",mysql_error(&mysql));
    return 0;
  }else{
    printf("Connect to Mysql successfully!\n");
    printf("database:%s\n",g_dbname2);
    mysql_query(&mysql,"set names utf8");
    return 1;
  }
}

} // namespace vidy
