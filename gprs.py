#coding=utf-8
""" 模拟 gprs 模块 """
import socket, struct
import binascii
import thread
import time
import util

def printhex(data):
        hexstr = ''
        for i in data:
            hexstr += ' ' + hex(ord(i));
        print(hexstr)

def gprsThread(num,mod):
        """ num 最大为 0xFFFF """
        print("mac : " + str(num))
        if num < 0:
            return

        if num <= 0xFF:
            srcmac = '10 00 00 00 00 00 ' + hex(num)
        elif num <= 0xFFFF:
            srcmac = '10 00 00 00 00 ' + hex((num & 0xFF00)>>8) + ' ' + hex(num & 0x00FF)
        else:
            return

	data90 = '7e 7e 10 00 10 00 00 00 01 ' + srcmac + ' 00 06 90 06 5F 00 01 41 42'
	data91 = '7e 7e 10 00 10 00 00 00 01 ' + srcmac + ' 00 06 91 ' + hex(mod) + ' 5F 00 01 41 42'

	# dataF3 = '10 00 10 00 00 00 01 10 00 00 00 00 00 ' + str(num) + ' 00 02 F3 11 66'
        data = '7e 7e 22 22 22 22 22 22 22 66 10 40 20 28 02 39 03 ED 96 73 0A 06 0F 24 35 02 01 00 AA AA 82 F6 7F 49 12 14 06 42 FF 00 AA AA 87 D6 7F 49 12 14 00 00 00 00 00 00 61 FF 00 AA AA 82 F6 7F 4A 12 0D 07 59 FF 00 AA AA 82 F6 7F 4A 12 14 06 41 FF 00 AA AA 87 D6 7F 4A 12 14 00 00 00 00 00 00 62 FF 00 AA AA 82 F6 7F 55 12 0D 07 46 FF 00 AA AA 82 F6 7F 55 12 14 06 5E FF 01 AA AA 82 F6 7F 5D 12 0D 07 4E FF 01 AA AA 82 F6 7F 5D 12 14 06 56 FF 01 AA AA 87 D6 7F 4B 12 14 00 00 00 00 00 00 63 FF 01 AA AA 82 F6 7F 42 12 0D 07 51 FF 01 AA AA 88 D6 7F 42 12 0D 00 00 00 00 00 00 00 7C FF 01 AA AA 82 F6 7F 42 12 14 06 49 FF 01 AA AA 87 D6 7F 42 12 14 00 00 00 00 00 00 6A FF 01 AA AA 82 F6 7F 4D 12 0D 07 5E FF 01 AA AA 82 F6 7F 4D 12 14 06 46 FF 01 AA AA 82 F6 7F 50 12 0D 07 43 FF 01 AA AA 82 F6 7F 50 12 14 06 5B FF 01 AA AA 82 F6 7F 51 12 0D 07 42 FF 01 AA AA 87 D6 7F 4D 12 14 00 00 00 00 00 00 65 FF 01 AA AA 82 F6 7F 51 12 14 06 5A FF 01 AA AA 82 F6 7F 52 12 0D 07 41 FF 01 AA AA 88 D6 7F 52 12 0D 00 00 00 00 00 00 00 6C FF 01 AA AA 82 F6 7F 52 12 14 06 59 FF 01 AA AA 87 D6 7F 52 12 14 00 00 00 00 00 00 7A FF 01 AA AA 87 D6 7F 4E 12 14 00 00 00 00 00 00 66 FF 01 AA AA 82 F6 7F 53 12 0D 07 40 FF 01 AA AA 82 F6 7F 53 12 14 06 58 FF 01 AA AA 82 F6 7F 54 12 0D 07 47 FF 01 AA AA 82 F6 7F 54 12 14 06 5F FF 01 AA AA 88 D6 7F 50 12 0D 00 00 00 00 00 00 00 6E FF 01 AA AA 87 D6 7F 50 12 14 00 00 00 00 00 00 78 FF 01 AA AA 82 F6 7F 56 12 0D 07 45 FF 01 AA AA' 
        data = data + ' 82 F6 7F 56 12 14 06 5D FF 01 AA AA 82 F6 7F 57 12 0D 07 44 FF 01 AA AA 87 D6 7F 51 12 14 00 00 00 00 00 00 79 FF 01 AA AA 82 F6 7F 57 12 14 06 5C FF 01 AA AA 88 F7 7F 53 20 00 7E 84 46 85 46 7D 7E 0F FF 02 AA AA 82 F6 7F 58 12 0D 07 4B FF 02 AA AA 82 F6 7F 58 12 14 06 53 FF 02 AA AA 87 D6 7F 53 12 14 00 00 00 00 00 00 7B FF 02 AA AA 82 F6 7F 59 12 0D 07 4A FF 02 AA AA 82 F6 7F 59 12 14 06 52 FF 02 AA AA 82 F6 7F 5A 12 0D 07 49 FF 02 AA AA 87 D6 7F 54 12 14 00 00 00 00 00 00 7C FF 02 AA AA 82 F6 7F 5A 12 14 06 51 FF 02 AA AA 82 F6 7F 5B 12 0D 07 48 FF 02 AA AA 82 F6 7F 5B 12 14 06 50 FF 02 AA AA 82 F6 7F 5C 12 0D 07 4F FF 02 AA AA 87 D6 7F 55 12 14 00 00 00 00 00 00 7D FF 02 AA AA 82 F6 7F 5C 12 14 06 57 FF 02 AA AA 88 D6 7F 56 12 0D 00 00 00 00 00 00 00 68 FF 02 AA AA 87 D6 7F 56 12 14 00 00 00 00 00 00 7E FF 02 AA AA 82 F6 7F 5F 12 0D 07 4C FF 02 AA AA 82 F6 7F 5F 12 14 06 54 FF 02 AA AA 82 F6 7F 60 12 0D 07 73 FF 02 AA AA 87 D6 7F 57 12 14 00 00 00 00 00 00 7F FF 02 AA AA 82 F6 7F 60 12 14 06 6B FF 02 AA AA 82 F6 7F 62 12 0D 07 71 FF 02 AA AA 82 F6 7F 62 12 14 06 69 FF 02 AA AA 87 D6 7F 58 12 14 00 00 00 00 00 00 70 FF 02 AA AA 82 F6 7F 63 12 0D 07 70 FF 02 AA AA 82 F6 7F 63 12 14 06 68 FF 02 AA AA 82 F6 7F 27 12 0D 07 34 FF 02 AA AA 88 D6 7F 27 12 0D 00 00 00 00 00 00 00 19 FF 02 AA AA 82 F6 7F 27 12 14 06 2C FF 02 AA AA 87 D6 7F 27 12 14 00 00 00 00 00 00 0F FF 02 AA AA 87 D6 7F 59 12 14 00 00 00 00 00 00 71 FF 02 AA AA 82 F6 7F 2F 12 0D 07 3C FF 02 AA AA 88 D6 7F 2F 12 0D 00 00 00 00 00 00 00 11 FF 02 AA AA' + ' 82 F6 7F 2F 12 14 06 24 FF 03 AA AA 87 D6 7F 2F 12 14 00 00 00 00 00 00 07 FF 83'

        sendData90 = util.str2hex(data90)
        sendData91 = util.str2hex(data91)
        sendData = util.str2hex(data)

        print(util.bin2hex(sendData90))
        return

	address = '127.0.0.1'
	port = 7103    #GPRS 7101
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	s.connect((address, port))
	s.send(result90)
	s.send(result91)

	while True:
		len = s.send(result)
                time.sleep(0.1)
        # da = s.recv(30)
        # print("%s data" % da)

def mutiThread(count):
    i = 0
    while i < count/2:
	thread.start_new_thread(gprsThread,(i,03))
        i = i + 1
    while i < count:
	thread.start_new_thread(gprsThread,(i,00))
        i = i + 1

if __name__ == '__main__':
    mutiThread(1)
    input("press enter to exit")
