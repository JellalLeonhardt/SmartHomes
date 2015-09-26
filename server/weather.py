import urllib.parse
import http.client
import time
import gzip
import json
import sys
import re

CITYCODE_WUHAN = 101200101

RANKS = {
    'sun_glasses' : 0,
    'clothing' : 1,
    'travealing' : 2,
    'sports' : 3,
    'car_clean' : 4,
    'dressing' : 5,
    'get_cold' : 6,
    'uv' : 7,
    'comforte' : 8
        }


def getHTML(url):
    header = { 'User-Agent' : 'Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.1) Gecko/20090624 Firefox/3.5',
               'Accept' : 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*',
               'Accept-Encoding' : 'gzip, deflate',
               'Accept-Language' : 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
               'Connection: ' : 'keep-alive' }

    url_info = urllib.parse.urlparse(url)

    conn = http.client.HTTPConnection(url_info.netloc, 80)
    path = url_info.path
    if url_info.query != '':
        path += '?' + url_info.query

    conn.request('GET', path, headers = header)

    try:
        fp = conn.getresponse()
    except http.client.error as e:
        print(e)
        return ""

    print("Status Code: "+str(fp.status));

    data = fp.read()
    if fp.info().get('Content-Encoding') == 'gzip':
        data = gzip.decompress(data)
    html = data.decode("utf-8")

    return html

def getLifeRank(citycode):
    while True:
        html = getHTML('http://www.weather.com.cn/weather1d/' + str(citycode) + '.shtml')
        if html != "":
            break
        print("Connection error! restart connect")
        time.sleep(2)

    partten = re.compile(r'<aside><b>(.*?)</b>(.*?)</aside>')
    ranks = partten.findall(html)
    result = {}
    if len(ranks) < len(RANKS) :
        return {}

    for val in RANKS.keys():
        result[val] = ranks[RANKS[val]][0] + '. ' + ranks[RANKS[val]][1]

    return result;

def getRawWeatherInfo(citycode):
    time_len = 13
    header = { 'User-Agent' : 'Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.1) Gecko/20090624 Firefox/3.5',
               'Referer' : 'http://www.weather.com.cn/weather1d/' + str(citycode) + '.shtml',
               'Accept' : '*/*',
               'Accept-Encoding' : 'gzip, deflate',
               'Accept-Language' : 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
               'Connection: ' : 'keep-alive' }


    conn = http.client.HTTPConnection('d1.weather.com.cn', 80)
    conn.request('GET', '/sk_2d/' + str(citycode) + '.html?_=' + str(time.time() * 1000)[:time_len], headers = header)

    try:
        fp = conn.getresponse()
    except http.client.error as e:
        print(e)
        return {}

    data = fp.read()
    if fp.info().get('Content-Encoding') == 'gzip':
        data = gzip.decompress(data)
    html = data.decode("utf-8")

    print(html)

    info = json.loads(html[html.find('{', 1):])
    return info

def getWeather(citicode):
    raw_info = getRawWeatherInfo(citicode)
    result = {}
    result['Temperature'] = raw_info['temp']
    result['Weather'] = raw_info['weather']
    result['Wind Speed'] = raw_info['WS'] + ": " + raw_info['wse'][4:]
    result['Wind Direction'] = raw_info['WD']
    result['Weat'] = raw_info['SD']
    result['AQI'] = raw_info['aqi']
    return result

def main(argv):
    print("getting ranks")
    ranks = getLifeRank(CITYCODE_WUHAN)
    print("getting weathers")
    info = getWeather(CITYCODE_WUHAN)
    for val in ranks.keys():
        print(val + ': ' + ranks[val])
    for val in info.keys():
        print(val + ': ' + info[val])
    
    
if __name__ == "__main__":
    main(sys.argv)
