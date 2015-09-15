//Copyright(c) 2015 Extreme Vision Ltd. All rights reserved.
//File:global.cc
//Author:Yun Luo(lauren.luo@extremevision.mo).

#include "global.h"

char* g_cid="";
char* g_dbname="";
int g_type = 0;

std::vector<cv::Point> g_calibrate;

std::vector<Pathway> g_pathways;

std::vector<cv::Point> g_area;

int g_count=0;
int g_female=0;
int g_male=0;

char* g_time="";

char g_time_sequence[50] = "0000-00-00 00:00:00";
