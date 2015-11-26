#! /bin/bash

pdir=`dirname $0`
cd $pdir
echo shell 执行目录 : `pwd`
echo "*********** start clean **************"
make clean -C ./Debug/

if [ $? -ne 0 ];then
exit $?
fi

echo "*********** start make **************"
make -C ./Debug/
if [ $? -ne 0 ];then
exit $?
fi

kill -9 `ps -ef|grep TransmitServer|grep -v grep | grep -v $pdir |awk '{print $2}'`

echo "*********** start service **************"

nohup ./Debug/TransmitServer &
#sleep 2 && kill -2 $!
echo "****************start Success!***********"

