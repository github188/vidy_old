//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File: direction.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_DIRECTION_H_
#define VIDY_INC_DIRECTION_H_

#include "blob.h"

#include <vector>

namespace vidy{

enum DirectionStruct{
  Up_branch1=0,
  Up_branch2=1,
  Up_branch3=2,
  Down_branch=3,
  Down_branch2=4,
  Down_branch3=5
}; //direction.

class Direction{

public:
  Direction();
  ~Direction();
  //DirectionStruct GetDirection(BlobNodeList blobnodelist);

}; //class Direction

} //namespace vidy

#endif //VIDY_INC_DIRECTION_H_
