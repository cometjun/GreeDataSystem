#! /bin/bash

echo "*********** start clean **************"
make clean -C ./Debug/

if [ $? != 0 ];
then
exit $?
fi

echo "*********** start make **************"

make -C ./Debug/
if [ $? -ne 0 ];then
	exit $?
fi

kill -9 `ps -ef|grep GuGongServer|grep -v grep|awk '{print $2}'`

echo "*********** start service **************"

nohup ./Debug/GuGongServer &
read name
#sleep 2 && kill -2 $!
echo "****************start Success!***********"

