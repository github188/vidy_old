//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File: global.h
//Author: Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_GLOBAL_H_
#define VIDY_INC_GLOBAL_H_

#include <vector>
#include "opencv2/core/core.hpp"

#define WIDTH 1280
#define HEIGHT 720
#define FPS 5

//default lowest staytime(seconds).
#define LOWEST_STAYTIME 5

extern char* g_cid;
extern char* g_dbname;
extern int g_type;
extern char* g_data_path;

//door calibration.
extern std::vector<cv::Point> g_calibrate;

extern char* g_time;

//--data--
extern int g_count;
extern int g_count_realtime;
extern int g_female;
extern int g_male;

#endif //VIDY_INC_GLOBAL_H_
