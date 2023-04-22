#!/usr/bin/env python3

import serial
import configparser
import array
import numpy as np
import peakfilter
from matplotlib import pyplot as plt

CONFIG_FILE = "config.ini"
TIMEOUT = 10
BUFFERSIZE = 256
FS = 1440

a = [-1.998072905031434, 0.998112346058487]
b = [9.438269707564562e-04, 0, -9.438269707564562e-04]

def init_config():
    config = configparser.ConfigParser()
    config.read(CONFIG_FILE)
    return config


def blood_pressure_monitoring(port, baudrate):
    serial_port = serial.Serial(port=port, baudrate=baudrate,
                            timeout=TIMEOUT, stopbits=1, bytesize=8)
    fig, ax = create_figure()
    floats = []
    stop = False
    while not stop:
        float_bytes = serial_port.read(BUFFERSIZE)
        floats += array.array('f', float_bytes).tolist()
        if floats[-1] > 5000:
            stop = True
            floats.pop()
        ax.clear()
        ax.set_ylim([0, 220])
        ax.plot(floats)
        plt.pause(0.01)
    serial_port.close()
    return floats


def create_figure():
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_ylim([0, 220])
    ax.set_xlim([0, 100000])
    return fig, ax


def blood_pressure_measure(result):
    filt = peakfilter.PeakFilter()
    index_peak = 0
    index_min = 0
    pi = 0
    mi = 0
    y = []
    input_value = []

    start = False
    peak_positions = []
    min_positions = []
    peak_values = []
    min_values = []
    amplitudes = []
    measured_values = []
    max_v = -2
    min_v = 2
    pump = True
    wait = False
    peak_found = False
    min_found = False
    m = 0
    for i in range(0, len(result)):
        sample = result[i]
        if pump and (sample > 195):
            pump = False
            wait = True
        if wait and (sample < 175):
            wait = False
            start = True
        y.append(filt.filter_sample(sample))
        if start:
            if(y[i] > max_v):
                max_v = y[i]
                pi = i
                m = sample
                peak_found = True
            elif (y[i] < max_v-0.075) and (i-pi > 280) and peak_found:
                peak_found = False
                peak_positions += pi
                peak_values += max_v
                measured_values += m
                index_peak = index_peak + 1
                max_v = y[i]
            if(y[i] < min_v):
                min_v = y[i]
                mi = i
                min_found = True
            elif (y[i] > min_v) and (min_found) and (i-mi > 280):
                min_found = False
                min_positions += mi;
                min_values += min_v;
                index_min = index_min + 1;
                min_v = y[i];
        input_value[i] = sample
    N = i - 1
    np = index_peak - 1
    nm = index_min - 1
    k = 0
    p = 0
    if peak_positions[p] < min_positions[k]:
        amplitudes[p] = peak_values[p] - min_values[k];
    else:
        amplitudes[p] = peak_values[p] - ((min_values[k] + min_values[k+1]) / 2);
        k = k + 1

    p = p + 1
    while (p <= np) or (k < nm):
        amplitudes[p] = peak_values[p] - ((min_values[k] + min_values[k+1]) / 2);
        k = k + 1
        p = p + 1

    max_v = amplitudes[0]
    map_index = 0
    for i in range(1, np):
        if(amplitudes[i] > max_v):
            max_v = amplitudes[i]
            map_index = i

    SP = max_v * 0.7
    DP = max_v * 0.65
    delta = 1.0
    sp_value = 0
    dp_value = 0
    sp_index = 0
    dp_index = 0
    for i in range(1, map_index-1):
        d = abs(y(peak_positions[i]) - SP)
        if d < delta:
            delta = d
            sp_value = measured_values[i]
            sp_index = peak_positions[i]

    delta = 1.0
    for i in range(0, map_index - 1):
        d = abs(amplitudes[i] - SP)
        if d < delta:
            delta = d
            sp_value = measured_values[i]
            sp_index = peak_positions[i]

    delta = 1.0
    for i in range(map_index + 1, np):
        d = abs(amplitudes[i] - DP)
        if d < delta:
            delta = d
            dp_value = measured_values[i]
            dp_index = peak_positions[i]

    pulse = 0
    for i in range(0, 9):
        pulse = pulse + peak_positions[i+1] - peak_positions[i];
    pulse = 1/(pulse/10 * 1/FS) * 60
    return pulse, sp_value, dp_value

def main():
    config = init_config()
    port = config["device"]["port"]
    baudrate = config["device"]["baudrate"]
    print(f"hello {port}")
    result = blood_pressure_monitoring(port, baudrate)
    pulse, sys, dia = blood_pressure_measure(result)
    print(f"pulse: {pulse}, sys: {sys}, dia: {dia}")


if __name__ == "__main__":
    main()