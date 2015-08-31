//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:blob_generate.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_BLOB_GENERATE_H_
#define VIDY_INC_BLOB_GENERATE_H_

#include <iostream>
#include "blob.h"
#include "gender_detect.h"
#include "age_estimate.h"

#ifdef TESTVIEW
#include "db_mysql.h"
#endif // TESTVIEW

namespace vidy{

class CBlobGenerate{
public:
  CBlobGenerate();
  ~CBlobGenerate();
  void Generate2(BlobNodeList& endBlobNodeList);

#ifdef TESTVIEW
  void GenerateTestView(BlobNodeList& endBlobNodeList);
#endif // TESTVIEW

private:
  GenderDetect* genderdetect;
  AgeEstimate* ageestimate;

#ifdef TESTVIEW
  IDBMySQL* dbmysql;
#endif // TESTVIEW


}; //class CBlobGenerate

} //namespace vidy

#endif //VIDY_INC_BLOB_GENERATE_H_
