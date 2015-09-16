//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:heatmap.h
//Author:Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_HEATMAP_H_
#define VIDY_INC_HEATMAP_H_

#include "global.h"

namespace vidy{

struct Heatmap{
  int x;
  int y;
  int value;
  int radius;
};

} // namespace vidy


#endif // VIDY_INC_HEATMAP_H_ 
