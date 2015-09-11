#########################################################################
# File Name: test.py
# Author: HL
# mail: HenryLee6991@gmail.com
#########################################################################
#!/bin/bash

from bottle import route, run, template, request


@route('/hello/<name>')
def index(name):
    result = name
    result = request.query.name
    return template('<b>Hello ,{{me}}</b>!', me = result)

run(host = 'localhost', port = 8080)
