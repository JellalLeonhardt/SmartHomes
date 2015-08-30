#########################################################################
# File Name: test.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

import _thread, time
from bottle import route, run, template

result = 0

@route('/hello/<name>')
def index(name):
    global result
    result = name
    return template('<b>Hello ,{{me}}</b>!', me = 'ass')

def test_thread(no, argv):
    while True:
        print(result)
        time.sleep(2)

_thread.start_new_thread(test_thread, (15, None))
run(host = 'localhost', port = 8080)
