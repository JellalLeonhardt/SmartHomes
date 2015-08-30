#########################################################################
# File Name: phy_com.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import sys
import socket
import struct

PACK_FORMAT = '@bbhi'

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
        if len(buf) < 8 and len(buf) != 0:
            print("[ERROR]Wrong Package haed length!!! length = " + str(len(buf)))
            return

        self.ID = SERVER_ID
        self.type = 0
        self.length = 0
        self.code = 0
        if len(buf) >= 8:
            self.ID, self.type, self.length, self.code = struct.unpack(PACK_FORMAT, buf[0:8])
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
        self.__ctrl_packs = []
        self.__data_packs = []

    def send(self, pack):
        buf = struct.pack(PACK_FORMAT, pack.ID, pack.type, pack.length, pack.code)
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

    def __recv_head(self):
        try:
            buf = self.__sock.recv(8)
        except socket.error as e:
            print('[ERROR]cannot recv package head data!!!')
            print(e)
            return None

        print('[INFO]recv head:\t' + str(buf))
        return PhyPack(buf)

    def recv(self):
        buf = b''

        pack = self.__recv_head()
        while pack.length < MIN_PACKAGE_SIZE or pack.length > MAX_PACKAGE_SIZE:    #Recieved a wrong head
            print('[WARNING]Wrong head data! Finding new head...')
            pack = self.__recv_head()

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
            print('[INFO]recv data:\t' + str(temp_buf))

            print('[INFO]recv data len: ' + str(len(temp_buf)))
            last_len -= len(temp_buf)

        return pack
     
    def close(self):
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

