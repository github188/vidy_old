//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:preblob.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_PREBLOB_H_
#define VIDY_INC_PREBLOB_H_

#define PREBLOBNUM 2

#include "blob.h"


namespace vidy{

struct PreBlob{
  Blob blob;
  int pre_index;
};

} //namespace vidy

#endif // VIDY_INC_PREBLOB_H_
