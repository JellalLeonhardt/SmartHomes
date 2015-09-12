#########################################################################
# File Name: client.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import sys
import phy_com
import socket
import struct
import logging

ID = 0

def main(argv):
    print('hello world')
    logger = logging.getLogger('client')
    logger.setLevel(logging.DEBUG)
    fh = logging.FileHandler('client.log')
    fh.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    formatter = logging.Formatter('[%(name)s][%(levelname)s]: %(message)s')
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)
    logger.addHandler(fh)
    logger.addHandler(ch)


    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        sock.connect(('localhost', 10086))
    except socket.error as e:
        print('cannot connect!')
        print(e)
        return -1

    conn = phy_com.DeviceConn(sock, 'localhost', logger)
    conn.registHandler(phy_com.PACK_TYPE_CTRL, ctrlHandler)
    conn.startRecvData()

    while ID == 0:
        pass

    print("ID: ", str(ID))
    pack1 = phy_com.PhyPack(struct.pack(phy_com.PACK_FORMAT + 'i', ID,
        phy_com.PACK_TYPE_DATA, 12, 1, 1 + 12 + 1 + phy_com.PACK_TYPE_DATA, 91))
    pack2 = phy_com.PhyPack(struct.pack(phy_com.PACK_FORMAT + 'i', ID,
        phy_com.PACK_TYPE_DATA, 12, phy_com.DATA_END_PACK, 1 + 12 - 1 + phy_com.PACK_TYPE_DATA, 92))
    conn.send(pack1)
    conn.send(pack2)
    while True:
        pass
    #conn.close()
    pass

def ctrlHandler(pack, conn_data):
    global ID
    ID = struct.unpack(phy_com.DATA_FORMAT, pack.data)[0]

if __name__ == '__main__':
    main(sys.argv)
