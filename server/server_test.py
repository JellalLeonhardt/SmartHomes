import urllib.request
import json
import time
import sys
import base64

host_addr = "http://25.0.0.150:8080/"
dev_id = 1

def urlread(url):
    fp = urllib.request.urlopen(url)
    data = fp.read()
    return data

def devTest():
    print("Device Test Start:")
    print('\tList Test')
    data = urlread(host_addr + "device/list").decode()
    print(data)
    print('\tSetting Test')
    data = urlread(host_addr + 'device/setting?ID=' + str(dev_id)).decode()
    print(data)
    print("Device Test End")

def ledTest():
    print("LED Test Start")

    print('\tMain Switch Test')
    url = host_addr + 'led/allswitch?ID=' + str(dev_id) + '&'
    print('\t\tAll LEDs On')
    data = urlread(url + "Switch=1").decode()
    print(data)
    ledStatusTest()
    time.sleep(2)

    print('\t\tAll LEDs Off')
    data = urlread(url + "Switch=0").decode()
    print(data)
    ledStatusTest()
    time.sleep(2)


    print('\tSingle Switch Test')
    url = host_addr + 'led/singleswitch?ID=' + str(dev_id) + '&'
    print('\t\tLED2 On')
    data = urlread(url + "LED_ID=2&Switch=1").decode()
    print(data)
    ledStatusTest()
    time.sleep(2)

    print('\t\tLED3 On')
    data = urlread(url + "LED_ID=3&Switch=1").decode()
    print(data)
    ledStatusTest()
    time.sleep(2)

    print('\t\tLED2 Off')
    data = urlread(url + "LED_ID=2&Switch=0").decode()
    print(data)
    ledStatusTest()

    print('LED Test End')
    
def ledStatusTest():
    print('\t\tLED Status:')
    data = urlread(host_addr + 'led/status?ID=' + str(dev_id)).decode()
    print(data)

def cameraTest():
    print('Camera Test Start')

    print('\tXrandr Test')
    url = host_addr + 'camera/xrandr?ID=' + str(dev_id) + '&xrandr='
    print('\t\tSet Xrandr to 320x240')
    data = urlread(url + '320x240').decode()
    print(data)
    cameraPictureTest('xrandr320x240')

    print('\t\tSet Xrandr to 1024x768')
    data = urlread(url + '1024x768').decode()
    print(data)
    cameraPictureTest('xrandr1024x768')

    print('\t\tSet Xrandr to 640x480')
    data = urlread(url + '640x480').decode()
    print(data)
    cameraPictureTest('xrandr640x480')


    print('\tWhite balance Test')
    url = host_addr + 'camera/white?ID=' + str(dev_id) + '&white='
    print('\t\tSet White Balance to sunny')
    data = urlread(url + 'sunny').decode()
    print(data)
    cameraPictureTest('white_sunny')

    print('\t\tSet White Balance to cloudy')
    data = urlread(url + 'cloudy').decode()
    print(data)
    cameraPictureTest('white_cloudy')

    print('\t\tSet White Balance to auto')
    data = urlread(url + 'auto').decode()
    print(data)
    cameraPictureTest('white_auto')


    print('\tEffect Test')
    url = host_addr + 'camera/effects?ID=' + str(dev_id) + '&effect='
    print('\t\tSet Effect to B&W')
    data = urlread(url + 'blackwhite').decode()
    print(data)
    cameraPictureTest('effect_B_W')

    print('\t\tSet Effect to antique')
    data = urlread(url + 'antique').decode()
    print(data)
    cameraPictureTest('effect_antique')

    print('\t\tSet Effect to normal')
    data = urlread(url + 'normal').decode()
    print(data)
    cameraPictureTest('effect_normal')


    print('\tExplosure Test')
    url = host_addr + 'camera/explosure?ID=' + str(dev_id) + '&explosure='
    print('\t\tSet Explosure to 4')
    data = urlread(url + '4').decode()
    print(data)
    cameraPictureTest('explosure_4')

    print('\t\tSet Explosure to 0')
    data = urlread(url + '0').decode()
    print(data)
    cameraPictureTest('explosure_0')

    print('\t\tSet Explosure to 1')
    data = urlread(url + '1').decode()
    print(data)
    cameraPictureTest('explosure_1')

    print('\t\tSet Explosure to 2')
    data = urlread(url + '2').decode()
    print(data)
    cameraPictureTest('explosure_2')

    print('\t\tSet Explosure to 3')
    data = urlread(url + '3').decode()
    print(data)
    cameraPictureTest('explosure_3')

    print('\tLightness Test')
    url = host_addr + 'camera/lightness?ID=' + str(dev_id) + '&lightness='
    print('\t\tSet Lightness to 4')
    data = urlread(url + '4').decode()
    print(data)
    cameraPictureTest('lightness_4')

    print('\t\tSet Lightness to 2')
    data = urlread(url + '2').decode()
    print(data)
    cameraPictureTest('lightness_2')

    print('\t\tSet Lightness to 2')
    data = urlread(url + '2').decode()
    print(data)
    cameraPictureTest('lightness_2')


    print('\tContrast Test')
    url = host_addr + 'camera/contrast?ID=' + str(dev_id) + '&contrast='
    print('\t\tSet Contrast to 4')
    data = urlread(url + '4').decode()
    print(data)
    cameraPictureTest('contrast_4')

    print('\t\tSet Contrast to 0')
    data = urlread(url + '0').decode()
    print(data)
    cameraPictureTest('contrast_0')

    print('\t\tSet Contrast to 2')
    data = urlread(url + '2').decode()
    print(data)
    cameraPictureTest('contrast_2')


    print('\tSaturation Test')
    url = host_addr + 'camera/saturation?ID=' + str(dev_id) + '&saturation='
    print('\t\tSet Saturation to 4')
    data = urlread(url + '4').decode()
    print(data)
    cameraPictureTest('saturation_4')

    print('\t\tSet Saturation to 0')
    data = urlread(url + '0').decode()
    print(data)
    cameraPictureTest('saturation_0')

    print('\t\tSet Saturation to 2')
    data = urlread(url + '2').decode()
    print(data)
    cameraPictureTest('saturation_2')

    print('Camera Test End')

def cameraPictureTest(pic_name):
    time.sleep(20)
    json_data = urlread(host_addr + 'camera/picture?ID=' + str(dev_id))
    filename = './pictest_' + pic_name + '.jpg'
    data = json.loads(json_data.decode())
    print(data['picture'])
    fp = open(filename, 'wb')
    fp.write(base64.b64decode(data))
    fp.close()
    print('\t\tPicture: ' + pic_name + ' had been written to ' + filename)

def main(argv):
    if len(argv) == 4:
        global host_addr, dev_id
        host_addr = 'http://' + argv[1] + ':' + argv[2] + '/'
        dev_id = int(argv[3])
    elif len(argv) != 1:
        print('usage: python serverTest.py [host_name] [port] [dev_id]')
        return

    print('Test addr: ' + host_addr)
    print('Test dev_id: ' + str(dev_id))
    print("--------Test Start--------")
    devTest()
    ledTest()
    cameraTest()
    print("--------Test End--------")
    
if __name__ == '__main__':
    main(sys.argv)
