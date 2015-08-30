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

def main(argv):
    print('hello world')

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        sock.connect(('25.0.0.109', 10086))
    except socket.error as e:
        print('cannot connect!')
        print (e)
        return -1

    conn = phy_com.DeviceConn(sock, 'localhost')
    pack1 = phy_com.PhyPack(struct.pack('@bbhii', 1, phy_com.PACK_TYPE_DATA, 12, 1, 91))
    pack2 = phy_com.PhyPack(struct.pack('@bbhii', 1, phy_com.PACK_TYPE_DATA, 12, phy_com.DATA_END_PACK, 92))
    conn.send(pack1)
    conn.send(pack2)
    while True:
        pass
    #conn.close()
    pass

if __name__ == '__main__':
    main(sys.argv)
