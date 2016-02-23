#coding=utf-8
import binascii,struct

def bin2hex(data,sp=' '):
    """
    二进制数据转换为十六进制字符串,结果中间用分隔符隔开
    sp : 分隔符
    """
    if len(data) < 1:
        return ''

    hexstr = hex(ord(data[0]))

    for i in range(1,len(data)):
        hexstr += sp + hex(ord(data[i]));

    return hexstr

def str2hex(hexstr):
    """ 十六进制字符串转换为二进制字符串,原格式用空格隔开 """
    hexArray = hexstr.split()
    
    result = ''
    for i in hexArray:
            result += struct.pack('>B',int(i,16))

    return result

if __name__ == '__main__':
    str = '32 33 41 42 43 44' # 0 1 2 3 A B C D
    result = str2hex(str)
    print(result)
    result = bin2hex(result,sp=' ')
    print(result)
