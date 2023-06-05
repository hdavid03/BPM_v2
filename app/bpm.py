#!/usr/bin/env python3

import serial
import configparser
import array
import peakfilter
import argparse
import numpy as np
from matplotlib import pyplot as plt

# Global Literals
BPM_FILE = "bpm.txt"
CONFIG_FILE = "config.ini"
TIMEOUT = 2
BUFFERSIZE = 512
TIME_OFFSET = 380
PEAK_OFFSET = 0.065
OFFSET = 29.91
FS = 1200
UART_STOP = 1e6
PUMP_STOP = 195
SYS_RATIO = 0.55
DIA_RATIO = 0.78
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
    _, ax = create_figure()
    floats = []
    bpm_res = []
    stop = False
    while not stop:
        float_bytes = serial_port.read_until(b"XXXX", BUFFERSIZE)
        floats += array.array('f', float_bytes).tolist()
        if floats[-1] > UART_STOP:
            stop = True
            floats.pop()
            bpm_res.append(floats.pop())
            bpm_res.append(floats.pop())
            bpm_res.append(floats.pop())
        ax.clear()
        ax.plot(floats)
        plt.pause(0.01)
    serial_port.close()
    return floats, bpm_res


def create_figure():
    fig = plt.figure(figsize=(19.20, 10.80))
    fig.set_tight_layout(True)
    ax = fig.add_subplot(111)
    return fig, ax


def filter_samples(result):
    filt = peakfilter.PeakFilter()
    start_index = 0
    y = []
    pump = True
    wait = False
    for ii in range(0, len(result)):
        sample = result[ii]
        if pump and (sample > PUMP_STOP):
            pump = False
            wait = True
        if wait and (sample < PUMP_STOP):
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
    max_v = -999.9
    min_v = 999.9
    for ii in range(start_index, len(y)):
        if(y[ii] > max_v):
            max_v = y[ii]
            pi = ii
            m = samples[ii]
            peak_found = True
        elif (y[ii] < max_v - PEAK_OFFSET) and (ii - pi > TIME_OFFSET) and peak_found:
            peak_found = False
            peak_positions.append(pi)
            peak_values.append(max_v)
            measured_values.append(m)
            max_v = y[ii]
        if(y[ii] < min_v):
            min_v = y[ii]
            mi = ii
            min_found = True
        elif (y[ii] > min_v + PEAK_OFFSET) and (ii - mi > TIME_OFFSET) and min_found:
            min_found = False
            min_positions.append(mi)
            min_values.append(min_v)
            min_v = y[ii]
    print(f"Length peaks: {len(peak_positions)}, length mins: {len(min_positions)}")
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
    _, ax = create_figure()
    amplitudes = amplitudes[3:]
    measured_values = measured_values[3:-1]
    x = list(range(len(amplitudes)))
    ax.bar(x, amplitudes, width=0.4)
    ax.set_title("Amplitúdók a nyomás értékek függvényében",
                 fontdict=FONT_TITLE)
    ax.set_xticks(list(range(len(measured_values))))
    ax.set_xticklabels(list(map(lambda x: f"{x:.2f}", measured_values)))
    ax.set_xlabel("Nyers mérési adatok [Hgmm]", fontdict=FONT_LABEL)
    ax.set_ylabel("Amplitúdók becsült értéke [Hgmm]", fontdict=FONT_LABEL)
    ax.grid(True, 'both', 'y')
    plt.setp(ax.xaxis.get_majorticklabels(), rotation=70)


def filtered_signal_plot(t, y, peak_positions, peak_values, min_positions, min_values):
    _, ax = create_figure()
    ax.plot(t, y, label='filtered')
    ax.scatter(peak_positions, peak_values, marker='v', c='red')
    ax.scatter(min_positions, min_values, marker='^', c='purple')
    ax.set_title("Lokális maximum és minimum értékek a szűrt jelen", 
                 fontdict=FONT_TITLE)
    ax.set_xlabel("Idő [sec]", fontdict=FONT_LABEL)
    ax.set_ylabel("Nyomás [Hgmm]", fontdict=FONT_LABEL)
    ax.grid(True, 'both')


def calculate_pulse(peak_positions):
    pulse = 0
    n = len(peak_positions) - 1
    for ii in range(0, n):
        pulse = pulse + peak_positions[ii + 1] - peak_positions[ii];
    return 1 / (pulse / n) * 60


def blood_pressure_measure(result):
    start_index, y = filter_samples(result)
    ret_dict = find_min_and_max_values(start_index, y, result)
    peak_positions = list(map(lambda x: x * 1/FS, ret_dict["peak_positions"]))
    peak_values = ret_dict["peak_values"]
    min_positions = list(map(lambda x: x * 1/FS, ret_dict["min_positions"]))
    min_values = ret_dict["min_values"]
    measured_values = ret_dict["measured_values"]
    amplitudes = get_amplitudes(peak_values, min_values, peak_positions, min_positions)
    amplitudes_barplot(amplitudes, measured_values)
    max_v = -1
    map_index = 0
    for ii in range(3, len(amplitudes)):
        if(amplitudes[ii] > max_v):
            max_v = amplitudes[ii]
            map_index = ii

    SP = max_v * SYS_RATIO
    DP = max_v * DIA_RATIO
    delta = 1.0
    sp_value = 0
    dp_value = 0
    for ii in range(3, map_index - 1):
        d = abs(amplitudes[ii] - SP)
        if d < delta:
            delta = d
            sp_value = measured_values[ii]

    delta = 1.0
    for ii in range(map_index + 1, len(amplitudes)):
        d = abs(amplitudes[ii] - DP)
        if d < delta:
            delta = d
            dp_value = measured_values[ii]

    pulse = calculate_pulse(peak_positions)
    N = len(y)
    t = [(1 / FS) * ii for ii in range(0, N)]
    filtered_signal_plot(t, y, peak_positions, peak_values, min_positions, min_values)
    plt.show()
    return pulse, sp_value, dp_value


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
        _, ax = create_figure()
        t = [1/FS * ii for ii in range(len(result))]
        ax.plot(t, result)
        ax.set_title("Nyers mérési adatok", fontdict=FONT_TITLE)
        ax.set_xlabel("Idő [sec]", fontdict=FONT_LABEL)
        ax.set_ylabel("Nyomás [Hgmm]", fontdict=FONT_LABEL)
        ax.grid(True, 'both')
        plt.show()
        print(f"result of bpm (offline):\n\tsys:{sys}\n\tdia:{dia}\n\tpulse:{pulse}")


if __name__ == "__main__":
    main()