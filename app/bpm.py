#!/usr/bin/env python3

import serial
import configparser
import array
import peakfilter
import argparse
import struct
import numpy as np
import scipy.signal as sig
from matplotlib import pyplot as plt

BPM_FILE = "bpm.txt"
CONFIG_FILE = "config.ini"
TIMEOUT = 10
BUFFERSIZE = 256
FS = 500

a = [-1.998072905031434, 0.998112346058487]
b = [9.438269707564562e-04, 0, -9.438269707564562e-04]

def init_config():
    config = configparser.ConfigParser()
    config.read(CONFIG_FILE)
    return config


def blood_pressure_monitoring(p, br):
    serial_port = serial.Serial(port=p, baudrate=br, timeout=TIMEOUT,
                                stopbits=1, bytesize=8)
    serial_port.flush()
    fig, ax = create_figure()
    floats = []
    stop = False
    # while not stop:
    while not stop:
        float_bytes = serial_port.read(4)
        float_value = struct.unpack('f', float_bytes)[0]
        print(float_value)
        if float_value > 15000:
            stop = True
        else:
            floats.append(float_value)
        # float_bytes = serial_port.read(BUFFERSIZE)
        # floats += array.array('f', float_bytes).tolist()
    ax.clear()
    ax.set_ylim([0, 200])
    ax.set_xlim([0, 40000])
    ax.plot(floats)
    #plt.pause(0.01)
    serial_port.close()
    return floats


def create_figure():
    fig = plt.figure(figsize=(19.20, 10.80))
    ax = fig.add_subplot(111)
    ax.set_ylim([0, 200])
    ax.set_xlim([0, 40000])
    return fig, ax


def filter_samples(result):
    filt = peakfilter.PeakFilter()
    start_index = 0
    y = []
    pump = True
    wait = False
    for ii in range(0, len(result)):
        sample = result[ii]
        if pump and (sample > 195):
            pump = False
            wait = True
        if wait and (sample < 175):
            wait = False
            start_index = ii
        y.append(filt.filter_sample(sample))
    return start_index, y


def find_min_and_max_values(start_index, y, samples):
    pi = 0
    mi = 0
    peak_found = False
    min_found = False
    peak_positions = []
    peak_values = []
    min_positions = []
    min_values = []
    measured_values = []
    max_v = -2
    min_v = 2
    for ii in range(start_index, len(y)):
        if(y[ii] > max_v):
            max_v = y[ii]
            pi = ii
            m = samples[ii]
            peak_found = True
        elif (y[ii] < max_v - 0.075) and (ii - pi > 280) and peak_found:
            peak_found = False
            peak_positions.append(pi)
            peak_values.append(max_v)
            measured_values.append(m)
            max_v = y[ii]
        if(y[ii] < min_v):
            min_v = y[ii]
            mi = ii
            min_found = True
        elif (y[ii] > min_v) and (min_found) and (ii - mi > 280):
            min_found = False
            min_positions.append(mi)
            min_values.append(min_v)
            min_v = y[ii]
    return {"peak_positions": peak_positions, "peak_values": peak_values,
            "min_positions": min_positions, "min_values": min_values,
            "measured_values": measured_values}

def get_amplitudes(peak_values, min_values, peak_positions, min_positions):
    amplitudes = []
    n_p = len(peak_values) - 2
    n_m = len(min_values) - 2
    k = 0
    p = 0
    if peak_positions[p] < min_positions[k]:
        amplitudes.append(peak_values[p] - min_values[k])
    else:
        amplitudes.append(peak_values[p] - ((min_values[k] + min_values[k+1]) / 2))
        k = k + 1
    p = p + 1
    while (p <= n_p) or (k < n_m):
        amplitudes.append(peak_values[p] - ((min_values[k] + min_values[k + 1]) / 2))
        k = k + 1
        p = p + 1
    return amplitudes


def blood_pressure_measure(result):
    start_index, y = filter_samples(result)
    ret_dict = find_min_and_max_values(start_index, y, result)
    peak_positions = ret_dict["peak_positions"]
    peak_values = ret_dict["peak_values"]
    min_positions = ret_dict["peak_positions"]
    min_values = ret_dict["peak_values"]
    measured_values = ret_dict["peak_values"]
    amplitudes = get_amplitudes(peak_values, min_values, peak_positions, min_positions)
    max_v = amplitudes[0]
    map_index = 0
    for ii in range(1, len(amplitudes)):
        if(amplitudes[ii] > max_v):
            max_v = amplitudes[ii]
            map_index = ii

    SP = max_v * 0.7
    DP = max_v * 0.65
    delta = 1.0
    sp_value = 0
    dp_value = 0
    sp_index = 0
    dp_index = 0
    for ii in range(1, map_index - 1):
        d = abs(y(peak_positions[ii]) - SP)
        if d < delta:
            delta = d
            sp_value = measured_values[ii]
            sp_index = peak_positions[ii]

    delta = 1.0
    for ii in range(0, map_index - 1):
        d = abs(amplitudes[ii] - SP)
        if d < delta:
            delta = d
            sp_value = measured_values[ii]
            sp_index = peak_positions[ii]

    delta = 1.0
    for ii in range(map_index + 1, len(amplitudes)):
        d = abs(amplitudes[ii] - DP)
        if d < delta:
            delta = d
            dp_value = measured_values[ii]
            dp_index = peak_positions[ii]

    pulse = 0
    print(f"len_peak_pos: {len(peak_positions)}, len_amplitudes: {len(amplitudes)}")
    for ii in range(0, len(peak_positions) - 1):
        pulse = pulse + peak_positions[ii + 1] - peak_positions[ii];
    pulse = 1 / (pulse / 10 * 1 / FS) * 60
    return pulse, sp_value, dp_value


def main():
    parser = argparse.ArgumentParser(description="Blood pressure monitoring script")
    parser.add_argument("--offline", action="store_true")
    parser.add_argument("--write_file", action="store_true")
    args = parser.parse_args()

    if args.offline:
        result = []
        with open(BPM_FILE, "r") as file:
            while file.readable():
                try:
                    result.append(float(file.readline()))
                except ValueError:
                    break
    else:
        config = init_config()
        port = config["device"]["port"]
        baudrate = config["device"]["baudrate"]
        print(f"hello {port}")
        result = blood_pressure_monitoring(port, baudrate)

    if args.write_file:
        with open(BPM_FILE, "w") as file:
            for ii in result:
                file.write(f"{ii}\n")

    pulse, sys, dia = blood_pressure_measure(result)
    n = len(result)
    print(f"pulse: {pulse}, sys: {sys}, dia: {dia}")
    result_fft = 1 / n * np.fft.fft(result)
    filt = peakfilter.PeakFilter()
    b = sig.firls(numtaps=1501, bands=[0, 0.1, 0.4, 0.7],
                  desired=[0, 0, 1, 1], fs=480)
    #result_filtered = sig.lfilter(b, 1, result)
    result_filtered = filt.filter_array(result)
    fig = plt.figure(figsize=(19.2, 10.8))
    ax = fig.add_subplot(111)
    ax.plot(result_filtered, label='filtered')
    plt.show()


if __name__ == "__main__":
    main()