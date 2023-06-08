import os
import sys

print('/******************************/')
print('version    : ' + sys.version)
print('platform   : ' + sys.platform)
print('working dir: ')
os.listdir()
print('/******************************/')

# 可以这样执行自启动脚本
execfile('uart_echo.py')