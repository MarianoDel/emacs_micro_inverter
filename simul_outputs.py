# -*- coding: utf-8 -*-
# use with python3
import numpy as np
import matplotlib.pyplot as plt


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

def get_vector_data (myfile, line):
    v_str = myfile[line]
    v_data = v_str.split(' ')
    v_data_type = v_data[0]
    v_name = v_data[1]
    return (v_data_type, v_name)

last_color = 0
colors_list = ['o', 'y', 'g', 'c', 'r', 'b']
def get_new_color_vector ():
    global last_color
    
    if last_color < len(colors_list):
        last_color += 1
    else:
        last_color = 0

    return colors_list[last_color]
        
################################################
# Open the data file with the vectors to graph #
################################################
file = open ('data.txt', 'r')

###################
# Get the vectors #
###################
#readlines reads the individual line into a list
fl = file.readlines()

#get line number and vectors qtty
lines = len(fl)
vectors_len = int(lines / 2)
print ('lines: ' + str(lines) + ' vectors: ' + str(vectors_len))

vectors_type = []
vectors_name = []
vectors_data = []

for x in range(vectors_len):
    (new_type, new_name) = get_vector_data(fl, x * 2)
    vectors_type.append(new_type)
    vectors_name.append(new_name)

    if new_type == 'ushort':
        new_data = get_vector_lines_ushort(fl, x * 2)
    elif new_type == 'short':
        new_data = get_vector_lines_short(fl, x * 2)
    elif new_type == 'float':
        new_data = get_vector_lines_float(fl, x * 2)
        
    vectors_data.append(new_data)
    
file.close()
###########################
# Armo la senial temporal #
###########################
if vectors_len:
    time_ref = vectors_data[0]
    t = np.linspace(0, time_ref.size, num=time_ref.size)

    fig, ax = plt.subplots()
    ax.set_title('Supply and Line')
    ax.set_xlabel('Tiempo en muestras')
    ax.grid()
    for x in range(vectors_len):        
        ax.plot(t, vectors_data[x], get_new_color_vector(), label=vectors_name[x])

    ax.legend(loc='upper left')
    plt.tight_layout()
    plt.show()

