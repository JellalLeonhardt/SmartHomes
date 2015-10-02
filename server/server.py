#########################################################################
# File Name: server.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import _thread
import sys
import logging
import logging.handlers
import time

import phy_com
from bottle import route, run, template, request

HOST_ADDR = 'localhost'
PORT_PHY = 10086
PORT_HTTP = 8080

SERVER_ID = 0

TIME_OUT = 5000

#key IP, value: 
dev_list = {}
#key: dev_id, value: data_of_picture
data_dic = { 0: b'' }
data_lock = _thread.allocate_lock()
#key: dev_id, value: (pack_type, pack_code, pack_data)
info_dic = { 0: (0, 0, 0)}
info_lock = _thread.allocate_lock()
#key: dev_id, value: DevConn
conn_dic = { 0: None}
conn_lock = _thread.allocate_lock()

#init global logger
logger = logging.getLogger('server')
logger.setLevel(logging.DEBUG)
fh = logging.handlers.RotatingFileHandler('server.log', 'w', 1024*1024*100, 10)
fh.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.INFO)
formatter = logging.Formatter('[%(asctime)s][%(name)s][%(levelname)s]: %(message)s')
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
        'blackwhite': phy_com.CAMERA_EFFECTS_BLACK_WHITE,
        'blue': phy_com.CAMERA_EFFECTS_BLUE,
        'green': phy_com.CAMERA_EFFECTS_GREEN,
        'red': phy_com.CAMERA_EFFECTS_RED,
        'antique': phy_com.CAMERA_EFFECTS_ANTUQUE,
        'negative': phy_com.CAMERA_EFFECTS_NEGATIVE,
        'bwnegative': phy_com.CAMERA_EFFECTS_B_W_NEGATIVE,
        }

EXPLOSURE = {
        '0': phy_com.CAMERA_EXPLOSURE_0,
        '1': phy_com.CAMERA_EXPLOSURE_1,
        '2': phy_com.CAMERA_EXPLOSURE_2,
        '3': phy_com.CAMERA_EXPLOSURE_3,
        '4': phy_com.CAMERA_EXPLOSURE_4,
        }

SATURATION = {
        '0': phy_com.CAMERA_SATURATION_HIGHEST,
        '1': phy_com.CAMERA_SATURATION_HIGH,
        '2': phy_com.CAMERA_SATURATION_MEDIUM,
        '3': phy_com.CAMERA_SATURATION_LOW,
        '4': phy_com.CAMERA_SATURATION_LOWEST,
        }

LIGHTNESS = {
        '0': phy_com.CAMERA_LIGHTNESS_HIGHEST,
        '1': phy_com.CAMERA_LIGHTNESS_HIGH,
        '2': phy_com.CAMERA_LIGHTNESS_MEDIUM,
        '3': phy_com.CAMERA_LIGHTNESS_LOW,
        '4': phy_com.CAMERA_LIGHTNESS_LOWEST,
        }

CONTRAST = {
        '0': phy_com.CAMERA_CONTRAST_HIGHEST,
        '1': phy_com.CAMERA_CONTRAST_HIGH,
        '2': phy_com.CAMERA_CONTRAST_MEDIUM,
        '3': phy_com.CAMERA_CONTRAST_LOW,
        '4': phy_com.CAMERA_CONTRAST_LOWEST,
        }

@route('/device/list')
def getDevList():
    devs = [] 
    for dev_id in dev_list.values():
        devs.append(str(dev_id))
    r_info = buildResponse(True, '', 'devList', devs)
    return r_info

@route('/led/singleswitch')
def ledSingleSwitch():
    dev_id = int(request.query.ID)
    led_id = int(request.query.LED_ID)
    switch = int(request.query.Switch)
    
    if switch == 0:
        result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_OFF, led_id)
    else:
        result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_ON, led_id)

    return r_info

@route('/led/allswitch')
def ledAllSwitch():
    dev_id = int(request.query.ID)
    switch = int(request.query.Switch)
    
    if switch == 0:
        result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_OFF_ALL, 0)
    else:
        result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_ON_ALL, 0)
    return r_info

@route('/led/status')
def ledStatus():
    dev_id = int(request.query.ID)
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_LED_GET_STATUS, 0)
    
    if not result:
        return r_info
 
    time_out = TIME_OUT
    while not conn_dic[dev_id].data.sem_update.acquire(False):
        time.sleep(0.001)
        time_out -= 1
        if time_out <= 0:
            return buildResponse(False, 'Device Time Out!') 

    led_status = [] 
    for i in range(0, len(conn_dic[dev_id].data.led_status)):
        led_status.append({'id': i, 'status:': conn_dic[dev_id].data.led_status[i]})

    r_info = buildResponse(True, '', 'LEDStatus', led_status)
#    if conn_dic[dev_id].data.update == 0:
#        conn_dic[dev_id].data.update = 1
#    conn_dic[dev_id].data.update -= 1

    return r_info

@route('/camera/picture')
def getPicture():
    dev_id = int(request.query.ID)

    if dev_id in data_dic.keys():
        return data_dic[dev_id]
    else:
        logger.warning("Require ID " + str(dev_id) + ", and data list is " + str(data_dic.keys()))
        return buildResponse(False, 'Device Not Found!') 

@route('/camera/xrandr')
def camera_xrandr():
    dev_id = int(request.query.ID)
    xrandr = request.query.xrandr

    if not xrandr in XRANDR.keys():
        return buildResponse(False, 'Wrong Parameter Value!')

    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_CHANGE_SIZE, XRANDR[xrandr])
    return r_info

@route('/camera/white')
def camera_whitebalance():
    dev_id = int(request.query.ID)
    white = request.query.white
    
    if not white in WHITE_BALANCE.keys():
        return buildResponse(False, 'Wrong Parameter Value!')

    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_WHITE_BALANCE, WHITE_BALANCE[white])
    return r_info

@route('/camera/effects')
def camera_effect():
    dev_id = int(request.query.ID)
    effect = request.query.effect
    
    if not effect in EFFECTS.keys():
        return buildResponse(False, 'Wrong Parameter Value!')

    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_EFFECTS, EFFECTS[effect])
    return r_info

@route('/camera/explosure')
def camera_explosure():
    dev_id = int(request.query.ID)
    exp = request.query.explosure
    
    if not exp in EXPLOSURE.keys():
        return buildResponse(False, 'Wrong Parameter Value!')

    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_EXPLOSURE, EXPLOSURE[exp])
    return r_info

@route('/camera/saturation')
def camera_saturation():
    dev_id = int(request.query.ID)
    sat = request.query.saturation

    if not sat in SATURATION.keys():
        return buildResponse(False, 'Wrong Parameter Value!')
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_SATURATION, SATURATION[sat])
    return r_info

@route('/camera/lightness')
def camera_lightness():
    dev_id = int(request.query.ID)
    lightness = request.query.lightness
    
    if not lightness in LIGHTNESS.keys():
        return buildResponse(False, 'Wrong Parameter Value!')
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_LIGHTNESS, LIGHTNESS[lightness])
    return r_info

@route('/camera/contrast')
def camera_contrast():
    dev_id = int(request.query.ID)
    contrast = request.query.contrast
    
    if not contrast in CONTRAST.keys():
        return buildResponse(False, 'Wrong Parameter Value!')
    
    result, r_info = sendCmd(dev_id, phy_com.CTRL_CAMERA_CONTRAST, CONTRAST[contrast])
    return r_info

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
        logger.warning('Acquiring unavaliable device! Acquiring dev ' + str(dev_id))
        return False, buildResponse(False, 'Device Not Found!')
    
    if not result:
        logger.error('Cmd send Error!')
        return result, buildResponse(False, 'Cannot Send Command To Device!')

    return result, buildResponse(True, '')

def buildResponse(status, msg, ex_data_name = None, ex_data = None):
    response = {}
    response['returnMsg'] = msg
    if status:
        response['returnState'] = 'S'
    else:
        response['returnState'] = 'F'

    if ex_data_name != None:
        response[ex_data_name] = ex_data

    return response

def run_phy_server(addr, ID):
    logger.info('ID: ' + str(ID) + ', Listening at ' + str(addr[0]) + ':' + str(addr[1]))
    server = phy_com.DeviceServer(addr[0], addr[1], ID, logger)
    server.listen()

    ID_counter = ID + 1
    while True:
        conn = server.accept()
        logger.info(str(conn.getAddr()) + ' connected!')
        conn.registHandler(phy_com.PACK_TYPE_DATA, dataPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_ACK, ackPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_REQ, reqPackHandler)
        conn.registHandler(phy_com.PACK_TYPE_DEFAULT, defualtPackHandler)

        ip = conn.getAddr()[0]
        if ip in dev_list.keys():
            dev_id = dev_list[ip]
        else:
            dev_id = ID_counter
        
        if conn.setDevID(ID, dev_id) == False:
            logger.error('Device at ' + str(conn.getAddr()) + ' connot get an ID!')
            del(conn)
            continue
        
        logger.info('ID: ' + str(dev_id))
        if dev_id == ID_counter:
            conn_lock.acquire()
            conn_dic[ID_counter] = conn
            conn_lock.release()
            dev_list[ip] = dev_id
            ID_counter += 1
        
        #Todo...
        #Open picture strem at beginning
        sendCmd(ID_counter - 1, phy_com.CTRL_CAMERA_STREAM, phy_com.CAMERA_STREAM_ON)
        conn.startRecvData()

pic_counter = 0
def dataPackHandler(pack, conn_data):
    global data_dic, data_lock, pic_counter
    conn_data.pic_data += pack.data
    if pack.code == phy_com.DATA_END_PACK:
        data_lock.acquire()
        data_dic[pack.ID] = conn_data.pic_data
        data_lock.release()
        
        logger.debug("Get data: " + str(conn_data.pic_data))
        pic_counter += 1
        logger.info("Get picture: " + str(pic_counter) + " at dev " + str(pack.ID))

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
