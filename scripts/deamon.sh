#!/bin/sh
path="/usr/local/vidy/bin/Vidy"
while:
do 
$path CTCCHZ $1 http://10.173.164.13:8090/flv$1.flv $2 &
sleep 100
done
