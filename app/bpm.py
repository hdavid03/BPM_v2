#!/usr/bin/env python3

import serial
import configparser
import array
import peakfilter
import argparse
import numpy as np
from matplotlib import pyplot as plt

BPM_FILE = "bpm.txt"
CONFIG_FILE = "config.ini"
TIMEOUT = 10
BUFFERSIZE = 384
OFFSET = 29.91
FS = 960
FONT_TITLE = {'family': 'serif',
        'color':  'black',
        'weight': 'bold',
        'size': 18,
        }
FONT_LABEL = {'family': 'serif',
        'color':  'black',
        'weight': 'normal',
        'size': 16,
        }


def init_config():
    config = configparser.ConfigParser()
    config.read(CONFIG_FILE)
    return config


def blood_pressure_monitoring(p, br):
    serial_port = serial.Serial(port=p, baudrate=br,
                            timeout=TIMEOUT, stopbits=1, bytesize=8)
    fig, ax = create_figure()
    floats = []
    bpm_res = []
    stop = False
    while not stop:
        float_bytes = serial_port.read(BUFFERSIZE)
        floats += array.array('f', float_bytes).tolist()
        if floats[-1] > 55000:
            stop = True
            floats.pop()
            bpm_res.append(floats.pop())
            bpm_res.append(floats.pop())
            bpm_res.append(floats.pop())
        ax.clear()
        # ax.set_ylim([0, 200])
        # ax.set_xlim([0, 104000])
        ax.plot(floats)
        plt.pause(0.01)
    serial_port.close()
    return floats, bpm_res


def create_figure():
    fig = plt.figure(figsize=(19.20, 10.80))
    ax = fig.add_subplot(111)
    # ax.set_ylim([0, 200])
    # ax.set_xlim([0, 104000])
    return fig, ax


def filter_samples(result):
    filt = peakfilter.PeakFilter()
    start_index = 0
    y = []
    pump = True
    wait = False
    for ii in range(0, len(result)):
        sample = result[ii]
        if pump and (sample > 190):
            pump = False
            wait = True
        if wait and (sample < 190):
            wait = False
            start_index = ii
        y.append(filt.filter_sample(sample))
    return start_index, y


def find_min_and_max_values(start_index, y, samples):
    pi = 0
    mi = 0
    print(start_index)
    peak_found = False
    min_found = False
    peak_positions = []
    peak_values = []
    min_positions = []
    min_values = []
    measured_values = []
    max_v = -999.9
    min_v = 999.9
    for ii in range(start_index, len(y)):
        if(y[ii] > max_v):
            max_v = y[ii]
            pi = ii
            m = samples[ii]
            peak_found = True
        elif (y[ii] < max_v) and (ii - pi > 340) and peak_found:
            peak_found = False
            peak_positions.append(pi)
            peak_values.append(max_v)
            measured_values.append(m)
            max_v = y[ii]
        if(y[ii] < min_v):
            min_v = y[ii]
            mi = ii
            min_found = True
        elif (y[ii] > min_v) and (ii - mi > 340) and min_found:
            min_found = False
            min_positions.append(mi)
            min_values.append(min_v)
            min_v = y[ii]
    return {"peak_positions": peak_positions, "peak_values": peak_values,
            "min_positions": min_positions, "min_values": min_values,
            "measured_values": measured_values}

def get_amplitudes(peak_values, min_values, peak_positions, min_positions):
    amplitudes = []
    n_p = len(peak_values) - 1
    n_m = len(min_values) - 2
    k = 0
    p = 0
    if peak_positions[p] < min_positions[k]:
        amplitudes.append(peak_values[p] - min_values[k])
    else:
        amplitudes.append(peak_values[p] - ((min_values[k] + min_values[k + 1]) / 2))
        k = k + 1
    p = p + 1
    while (p < n_p) and (k < n_m):
        amplitudes.append(peak_values[p] - ((min_values[k] + min_values[k + 1]) / 2))
        k = k + 1
        p = p + 1
    return amplitudes


def amplitudes_barplot(amplitudes, measured_values):
    fig = plt.figure()
    fig.set_tight_layout(True)
    ax = fig.add_subplot(111)
    amplitudes = amplitudes[3:]
    measured_values = measured_values[3:-1]
    x = list(range(len(amplitudes)))
    ax.bar(x, amplitudes, width=0.4)
    ax.set_title("Amplitúdók a nyomás értékek függvényében",
                 fontdict=FONT_TITLE)
    ax.set_xticks(list(range(len(measured_values))))
    ax.set_xticklabels(list(map(lambda x: f"{x - OFFSET:.2f}", measured_values)))
    ax.set_xlabel("Korrigált nyers mérési adatok [Hgmm]", fontdict=FONT_LABEL)
    ax.set_ylabel("Amplitúdók becsült értéke [Hgmm]", fontdict=FONT_LABEL)
    plt.setp(ax.xaxis.get_majorticklabels(), rotation=70)


def blood_pressure_measure(result):
    start_index, y = filter_samples(result)
    ret_dict = find_min_and_max_values(start_index, y, result)
    peak_positions = list(map(lambda x: x * 1/FS, ret_dict["peak_positions"]))
    peak_values = ret_dict["peak_values"]
    min_positions = list(map(lambda x: x * 1/FS, ret_dict["min_positions"]))
    min_values = ret_dict["min_values"]
    measured_values = ret_dict["measured_values"]
    print(f"Length of min values: {len(min_values), len(min_positions)}")
    print(f"Length of max values: {len(peak_values), len(peak_positions)}")
    amplitudes = get_amplitudes(peak_values, min_values, peak_positions, min_positions)
    amplitudes_barplot(amplitudes, measured_values)
    max_v = -1
    map_index = 0
    for ii in range(3, len(amplitudes)):
        if(amplitudes[ii] > max_v):
            max_v = amplitudes[ii]
            map_index = ii

    SP = max_v * 0.55
    DP = max_v * 0.7
    print(f"SP: {SP}, DP: {DP}")
    print(f"MAP index: {map_index}")
    delta = 1.0
    sp_value = 0
    dp_value = 0
    sp_index = 0
    dp_index = 0
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
    for ii in range(0, 10):
        pulse = pulse + peak_positions[ii + 1] - peak_positions[ii];
    pulse = 1 / (pulse / 10) * 60
    fig = plt.figure(figsize=(19.2, 10.8))
    N = len(y)
    t = [(1 / FS) * ii for ii in range(0, N)]
    fig.set_tight_layout(True)
    ax = fig.add_subplot(111)
    ax.plot(t, y, label='filtered')
    ax.scatter(peak_positions, peak_values)
    ax.scatter(min_positions, min_values)
    ax.set_title("Lokális maximum és minimum értékek a szűrt jelen", 
                 fontdict=FONT_TITLE)
    ax.set_xlabel("Idő [sec]", fontdict=FONT_LABEL)
    ax.set_ylabel("Nyomás [Hgmm]", fontdict=FONT_LABEL)
    ax.grid(True, 'both')
    plt.show()
    return pulse, sp_value - OFFSET, dp_value - OFFSET


def main():
    parser = argparse.ArgumentParser(description="Blood pressure monitoring script")
    parser.add_argument("--offline", action="store_true")
    parser.add_argument("--test", action="store_true")
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
        result, bpm_res = blood_pressure_monitoring(port, baudrate)
        print(f"result of bpm:\n\tsys:{bpm_res[2]}\n\tdia:{bpm_res[1]}\n\tpulse:{bpm_res[0]}")

    if args.write_file:
        with open(BPM_FILE, "w") as file:
            for ii in result:
                file.write(f"{ii}\n")

    if not args.test:
        pulse, sys, dia = blood_pressure_measure(result)
        fig = plt.figure(figsize=(19.2, 10.8))
        fig.set_tight_layout(True)
        ax = fig.add_subplot(111)
        t = [1/FS * ii for ii in range(len(result))]
        ax.plot(t, result)
        ax.set_title("Offset értékkel terhelt nyers mérési adatok",
                     fontdict=FONT_TITLE)
        ax.set_xlabel("Idő [sec]", fontdict=FONT_LABEL)
        ax.set_ylabel("Nyomás [Hgmm]", fontdict=FONT_LABEL)
        plt.show()
        n = len(result)
        print(f"pulse: {pulse}, sys: {sys}, dia: {dia}")
        result_fft = 1 / n * np.fft.fft(result)


if __name__ == "__main__":
    main()