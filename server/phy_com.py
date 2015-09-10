#########################################################################
# File Name: phy_com.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import sys
import socket
import struct

PACK_FORMAT = '@bbhhH'

MAX_PACKAGE_SIZE = 2048
MIN_PACKAGE_SIZE = 12

SERVER_ID = 0

#package type code
PACK_TYPE_CTRL = -2
PACK_TYPE_DATA = -3
PACK_TYPE_ACK = -4
PACK_TYPE_REQ = -5

#package control code
CTRL_LED_OFF = 0
CTRL_LED_ON = 1
CTRL_LED_OFF_ALL = 2
CTRL_LED_ON_ALL = 3
CTRL_LED_GET_STATUS = 4

#------------------------------------------------------

# CAMERA control
	# 拍一张图片
CTRL_TYPE_CAMERA_SHUT = 5

	# 开关图像数据流
CTRL_TYPE_CAMERA_STREAM = 6
CAMERA_STREAM_OFF = 0
CAMERA_STREAM_ON = 1

	# 改变分辨率
CTRL_TYPE_CAMERA_CHANGE_SIZE = 7
CAMERA_SIZE_176X144 = 0
CAMERA_SIZE_320X240 = 1
CAMERA_SIZE_352X288 = 2
CAMERA_SIZE_640X480 = 3
CAMERA_SIZE_800X600 = 4
CAMERA_SIZE_1024X768 = 5

	# 白平衡
CTRL_TYPE_CAMERA_WHITE_BALANCE = 8
		# 自动白平衡
CAMERA_WHITE_BALANCE_AUTO = 0x00000000
		# 阳光
CAMERA_WHITE_BALANCE_SUNNY = 0x0054415e
		# 阴天
CAMERA_WHITE_BALANCE_CLOUDY = 0x004f4165
		# 办公室
CAMERA_WHITE_BALANCE_OFFICE = 0x00664152
		# 家庭
CAMERA_WHITE_BALANCE_HOME = 0x00713f42

	# 特殊效果
CTRL_TYPE_CAMERA_EFFECTS = 9
		# 普通
CAMERA_EFFECTS_NORMAL = 0x00808000
		# 黑白
CAMERA_EFFECTS_BLACK_WHITE = 0x00808018
		# 偏蓝
CAMERA_EFFECTS_BLUE = 0x0040a018
		# 偏绿
CAMERA_EFFECTS_GREEN = 0x00404018
		# 偏红
CAMERA_EFFECTS_RED = 0x00c04018
		# 复古
CAMERA_EFFECTS_ANTUQUE = 0x00a64018
		# 反色
CAMERA_EFFECTS_NEGATUVE = 0x00808040
		# 黑白+反色 
CAMERA_COLOR_TYPE_B_W_NEGATIVE = 0x00808058

	# 曝光等级 
CTRL_TYPE_CAMERA_EXPLOSURE = 10
CAMERA_EXPLOSURE_0 = 0x00601820
CAMERA_EXPLOSURE_1 = 0x00701c34
CAMERA_EXPLOSURE_2 = 0x0081383e
CAMERA_EXPLOSURE_3 = 0x00814048
CAMERA_EXPLOSURE_4 = 0x00925058

	# 色彩饱和度 
CTRL_TYPE_CAMERA_SATURATION = 11
CAMERA_SATURATION_HIGHEST = 0x00006868
CAMERA_SATURATION_HIGH = 0x00005858
CAMERA_SATURATION_MEDIUM = 0x00004848
CAMERA_SATURATION_LOW = 0x00003838
CAMERA_SATURATION_LOWEST = 0x00002828

	# 图片亮度 
CTRL_TYPE_CAMERA_LIGHTNESS = 12
CAMERA_LIGHTNESS_HIGHEST = 0x00000040
CAMERA_LIGHTNESS_HIGH = 0x00000030
CAMERA_LIGHTNESS_MEDIUM = 0x00000020
CAMERA_LIGHTNESS_LOW = 0x00000010
CAMERA_LIGHTNESS_LOWEST = 0x00000000

	# 对比度 
CTRL_TYPE_CAMERA_CONTRAST = 13
CAMERA_CONTRAST_HIGNEST = 0x00000c28
CAMERA_CONTRAST_HIGH = 0x00001624
CAMERA_CONTRAST_MEDIUM = 0x00002020
CAMERA_CONTRAST_LOW = 0x00002a1c
CAMERA_CONTRAST_LOWEST = 0x00003418

#------------------------------------------------------

#ack code
ACK_LED = 5

#make the last data package
DATA_END_PACK = -1

class PhyState:
    def __init__(self):
        self.led_status = [0 for i in range(0,8)]
        self.picture_data = b''
        
class PhyPack:
    def __init__(self, buf):
        print('[INFO]make package:\t' + str(buf))
        print('[INFO]data length:\t' + str(len(buf)))
        if type(buf) != bytes:
            print('[ERROR]Wrong data type!')
            return

        self.ID = SERVER_ID
        self.type = 0
        self.length = 0
        self.code = 0
        self.checksum = 0

        if len(buf) < 8 and len(buf) != 0:
            print("[ERROR]Wrong Package haed length!!! length = " + str(len(buf)))
            return

        if len(buf) >= 8:
            self.ID, self.type, self.length, self.code, self.checksum = struct.unpack(PACK_FORMAT, buf[0:8])
        if len(buf) >= 12:
            self.data = buf[8:]
        else:
            self.data = struct.pack('i', 0)

        print('[INFO]package head:\t' + str(self))

    def __str__(self):
        return 'ID: ' + str(self.ID) + ', type: ' + str(self.type) + ', len: ' + str(self.length) + ', code: ' + str(self.code)

class DeviceServer:
    def __init__(self, addr, port):
        self.__backlog = 5
        self.__sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.__sock.bind((addr, port))
        except socket.error as e:
            print('[ERROR]bind error!')
            print(e)
            self.__sock = None

    def listen(self):
        if self.__sock == None:
            print("[ERROR]inavalid socket")
            return
        self.__sock.listen(self.__backlog)

    def accept(self):
        sock, addr = self.__sock.accept();
        return DeviceConn(sock, addr)

class DeviceConn:
    def __init__(self, sock, addr):
        self.__sock = sock
        self.__addr = addr

    def send(self, pack):
        if self.__sock == None:
            return False

        buf = struct.pack(PACK_FORMAT, pack.ID, pack.type, pack.length, pack.code, pack.checksum)
        buf += pack.data
        last_len = len(buf)
        if last_len != pack.length:
            print('[WARNING]Wrong package length')

        try:
            while last_len != 0:
                last_len -= self.__sock.send(buf, last_len)
        except socket.error as e:
            print('[ERROR]connot send the package')
            print(e)
            return False

        print('[INFO]send data:\t' + str(buf))
        return True

    def __checkHead(self, pack):
        return (pack.length >= MIN_PACKAGE_SIZE and pack.length <=
                MAX_PACKAGE_SIZE) and (pack.ID + pack.type + pack.length + pack.code == pack.checksum)

    def __recvHead(self):
        try:
            buf = self.__sock.recv(8)
        except socket.error as e:
            print('[ERROR]cannot recv package head data!!!')
            print(e)
            return None

        print('[INFO]recv head:\t' + str(buf))
        return PhyPack(buf)

    def recv(self):
        if self.__sock == None:
            return None

        buf = b''

        pack = self.__recvHead()
        while self.__checkHead(pack) == False:    #Recieved a wrong head
            print('[WARNING]Wrong head data! Finding new head...')
            pack = self.__recvHead()

        last_len = pack.length - 8
        
        if last_len < 0:
            print('[WARNING]Disconnected!')
            return None
        
        #Recieve data
        pack.data = b''
        temp_buf = b''
        while last_len > 0:
            try:
                temp_buf = self.__sock.recv(last_len)
                pack.data += temp_buf
            except socket.error as e:
                print('[ERROR]cannot recv package data!!!')
                print(e)
                return None
            #print('[INFO]recv data:\t' + str(temp_buf))

            print('[INFO]recv data len: ' + str(len(temp_buf)))
            last_len -= len(temp_buf)

        return pack
     
    def close(self):
        if self.__sock == None:
            return
        self.__sock.close()

    def get_addr(self):
        return self.__addr

def builtFile(packages):
    data = b''
    for pack in packages:
        if pack.type != PACK_TYPE_DATA:
            return b''
        data += pack.data

    return data


def main(argv):
    server = DeviceServer(10086)
    server.listen()
    
    data = b''

    while True:
        conn = server.accept()
        print('connected!')
        pack = conn.recv()
        print('head: ')
        print(pack)
    
        if pack.type == PACK_TYPE_DATA :
            while pack.code != DATA_END_PACK:
                data += pack.data
                pack = conn.recv()
            data += pack.data
        
        print('data: ')
        print(data)
        #conn.close()

if __name__ == '__main__':
    main(sys.argv)

