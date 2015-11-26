#coding=utf-8
import socket, struct
import binascii
import thread
import time

def printhex(data):
        hexstr = ''
        for i in data:
		hexstr += ' ' + hex(ord(i))[2:];
        print(hexstr)

def recvdata(ip,port):
        print("ip : " + ip)
        print("port : " + str(port))
        serveradd = "/root/unixsocket"
	# s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((ip, port))
	# s.connect(serveradd)
	data = '10 00 00 00 00 00 00 10 00 00 00 00 00 FF 00 02 93 00 66'
        ss = data.split();
	result = struct.pack('>H',int('7e7e',16))
	for c2 in ss:
		result += struct.pack('>B',int(c2,16))
	s.send(result)
	while True:
                print("recv")
		header = s.recv(18)
		if not header:
			break
		taillen = int(ord(header[16]) * 256 + ord(header[17]) ) + 1
		header += s.recv(taillen)
		printhex(header)

def mutiThread(count):
    i = 0
    while i < count:
	thread.start_new_thread(recvdata,("127.0.0.1",7002))
        i = i + 1
if __name__ == '__main__':
    mutiThread(1)
    input("press enter to exit")
