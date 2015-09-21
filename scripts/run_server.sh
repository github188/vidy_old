#!/bin/sh
i=1
path="/usr/local/vidy/scripts"
cat $path/config.conf | while read line
do
b=$(($i%2))
if [ $b -eq 1 ] ; then
id=$line
else
type=$line
echo "$path/deamon.sh $id $type &"
sh $path/deamon.sh $id $type &
fi
i=$(($i+1))
done

