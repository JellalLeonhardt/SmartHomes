#########################################################################
# File Name: server.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import _thread
import sys

import phy_com
from bottle import route, run, template

HOST_ADDR = '25.0.0.109'
PORT_PHY = 10086
PORT_HTTP = 8080

#key:dev_id, value:data_of_picture
data_dic = { 0: b'' }
data_lock = _thread.allocate_lock()
#key:dev_id, value:(pack_type, pack_code, pack_data)
cmd_dic = { 0: (0, 0, 0)}
cmd_lock = _thread.allocate_lock()

@route('/picture/<ID>')
def send_pic(ID):
    dev_id = int(ID)
    if dev_id in data_dic.keys():
        #return template('{{pic_data}}', pic_data = str(data_dic[dev_id]))
        return data_dic[dev_id]
    else:
        print('Acquiring unavaliable device!')
        return template('<b>{{info}}</b>', info = 'Device Not Found!')

def conn_thread(conn, addr):
    global data_dic, data_lock
    global cmd_dic, cmd_lock
    data = b''

    print(str(addr) + ' connected!')
    while True:
        pack = conn.recv()
        if pack == None:
            print('[INFO]Device disconnected!')
            continue
            #return

        print('head: ')
        print(pack)
        print('----------------------------------------------------------')

        if pack.type == phy_com.PACK_TYPE_DATA :
            data += pack.data
            print('[INFO]now data: ')
            print(data)
            if pack.code == phy_com.DATA_END_PACK:
                data_lock.acquire()
                data_dic[pack.ID] = data
                data_lock.release()
                print('[INFO]final data: ')
                print(data)
                data = b''
        elif pack.type == phy_com.PACK_TYPE_ACK or pack.type == phy_com.PACK_TYPE_REQ:
            cmd_lock.acquire()
            cmd_dic[pack.ID] = (pack.type, pack.code, pack.data)
            cmd_lock.release()
        else:
            print('[WARNING]Wrong package type! code = ' + str(pack.type))

    conn.close()

def run_phy_server(addr, port):
    print('[INFO]Listening at ' + str(addr) + ':' + str(port))
    server = phy_com.DeviceServer(addr, port)
    server.listen()

    while True:
        conn = server.accept()
        _thread.start_new_thread(conn_thread, (conn, conn.get_addr()))

def main(argv):
    _thread.start_new_thread(run_phy_server, (HOST_ADDR, PORT_PHY))
    run(host = HOST_ADDR, port = PORT_HTTP)

if __name__ == '__main__':
    main(sys.argv)
