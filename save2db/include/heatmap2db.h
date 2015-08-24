//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:heatmap2db.h
//Author:Yun Luo(lauren.luo@extremevision.mo)
//Save heatmap data to database.
//Data file dir is ~/ExtremeVision-data/ .
//Data file name : heatmap+[datatime].dat
//Format of the file: X  Y  VALUE 

#ifndef VIDY_SAVE2DB_INC_HEATMAP2DB_H_
#define VIDY_SAVE2DB_INC_HEATMAP2DB_H_

#include <iostream>
#include <fstream>

#include "db_mysql.h"

namespace vidy{

class Heatmap2DB:public IDBMySQL{

public:
  explicit Heatmap2DB(const int cid);
  ~Heatmap2DB();
  void saveheatmap();
}; //class Heatmap2DB

} //namespace vidy

#endif
