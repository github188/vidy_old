//Copyright(c) 2015 ExtremeVision Ltd. All rights reserved.
//Author: Yun Luo(laurne.luo@extremevision.mo)
//File:save2db_interface.h

#ifndef VIDY_SAVE2DB_INC_SAVE2DB_INTERFACE_H_
#define VIDY_SAVE2DB_INC_SAVE2DB_INTERFACE_H_

namespace vidy{

class ISave2DB{

public:
  virtual void Save2DB()=0;

}; //class ISave2DB

} //namespace vidy

#endif // VIDY_SAVE2DB_INC_SAVE2DB_INTERFACE_H_
