//Copyright(c) 2015 ExtremeVision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevision.mo)
//File:db_mysql.h

#ifndef VIDY_SAVE2DB_INC_DB_MYSQL_H_
#define VIDY_SAVE2DB_INC_DB_MYSQL_H_

#include "save2db_interface.h"
#include "mysql.h"
#include "opencv2/core/core.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include "configs.h"
#include "global.h"

#define random(x) (rand()%x)

namespace vidy{

class IDBMySQL : public ISave2DB{
  public:
    //get params of database.
    IDBMySQL();
    ~IDBMySQL();
    virtual int InsertData(const char* sql);
    std::vector<std::vector<std::string> > GetData(const char* sql);
    virtual void Save2DB(){};
  protected:
    int InitMySQL();
    inline int Close(){
      mysql_close(&mysql);
      return 1;
    }
    MYSQL mysql;
}; //class IDBMysql

class CDBMySQL1 : public IDBMySQL{
  public:
    CDBMySQL1();
    ~CDBMySQL1();
    virtual void Save2DB();
  protected:
    void SaveCount();
    void SavePathway();
  private:
    std::string pathway_filename;
    std::vector<std::vector<cv::Point> > pathways;
    //raw data.
    int count;
    int female;
    int male;
    int direct_1;
    int direct_2;
    int direct_3;
    int age_1;
    int age_2;
    int age_3;
    int age_4;
    int age_5;
    int age_6;
    //sql sentence.
    char sql[1024];
}; //class CDBMySQL1

class CDBMySQL2 : public IDBMySQL{
  public:
    CDBMySQL2();
    ~CDBMySQL2();
    virtual void Save2DB();
}; //class CDBMySQL2

class CDBMySQL3 : public IDBMySQL{
  public:
    CDBMySQL3();
    ~CDBMySQL3();
    virtual void Save2DB();
  protected:
    void SavePathway();
  private:
    std::string pathway_filename;
    std::vector<std::vector<cv::Point> > pathways;
    //raw data.
    int direct_1;
    int direct_2;
    int direct_3;
    //sql sentence.
    char sql[1024];
}; //class CDBMySQL3

} //namespace vidy


#endif
