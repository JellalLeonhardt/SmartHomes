#########################################################################
# File Name: server.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import _thread
import sys
import logging

import phy_com
from bottle import route, run, template, request

HOST_ADDR = '25.0.0.109'
PORT_PHY = 10086
PORT_HTTP = 8080

#key:dev_id, value:data_of_picture
data_dic = { 0: b'' }
data_lock = _thread.allocate_lock()
#key:dev_id, value:(pack_type, pack_code, pack_data)
info_dic = { 0: (0, 0, 0)}
info_lock = _thread.allocate_lock()

#init global logger
logger = logging.getLogger('server')
logger.setLevel(logging.DEBUG)
fh = logging.FileHandler('server.log')
fh.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter('[%(name)s][%(levelname)s]: %(message)s')
fh.setFormatter(formatter)
ch.setFormatter(formatter)
logger.addHandler(fh)
logger.addHandler(ch)


@route('/picture')
def send_pic():
    dev_id = int(request.query.ID)
    if dev_id in data_dic.keys():
        return data_dic[dev_id]
    else:
        print('Acquiring unavaliable device!')
        return template('<b>{{info}}</b>', info = 'Device Not Found!')

@route('/led/on')
def led_on():
    dev_id = int(request.query.ID)
    led_id = int(request.query.led_id)
    pass

@route('/led/off')
def led_off():
    pdev_id = int(request.query.ID)
    led_id = int(request.query.led_id)
    pass

@route('/led/all_on')
def led_all_on():
    dev_id = int(request.query.ID)
    pass

@route('/led/all_off')
def led_all_off():
    dev_id = int(request.query.ID)
    pass

@route('/led/state')
def led_get_state():
    dev_id = int(request.query.ID)
    pass

@route('/camera/xrandr')
def camera_xrandr():
    dev_id = int(request.query.ID)
    xrandr = request.query.xrandr
    pass

@route('/camera/white')
def camera_whitebalance():
    dev_id = int(request.query.ID)
    white = request.query.white
    pass

@route('/camera/effect')
def camera_effect():
    dev_id = int(request.query.ID)
    effect = request.query.effect
    pass

@route('/camera/explosure')
def camera_explosure():
    dev_id = int(request.query.ID)
    exp = request.query.exp
    pass

@route('/camera/saturation')
def camera_saturation(ID, sat):
    dev_id = int(request.query.ID)
    sat = request.query.sat
    pass

@route('/camera/light')
def camera_lightness(ID, lightness):
    dev_id = int(request.query.ID)
    lightness = request.query.lightness
    pass

@route('/camera/contrast')
def camera_contrast(ID, contrast):
    pdev_id = int(request.query.ID)
    contrast = request.query.contrast
    pass

def conn_thread(conn, addr):
    global data_dic, data_lock
    global info_dic, info_lock
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
            #print('[INFO]now data: ')
            #print(data)
            if pack.code == phy_com.DATA_END_PACK:
                data_lock.acquire()
                data_dic[pack.ID] = data
                data_lock.release()
                #print('[INFO]final data: ')
                #print(data)
                data = b''
        elif pack.type == phy_com.PACK_TYPE_ACK or pack.type == phy_com.PACK_TYPE_REQ:
            info_lock.acquire()
            info_dic[pack.ID] = (pack.type, pack.code, pack.data)
            info_lock.release()
        else:
            print('[WARNING]Wrong package type! code = ' + str(pack.type))

    conn.close()

def run_phy_server(addr, port):
    print('[INFO]Listening at ' + str(addr) + ':' + str(port))
    server = phy_com.DeviceServer(addr, port, logger)
    server.listen()

    while True:
        conn = server.accept()
        conn.registHandler(phy_com.PACK_TYPE_DATA, dataPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_ACK, ackPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_REQ, reqPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_DEFAULT, defualtPackHandler)
        conn.startRecvData()

def dataPackHandler(pack):
    pass

def ackPackHandler(pack):
    pass

def reqPackHandler(pack):
    pass

def defualtPackHandler(pack):
    pass

def main(argv):
    _thread.start_new_thread(run_phy_server, (HOST_ADDR, PORT_PHY))
    run(host = HOST_ADDR, port = PORT_HTTP)

if __name__ == '__main__':
    main(sys.argv)
