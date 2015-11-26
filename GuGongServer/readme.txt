1. src/ip.cfg 配置 -- 服务器配置
	messageip : 短信服务器 ip 地址
	messageport : 短信服务器端口
	gprsport : gprs 要连接的服务器端口
	
2. conf/zlog.conf 配置 -- 配置 log 文件格式与存储位置
	
3. src/syscount.txt -- 各个系统写文件名配置
		# 配置格式 ： 系统标识(900000) + 空格 + 开始写文件序列号
		
4. 启动程序
	./start.sh
	
注意 ：
	删除 unix 域套接字
	修改配置文件位置 BufferedFileName.h