import select
import time
import rtthread
from machine import UART

def main():
    print("Welcome to RT-Thread MicroPython uart echo test!")
    uart = UART(7, 115200)
    uart.init(baudrate=115200, bits=8, parity=None, stop=1)

    while(1) :
        rcv = uart.read(256)
        if len(rcv) > 0:
            print('(%d) %s' % (len(rcv), rcv))
            uart.write(rcv)
    
if __name__ == '__main__':
    main()