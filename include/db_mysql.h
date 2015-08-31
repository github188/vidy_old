//Copyright(c) 2015 ExtremeVision Ltd. All rights reserved.
//Author: Yun Luo(lauren.luo@extremevision.mo)
//File:db_mysql.h

#ifndef VIDY_SAVE2DB_INC_DB_MYSQL_H_
#define VIDY_SAVE2DB_INC_DB_MYSQL_H_

#include "mysql.h"
#include "opencv2/core/core.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include "configs.h"

#define random(x) (rand()%x)

namespace vidy{

class IDBMySQL{
  public:
    //get params of database.
    IDBMySQL();
    ~IDBMySQL();

    int InsertData(const char* sql);

    //Get data from databese with sql sentence.
    //Each std::vector<std::string> > is a row and each std::string is a field.
    //usage sample:
    //std::vector<std::vector<std::string> > res = this->GetData("select * from t_data_staytime_day");
    //for(int i=0;i<res.size();i++){
    //  for(int j=0;j<res[i].size();j++){
    //  std::cout<<res[i][j]<<" ";
    //}
    //  std::cout<<std::endl;
    //}
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

} //namespace vidy


#endif
