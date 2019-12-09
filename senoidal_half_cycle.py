# -*- coding: utf-8 -*-
#usar python3
import numpy as np
import matplotlib.pyplot as plt

muestras = 120
Vmax = 1023

s_sen = np.zeros(muestras)

for i in range(np.size(s_sen)):
    s_sen[i] = np.sin(np.pi*(i+1)/muestras) * Vmax

s_sen_enteros = s_sen.astype(int)

print (s_sen_enteros)



### imprimo para codico C ###
cant_por_linea = 10
linea = 1

print ("{",end='')
for i in range(np.size(s_sen_enteros)):
    if i < ((linea * cant_por_linea) - 1):
        print (str(s_sen_enteros[i]) + ",",end='')
    else:
        if i == (np.size(s_sen_enteros) - 1):
            print (str(s_sen_enteros[i]),end='')
        else:                
            print (str(s_sen_enteros[i]) + ",\n",end='')
            linea += 1
        
print ("};")

