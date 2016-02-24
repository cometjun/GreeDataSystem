#coding=utf-8
""" 模拟接收客户端 """
import socket, struct
import binascii
import thread
import time
import util

def recvdata(ip,port):
        """ 接收转发客户端数据并以十六进制打印 """
        print("ip : " + ip)
        print("port : " + str(port))

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((ip, port))

	data93 = '7e 7e 10 00 00 00 00 00 00 10 00 00 00 00 00 FF 00 02 93 00 66'
        data93 = util.str2hex(data93)

	s.send(result)

	while True:
		data = s.recv(18)     # 接收报文头
		if not data:
			break

		taillen = int(ord(data[16]) * 256 + ord(data[17]) ) + 1     # 报文剩下部分的长度
		data += s.recv(taillen)
		print(util.bin2hex(data))    # 打印结果

def mutiThread(count):
    i = 0
    while i < count:
	thread.start_new_thread(recvdata,("127.0.0.1",7002))
        i = i + 1
if __name__ == '__main__':
    mutiThread(1)
    input("press enter to exit")
