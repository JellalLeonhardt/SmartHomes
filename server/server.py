#########################################################################
# File Name: server.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import _thread
import sys
import logging
import time

import phy_com
from bottle import route, run, template, request

HOST_ADDR = 'localhost'
PORT_PHY = 10086
PORT_HTTP = 8080

SERVER_ID = 0

TIME_OUT = 5000

#key:dev_id, value:data_of_picture
data_dic = { 0: b'' }
data_lock = _thread.allocate_lock()
#key:dev_id, value:(pack_type, pack_code, pack_data)
info_dic = { 0: (0, 0, 0)}
info_lock = _thread.allocate_lock()
#key:dev_id, value: DevConn
conn_dic = { 0: None}
conn_lock = _thread.allocate_lock()

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

#const val table
XRANDR = {
        '176x144': phy_com.CAMERA_SIZE_176X144,
        '320x240': phy_com.CAMERA_SIZE_320X240,
        '352x288': phy_com.CAMERA_SIZE_352X288,
        '640x480': phy_com.CAMERA_SIZE_640X480,
        '800x600': phy_com.CAMERA_SIZE_800X600,
        '1024x768': phy_com.CAMERA_SIZE_1024X768
        }
WHITE_BALANCE = {
        'auto': phy_com.CAMERA_WHITE_BALANCE_AUTO,
        'sunny': phy_com.CAMERA_WHITE_BALANCE_SUNNY,
        'cloudy': phy_com.CAMERA_WHITE_BALANCE_CLOUDY,
        'office': phy_com.CAMERA_WHITE_BALANCE_OFFICE,
        'home': phy_com.CAMERA_WHITE_BALANCE_HOME,
        }

EFFECTS = {
        'normal': phy_com.CAMERA_EFFECTS_NORMAL,
        'black_white': phy_com.CAMERA_EFFECTS_BLACK_WHITE,
        'blue': phy_com.CAMERA_EFFECTS_BLUE,
        'green': phy_com.CAMERA_EFFECTS_GREEN,
        'red': phy_com.CAMERA_EFFECTS_RED,
        'antuque': phy_com.CAMERA_EFFECTS_ANTUQUE,
        'negative': phy_com.CAMERA_EFFECTS_NEGATIVE,
        'b_w_negative': phy_com.CAMERA_EFFECTS_B_W_NEGATIVE,
        }

EXPLOSURE = {
        '0': phy_com.CAMERA_EXPLOSURE_0,
        '1': phy_com.CAMERA_EXPLOSURE_1,
        '2': phy_com.CAMERA_EXPLOSURE_2,
        '3': phy_com.CAMERA_EXPLOSURE_3,
        '4': phy_com.CAMERA_EXPLOSURE_4,
        }

SATURATION = {
        'highest': phy_com.CAMERA_SATURATION_HIGHEST,
        'high': phy_com.CAMERA_SATURATION_HIGH,
        'medium': phy_com.CAMERA_SATURATION_MEDIUM,
        'low': phy_com.CAMERA_SATURATION_LOW,
        'lowest': phy_com.CAMERA_SATURATION_LOWEST,
        }

LIGHTNESS = {
        'highest': phy_com.CAMERA_LIGHTNESS_HIGHEST,
        'high': phy_com.CAMERA_LIGHTNESS_HIGH,
        'medium': phy_com.CAMERA_LIGHTNESS_MEDIUM,
        'low': phy_com.CAMERA_LIGHTNESS_LOW,
        'lowest': phy_com.CAMERA_LIGHTNESS_LOWEST,
        }

CONTRAST = {
        'highest': phy_com.CAMERA_CONTRAST_HIGHEST,
        'high': phy_com.CAMERA_CONTRAST_HIGH,
        'medium': phy_com.CAMERA_CONTRAST_MEDIUM,
        'low': phy_com.CAMERA_CONTRAST_LOW,
        'lowest': phy_com.CAMERA_CONTRAST_LOWEST,
        }

@route('/picture')
def send_pic():
    dev_id = int(request.query.ID)
    if dev_id in data_dic.keys():
        return data_dic[dev_id]
    else:
        return template('<b>{{info}}</b>', info = 'Device Not Found!')

@route('/led/on')
def led_on():
    dev_id = int(request.query.ID)
    led_id = int(request.query.led_id)
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_ON, led_id)
    return template('<b>{{info}}</b>', info = r_info)

@route('/led/off')
def led_off():
    dev_id = int(request.query.ID)
    led_id = int(request.query.led_id)
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_OFF, led_id)
    return template('<b>{{info}}</b>', info = r_info)

@route('/led/all_on')
def led_all_on():
    dev_id = int(request.query.ID)
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_ON_ALL, 0)
    return template('<b>{{info}}</b>', info = r_info)

@route('/led/all_off')
def led_all_off():
    dev_id = int(request.query.ID)

    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_OFF_ALL, 0)
    return template('<b>{{info}}</b>', info = r_info)

@route('/led/state')
def led_get_state():
    dev_id = int(request.query.ID)
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_GET_STATUS, 0)
    
    if not result:
        return template('<b>{{info}}</b>', info = r_info)

    conn_dic[dev_id].data.sem_update.acquire()
#    time_out = TIME_OUT
#    while conn_dic[dev_id].data.update == 0:
#        time.sleep(0.001)
#        time_out -= 1
#        if time_out >= 0:
#            return template('<b>{{info}}</b>', info = "Device Time Out!")

    r_info = str(conn_dic[dev_id].data.led_status)
    if conn_dic[dev_id].data.update == 0:
        conn_dic[dev_id].data.update = 1
    conn_dic[dev_id].data.update -= 1

    return template('<b>{{info}}</b>', info = r_info)

@route('/camera/xrandr')
def camera_xrandr():
    dev_id = int(request.query.ID)
    xrandr = request.query.xrandr

    if not xrandr in XRANDR.keys():
        pass

    result, r_info = sendCmd(dev_id, phy_com.CTRL_TYPE_CAMERA_CHANGE_SIZE, XRANDR[xrandr])
    return template('<b>{{info}}</b>', info = r_info)

@route('/camera/white')
def camera_whitebalance():
    dev_id = int(request.query.ID)
    white = request.query.white
    
    if not white in WHITE_BALANCE.keys():
        pass

    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_WHITE_BALANCE, WHITE_BALANCE[white])
    return template('<b>{{info}}</b>', info = r_info)

@route('/camera/effect')
def camera_effect():
    dev_id = int(request.query.ID)
    effect = request.query.effect
    
    if not effect in EFFECTS.keys():
        pass

    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_EFFECTS, EFFECTS[effect])
    return template('<b>{{info}}</b>', info = r_info)

@route('/camera/explosure')
def camera_explosure():
    dev_id = int(request.query.ID)
    exp = request.query.exp
    
    if not exp in EXPLOSURE.keys():
        pass

    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_EXPLOSURE, EXPLOSURE[exp])
    return template('<b>{{info}}</b>', info = r_info)

@route('/camera/saturation')
def camera_saturation(ID, sat):
    dev_id = int(request.query.ID)
    sat = request.query.sat
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_SATURATION, SATURATION[sat])
    return template('<b>{{info}}</b>', info = r_info)

@route('/camera/light')
def camera_lightness(ID, lightness):
    dev_id = int(request.query.ID)
    lightness = request.query.lightness
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_ON_ALL, 0)
    return template('<b>{{info}}</b>', info = r_info)

@route('/camera/contrast')
def camera_contrast(ID, contrast):
    dev_id = int(request.query.ID)
    contrast = request.query.contrast
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_ON_ALL, 0)
    return template('<b>{{info}}</b>', info = r_info)

def sendCmd(dev_id, cmd_code, cmd_data):
    pack = phy_com.PhyPack()
    pack.ID = SERVER_ID
    pack.type = phy_com.PACK_TYPE_CTRL
    pack.length = phy_com.MIN_PACKAGE_SIZE
    pack.code = cmd_code
    pack.setData(cmd_data)
    
    if dev_id in conn_dic.keys():
        result = conn_dic[dev_id].send(pack)
    else:
        result = False
        logger.warning('Acquiring unavaliable device! Acquiring dev ' + str(dev_id))
        return result, "Device Not Found!"
    
    if not result:
        logger.error('Cmd send Error!')
        return result, "Cannot send the command!"
    else:
        return result, "Sended the command!"

def run_phy_server(addr, ID):
    logger.info('Listening at ' + str(addr[0]) + ':' + str(addr[1]))
    server = phy_com.DeviceServer(addr[0], addr[1], ID, logger)
    server.listen()

    ID_counter = SERVER_ID + 1
    while True:
        conn = server.accept()
        logger.info(str(conn.getAddr()) + ' connected!')
        conn.registHandler(phy_com.PACK_TYPE_DATA, dataPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_ACK, ackPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_REQ, reqPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_DEFAULT, defualtPackHandler)

        if conn.setDevID(ID, ID_counter) == False:
            logger.error('Device at ' + str(conn.getAddr()) + ' connot get an ID!')
            del(conn)
            continue

        conn_lock.acquire()
        conn_dic[ID_counter] = conn
        conn_lock.release()
        logger.info('ID: ' + str(ID_counter))
        ID_counter += 1
        
        #Todo...
        #Open picture strem at beginning
        sendCmd(ID_counter - 1, phy_com.CTRL_CAMERA_STREAM, phy_com.CAMERA_STREAM_ON)
        conn.startRecvData()

def dataPackHandler(pack, conn_data):
    global data_dic, data_lock
    conn_data.pic_data += pack.data
    if pack.code == phy_com.DATA_END_PACK:
        data_lock.acquire()
        data_dic[pack.ID] = conn_data.pic_data
        data_lock.release()
        
        logger.debug("Get data: " + str(conn_data.pic_data))

        conn_data.pic_data = b''

def ackPackHandler(pack, conn_data):
    if pack.code == phy_com.ACK_LED:
        ack = pack.getData()
        for i in range(8):
            if (ack & (0x01 << i)) > 0:
                conn_data.led_status[i] = 1
#        conn_data.update += 1
        conn_data.sem_update.release()
    else:
        logger.warning('Wrong ack code! code = ' + str(pack.code))

def reqPackHandler(pack, conn_data):
    pass

def defualtPackHandler(pack, conn_data):
    logger.warning('Wrong package type! type = ' + str(pack.type))

def main(argv):
    if len(argv) == 1:
        host_addr = HOST_ADDR
        port_http = PORT_HTTP
        port_phy = PORT_PHY
        server_id = SERVER_ID
    elif len(argv) == 5:
        host_addr = argv[1]
        port_http = int(argv[2])
        port_phy = int(argv[3])
        server_id = int(argv[4])

        global SERVER_ID
        SERVER_ID = server_id
    else:
        print("Usage: python server.py host_addr http_port phy_port server_ID")
        return

    _thread.start_new_thread(run_phy_server, ((host_addr, port_phy), server_id))
    run(host = host_addr, port = port_http)

if __name__ == '__main__':
    main(sys.argv)
