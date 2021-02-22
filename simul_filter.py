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
input_data = np.fromstring(str_data, dtype=np.float32, sep=' ')
print(f"vector: {v1} numpy array size: {input_data.size} first element: {input_data[0]}")

v2 = fl[2]
v2 = v2.strip('\n')
str_data = str(fl[3])
output_data = np.fromstring(str_data, dtype=np.float32, sep=' ')
print(f"vector: {v2} numpy array size: {output_data.size} first element: {output_data[0]}")

file.close()
###########################
# Armo la senial temporal #
###########################
t = np.linspace(0, input_data.size, num=input_data.size)
# vmax_power = np.ones(t.size) * max_power

fig, ax = plt.subplots()
ax.set_title('Input & Output')
ax.set_xlabel('Tiempo en muestras')
ax.grid()
ax.plot(t, input_data, 'r', label=v1)
ax.plot(t, output_data, 'y', label=v2)
ax.legend(loc='upper right')
plt.tight_layout()
plt.show()
