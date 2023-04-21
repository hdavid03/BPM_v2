#!/usr/bin/env python3

import serial
import struct
import configparser
from matplotlib import pyplot as plt

CONFIG_FILE = "config.ini"
TIMEOUT = 10


def init_config():
    config = configparser.ConfigParser()
    config.read(CONFIG_FILE)
    return config


def read_port(serial_port):
    byte = b'0'
    floats = []
    while byte != b'XXXX':
        byte = serial_port.read(4)
        print("ok")
        floats.append(struct.unpack('f', byte))
    floats.pop()
    serial_port.close()
    return floats


def get_serial(port, baudrate):
    return serial.Serial(port=port, baudrate=baudrate, 
                        timeout=TIMEOUT, stopbits=1, bytesize=8)


def main():
    config = init_config()
    port = config["device"]["port"]
    baudrate = config["device"]["baudrate"]
    print(f"hello {port}")
    ser = get_serial(port, baudrate)
    floats = read_port(ser)
    plt.plot(floats)
    plt.show()


if __name__ == "__main__":
    main()