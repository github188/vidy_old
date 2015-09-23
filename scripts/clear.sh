#!/bin/sh
ps -ef|grep deamon | grep -v grep | cut -c 9-15|xargs kill -9
ps -ef|grep Vidy |grep -v grep|cut -c 9-15|xargs kill -9
