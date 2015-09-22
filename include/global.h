//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File: global.h
//Author: Yun Luo(lauren.luo@extremevision.mo)

#ifndef VIDY_INC_GLOBAL_H_
#define VIDY_INC_GLOBAL_H_

#include <vector>
#include <string>
#include "opencv2/core/core.hpp"

#define WIDTH 1280
#define HEIGHT 720
#define FPS 5

//default lowest staytime(seconds).
#define LOWEST_STAYTIME 5

//--inputs(argv[])--
extern char* g_cid;
extern char* g_dbname;
extern int g_type;

//--door calibration(for g_type==1).--
extern std::vector<cv::Point> g_calibrate;

//--pathways.--
typedef std::vector<cv::Point> Pathway;
extern std::vector<Pathway> g_pathways;
extern std::vector<Pathway> g_pathways_custom;

//--areas.--
extern std::vector<cv::Point> g_area;

//--time sequence--
extern char g_time_sequence[50];

//--time--
extern char* g_time;

//--data--
extern int g_count;
extern int g_enter;
extern int g_exit;
extern int g_female;
extern int g_male;
extern int g_age1; //age less than 20.
extern int g_age2; //age between 20-30.
extern int g_age3; //age between 30-40.
extern int g_age4; //age between 40-50.
extern int g_age5; //age more than 50.

extern int g_direction;

//--roi--
extern cv::Rect g_roi;

#endif //VIDY_INC_GLOBAL_H_
