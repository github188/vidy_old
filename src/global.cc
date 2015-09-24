//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:global.cc
//Author:Yun Luo(lauren.luo@extremevision.mo).

#include "global.h"

char* g_cid="";
char* g_dbname="";
int g_type = 0;

std::vector<cv::Point> g_calibrate;

std::vector<Pathway> g_pathways;
std::vector<Pathway> g_pathways_custom;

std::vector<cv::Point> g_area;

int g_count=0;
int g_enter=0;
int g_exit=0;
int g_female=0;
int g_male=0;
int g_age1=0;
int g_age2=0;
int g_age3=0;
int g_age4=0;
int g_age5=0;

int g_direction=0;
int g_direction_custom=0;

char* g_time="";

char g_time_sequence[50] = "0000-00-00 00:00:00";

cv::Rect g_roi;
