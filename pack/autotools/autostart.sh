#!/bin/bash
currentpath=`dirname $0`
conffile=$currentpath/auto.conf
outfile=$currentpath/alog/autostart-`date +%Y%m`.out
dateformat=`date +%Y-%m-%d\ %H:%M:%S`
# 检测配置文件是否存在
if [ ! -f $conffile ];then
	echo 没有配置文件>> $outfile
	exit
fi
cat $conffile | while read LINE
do
	portname=`echo $LINE | cut -f1 -d ' '`
	sshell=`echo $LINE | cut -f2 -d ' '`
	if [[ $portname =~ ^[0-9]+$ ]];then 
		linenum=`netstat -an | grep 0:$portname | wc -l`
		if [ $linenum -eq 0 ];then
			echo $dateformat 端口 [$portname] 不存在 >>$outfile
			$sshell >> $outfile
		fi
	else # file name
		linenum=`ps -ef | grep $portname | grep -v grep | wc -l`
		if [ $linenum -eq 0 ];then
			echo $dateformat 进程 [$portname] 不存在 >>$outfile
			$sshell >> $outfile
		fi
	fi
done

