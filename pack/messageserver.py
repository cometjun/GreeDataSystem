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

def mserver(port):
        print("port : " + str(port))
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind(("127.0.0.1",port))
	s.listen(10)

	while True:
		print("accept")
		clientsock,address = s.accept()
		while True:
			mes = clientsock.recv(1024)
			if not mes:
				clientsock.close()
				break;
			printhex(mes)

def mutiThread(count):
	thread.start_new_thread(mserver,(7003,))

if __name__ == '__main__':
    mutiThread(1)
    input("press enter to exit")
