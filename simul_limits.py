# -*- coding: utf-8 -*-
#usar python3
import numpy as np
import matplotlib.pyplot as plt
# import sys
# import math


################################################
# Open the data file with the vectors to graph #
################################################
file = open ('data.txt', 'r')

###################
# Get the vectors #
###################
#readlines reads the individual line into a list
fl =file.readlines()

v1 = fl[0]
v1 = v1.strip('\n')
str_data = str(fl[1])
reference_data = np.fromstring(str_data, dtype=np.uint16, sep=' ')
print(f"vector: {v1} numpy array size: {reference_data.size} first element: {reference_data[0]}")

v2 = fl[2]
v2 = v2.strip('\n')
str_data = str(fl[3])
duty_data = np.fromstring(str_data, dtype=np.uint16, sep=' ')
print(f"vector: {v2} numpy array size: {duty_data.size} first element: {duty_data[0]}")

v3 = fl[4]
v3 = v3.strip('\n')
str_data = str(fl[5])
vinput_data = np.fromstring(str_data, dtype=np.float32, sep=' ')
print(f"vector: {v3} numpy array size: {vinput_data.size} first element: {vinput_data[0]}")

v4 = fl[6]
v4 = v4.strip('\n')
str_data = str(fl[7])
voutput_data = np.fromstring(str_data, dtype=np.float32, sep=' ')
print(f"vector: {v4} numpy array size: {voutput_data.size} first element: {voutput_data[0]}")

v5 = fl[8]
v5 = v5.strip('\n')
str_data = str(fl[9])
adc_data = np.fromstring(str_data, dtype=np.uint16, sep=' ')
print(f"vector: {v5} numpy array size: {adc_data.size} first element: {adc_data[0]}")

# d3 = fl[4]
# d3 = d3.strip('\n')
# max_power = int(fl[5])
# print(f"data: {d3} value: {max_power}")

file.close()
###########################
# Armo la senial temporal #
###########################
t = np.linspace(0, reference_data.size, num=reference_data.size)
# vmax_power = np.ones(t.size) * max_power

fig, ax = plt.subplots()
ax.set_title('Output')
# ax.set_ylabel('PWM')
ax.set_xlabel('Tiempo en muestras')
ax.grid()
ax.plot(t, reference_data, 'y', label=v1)
ax.plot(t, duty_data, 'b', label=v2)
ax.plot(t, adc_data, 'g', label=v5)
ax.legend(loc='upper left')
plt.tight_layout()
plt.show()


fig, ax = plt.subplots()
ax.set_title('Input Applied, Output Getted')
# ax.set_ylabel('PWM')
ax.set_xlabel('Tiempo en muestras')
ax.grid()
ax.plot(t, vinput_data, 'y', label=v3)
ax.plot(t, voutput_data, 'g', label=v4)
ax.legend(loc='upper left')
plt.tight_layout()
plt.show()
