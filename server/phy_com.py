#########################################################################
# File Name: phy_com.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import sys
import socket
import struct
import _thread
import threading
import logging

PACK_FORMAT = '@bbhhH'
DATA_FORMAT = '@i'

MAX_PACKAGE_SIZE = 2048
MIN_PACKAGE_SIZE = 12

MAX_WRONG_HAED = MAX_PACKAGE_SIZE / 8

SERVER_ID = 0

#package type code
PACK_TYPE_DEFAULT = -128
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

CTRL_ID_SET = 32767 

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
CAMERA_EFFECTS_NEGATIVE = 0x00808040
		# 黑白+反色 
CAMERA_EFFECTS_B_W_NEGATIVE = 0x00808058

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
CAMERA_CONTRAST_HIGHEST = 0x00000c28
CAMERA_CONTRAST_HIGH = 0x00001624
CAMERA_CONTRAST_MEDIUM = 0x00002020
CAMERA_CONTRAST_LOW = 0x00002a1c
CAMERA_CONTRAST_LOWEST = 0x00003418

#------------------------------------------------------

#ack code
ACK_LED = 5

#make the last data package
DATA_END_PACK = -1

#default error data code
ERR_DEFAULT_CODE = 0xeeeeeeee

class PhyState:
    def __init__(self):
        self.led_status = [0 for i in range(0,8)]
        self.pic_data = b''
        self.sem_update = threading.Semaphore() 
        
class PhyPack:
    def __init__(self, buf = b''):
        #print('[INFO]make package:\t' + str(buf))
        #print('[INFO]data length:\t' + str(len(buf)))
        if type(buf) != bytes:
            #print('[ERROR]Wrong data type!')
            return

        self.ID = 0 
        self.type = 0
        self.length = 0
        self.code = 0
        self.checksum = 0

        buf_len = len(buf)

        if (buf_len < 8 and buf_len != 0) or (buf_len > 8 and buf_len < 12):
            #print("[ERROR]Wrong Package haed length!!! length = " + str(len(buf)))
            return

        if len(buf) >= 8:
            self.ID, self.type, self.length, self.code, self.checksum = struct.unpack(PACK_FORMAT, buf[0:8])
        if len(buf) >= 12:
            self.data = buf[8:]
        else:
            self.data = struct.pack(DATA_FORMAT, 0)

        #print('[INFO]package head:\t' + str(self))

    def __str__(self):
        return 'ID: ' + str(self.ID) + ', type: ' + str(self.type) + ', len: ' + str(self.length) + ', code: ' + str(self.code)

    def calcChecksum(self):
        self.checksum = self.ID + self.type + self.length + self.code

    def setData(self, data):
        if type(data) == int:
            self.data = struct.pack(DATA_FORMAT, data)
        elif type(data) == bytes:
            self.data = data
        else:
            self.data = struct.pack(DATA_FORMAT, ERR_DEFAULT_CODE)

    def getData(self):
        if self.length == MIN_PACKAGE_SIZE:
            return struct.unpack(DATA_FORMAT, self.data)[0]
        else:
            return self.data

class DeviceServer:
    def __init__(self, addr, port, server_ID, logger):
        self.__ID = server_ID
        if server_ID < -128 or server_ID > 127:
            logger.error('Server invalid ID! Using defualt ID: ' + str(SERVER_ID))
            self.__ID = SERVER_ID

        self.__backlog = 5
        self.__sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.logger = logger
        
        try:
            self.__sock.bind((addr, port))
        except socket.error as e:
            self.logger.critical('bind error!')
            self.logger.critical(e)
            self.__sock = None

    def listen(self):
        if self.__sock == None:
            self.logger.critical("inavalid socket")
            return
        self.__sock.listen(self.__backlog)

    def accept(self):
        sock, addr = self.__sock.accept();
        return DeviceConn(sock, addr, self.logger)

    def getSerID(self):
        return self.__ID

class DeviceConn:
    def __init__(self, sock, addr, logger):
        self.__sock = sock
        self.__addr = addr
        self.data = PhyState()
        self.logger = logger
        self.__handler_dic = {}
        self.__thread_stop = True

    def __del__(self):
        if not self.__thread_stop:
            self.stopRecvData()  

    def registHandler(self, pack_type, handle_func):
        self.__handler_dic[pack_type] = handle_func;

    def startRecvData(self):
        if not self.__thread_stop:
            self.logger.warning('try to start recv thread while it has beenrunning')
            return

        self.__thread_stop = False
        self.data.sem_update.acquire()
        _thread.start_new_thread(self.__recvThread, (None, None))

    def stopRecvData(self):
        if self.__thread_stop:
            self.logger.warning('try to stop recv thread while it had beenstoped')
            return

        self.__thread_stop = True
        #wait for thread
        while self.__thread_stop:
            pass

        self.__thread_stop = True

    def __recvThread(self, arg1, arg2):
        arg1 = arg2
        while not self.__thread_stop:
            pack = self.__recv()
            if pack == None:
                self.logger.warning('Device disconnected!')
                self.__thread_stop = True
                return

            #Call the handler if registed
            if pack.type in self.__handler_dic.keys():
                self.__handler_dic[pack.type](pack, self.data)
            elif PACK_TYPE_DEFAULT in self.__handler_dic.keys():
                self.__handler_dic[PACK_TYPE_DEFAULT](pack, self.data)

        #mark the thread had exit
        self.__thread_stop = False


    def send(self, pack):
        if self.__sock == None:
            return False
        
        pack.calcChecksum()
        buf = struct.pack(PACK_FORMAT, pack.ID, pack.type, pack.length, pack.code, pack.checksum)
        buf += pack.data
        last_len = len(buf)
        if last_len != pack.length:
            self.logger.warning('Wrong package length')

        try:
            while last_len != 0:
                last_len -= self.__sock.send(buf, last_len)
        except socket.error as e:
            self.logger.error('connot send the package')
            self.logger.error(e)
            return False

        self.logger.info('send data:\t' + str(buf))
        return True

    def setDevID(self, server_ID, dev_ID):
        if dev_ID < -128 or dev_ID > 127:
            self.logger.error("Trying to set an invalid ID!")
            return False
        
        pack = PhyPack()
        pack.ID = server_ID
        pack.type = PACK_TYPE_CTRL
        pack.length = MIN_PACKAGE_SIZE
        pack.code = CTRL_ID_SET
        pack.setData(dev_ID)

        return self.send(pack)


    def __checkHead(self, pack):
        return (pack.length >= MIN_PACKAGE_SIZE and pack.length <=
                MAX_PACKAGE_SIZE) and (pack.ID + pack.type + pack.length + pack.code == pack.checksum)

    def __recvHead(self):
        try:
            buf = self.__sock.recv(8)
        except socket.error as e:
            self.logger.error('cannot recv package head data!!!')
            self.logger.error(e)
            return None

        self.logger.info('recv head:\t' + str(buf))
        return PhyPack(buf)

    def __recv(self):
        if self.__sock == None:
            return None

        buf = b''
        wrong_head_counter = 0

        pack = self.__recvHead()
        while self.__checkHead(pack) == False:    #Recieved a wrong head
            self.logger.warning('Wrong head data! Finding new head...')
            #Limit numbers of wrong head
            wrong_head_counter += 1
            if wrong_head_counter >= MAX_WRONG_HAED:
                self.logger.warning('Too many wrong head! Maybe disconnected!')
                return None
            pack = self.__recvHead()

        last_len = pack.length - 8
        
        if last_len < 0:
            self.logger.warning('Disconnected!')
            return None
        
        #Recieve data
        pack.data = b''
        temp_buf = b''
        while last_len > 0:
            try:
                temp_buf = self.__sock.recv(last_len)
                pack.data += temp_buf
            except socket.error as e:
                self.logger.error('cannot recv package data!!!')
                self.logger.error(e)
                return None

            self.logger.info('recv data len: ' + str(len(temp_buf)))
            last_len -= len(temp_buf)

        return pack
     
    def close(self):
        if self.__sock == None:
            return
        self.__sock.close()

    def getAddr(self):
        return self.__addr

def builtFile(packages):
    data = b''
    for pack in packages:
        if pack.type != PACK_TYPE_DATA:
            return b''
        data += pack.data

    return data


def main(argv):
    logger = logging.getLogger('phy_com')
    logger.setLevel(logging.DEBUG)
    fh = logging.FileHandler('phy_com.log')
    fh.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    formatter = logging.Formatter('[%(name)s][%(levelname)s]: %(message)s')
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)
    logger.addHandler(fh)
    logger.addHandler(ch)

    server = DeviceServer('localhost', 10086, 88, logger)
    server.listen()
    
    data = b''

    while True:
        conn = server.accept()
        print('connected!')
        
        conn.registHandler(PACK_TYPE_DATA, dataPackHandler)
        conn.setDevID(server.getSerID(), 0x2b)
        conn.startRecvData()
        
        #conn.close()

def dataPackHandler(pack, conn_data):
    conn_data.pic_data += pack.data
    if pack.code == DATA_END_PACK:
        print("data: ")
        print(conn_data.pic_data)
        conn_data.pic_data = b''

if __name__ == '__main__':
    main(sys.argv)

