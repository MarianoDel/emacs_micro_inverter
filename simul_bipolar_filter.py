# -*- coding: utf-8 -*-
#usar python3
import numpy as np
import matplotlib.pyplot as plt
# import sys
# import math

#####################
# Utility Functions #
#####################
def get_vector_lines_float (myfile, line):
    v_str = myfile[line]
    v_str = v_str.strip('\n')
    str_data = str(fl[line + 1])
    v_data = np.fromstring(str_data, dtype=np.float32, sep=' ')
    print(f"vector: {v_str} numpy array size: {v_data.size} first element: {v_data[0]}")
    return v_data

def get_vector_lines_short (myfile, line):
    v_str = myfile[line]
    v_str = v_str.strip('\n')
    str_data = str(fl[line + 1])
    v_data = np.fromstring(str_data, dtype=np.int16, sep=' ')
    print(f"vector: {v_str} numpy array size: {v_data.size} first element: {v_data[0]}")
    return v_data

def get_vector_lines_ushort (myfile, line):
    v_str = myfile[line]
    v_str = v_str.strip('\n')
    str_data = str(fl[line + 1])
    v_data = np.fromstring(str_data, dtype=np.uint16, sep=' ')
    print(f"vector: {v_str} numpy array size: {v_data.size} first element: {v_data[0]}")
    return v_data


################################################
# Open the data file with the vectors to graph #
################################################
file = open ('data.txt', 'r')

###################
# Get the vectors #
###################
#readlines reads the individual line into a list
fl =file.readlines()

vline = get_vector_lines_float (fl, 0)
vinput = get_vector_lines_float (fl, 2)

reference = get_vector_lines_short (fl, 4)
duty_bipolar = get_vector_lines_short (fl, 6)
vapplied = get_vector_lines_float (fl, 8)
voutput = get_vector_lines_float (fl, 10)

adc_data = get_vector_lines_ushort(fl, 12)

file.close()
###########################
# Armo la senial temporal #
###########################
t = np.linspace(0, reference.size, num=reference.size)
# vmax_power = np.ones(t.size) * max_power

fig, ax = plt.subplots()
ax.set_title('Supply and Line')
# ax.set_ylabel('PWM')
ax.set_xlabel('Tiempo en muestras')
ax.grid()
ax.plot(t, vline, 'y', label='vline')
ax.plot(t, vinput, 'b', label='vinput')
ax.legend(loc='upper left')
plt.tight_layout()
plt.show()


fig, ax = plt.subplots()
ax.set_title('Output')
# ax.set_ylabel('PWM')
ax.set_xlabel('Tiempo en muestras')
ax.grid()
ax.plot(t, reference, 'y', label='reference')
ax.plot(t, duty_bipolar, 'b', label='duty_bipolar')
ax.plot(t, adc_data, 'g', label='adc_data')
ax.legend(loc='upper left')
plt.tight_layout()
plt.show()


# fig, ax = plt.subplots()
# ax.set_title('Input Applied, Output Getted')
# # ax.set_ylabel('PWM')
# ax.set_xlabel('Tiempo en muestras')
# ax.grid()
# ax.plot(t, vinput_data, 'y', label=v3)
# ax.plot(t, voutput_data, 'g', label=v4)
# ax.legend(loc='upper left')
# plt.tight_layout()
# plt.show()
