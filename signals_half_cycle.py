# -*- coding: utf-8 -*-
#usar python3
import numpy as np
import matplotlib.pyplot as plt

# signal = 'SINUSOIDAL'
signal = 'TRIANGULAR'
samples = 120
Vmax = 4095

if signal == 'SINUSOIDAL':
    s_sen = np.zeros(samples)
    for i in range(np.size(s_sen)):
        s_sen[i] = np.sin(np.pi*(i+1)/samples) * Vmax

    signal_int = s_sen.astype(int)
    print (signal_int)


if signal == 'TRIANGULAR':
    s_triang = np.zeros(samples)
          
    for i in range(int(np.size(s_triang)/2)):
        s_triang[i] = Vmax * i / int(np.size(s_triang)/2)

    for i in range(int(np.size(s_triang)/2), np.size(s_triang)):
        s_triang[i] = (Vmax * (np.size(s_triang) - i)) / int(np.size(s_triang)/2)
        
    signal_int = s_triang.astype(int)
    print (signal_int)
    


### imprimo para codico C ###
cant_por_linea = 10
linea = 1

print ("{",end='')
for i in range(np.size(signal_int)):
    if i < ((linea * cant_por_linea) - 1):
        print (str(signal_int[i]) + ",",end='')
    else:
        if i == (np.size(signal_int) - 1):
            print (str(signal_int[i]),end='')
        else:                
            print (str(signal_int[i]) + ",\n",end='')
            linea += 1
        
print ("};")

