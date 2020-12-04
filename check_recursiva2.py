# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand
from recursive_tf import RecursiveTF

"""
        Chequeo de la simulacion recursiva, filtros
        Polo simple RC
"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Planta_Analog = False
Bode_Planta_Digital = False
Polos_Ceros_Digital = False
Escalon_Sensor_Digital = True
Escalon_Sensor_Digital_Recursivo = True


#TF without constant
s = Symbol('s')

# FILTRO RC
R = 1800
C = 56e-9

# Polo simple RC
transf_filtro1 = 1 / (1 + s*R*C)
transf_filtro1_sim = transf_filtro1.simplify()
print ('Planta filtro1:')
print (transf_filtro1_sim)

################################################
# Algunas confirmaciones de la parte Analogica #
################################################
polo_TF = sympy_to_lti(transf_filtro1_sim)

if Bode_Planta_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(polo_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    ax1.set_title('Magnitude')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()



########################################################
# Convierto Planta por zoh a la frecuencia de muestreo #
# para que no afecte polos o ceros                     #
########################################################
Fsampling = 70000
Tsampling = 1 / Fsampling

polo_dig_zoh_n, polo_dig_zoh_d, td = cont2discrete((polo_TF.num, polo_TF.den), Tsampling, method='zoh')

#normalizo con TransferFunction
polo_dig_zoh = TransferFunction(polo_dig_zoh_n, polo_dig_zoh_d, dt=td)
print ("Polo Digital Zoh:")
print (polo_dig_zoh)

print ("Polo Digital Zoh ZPK:")
polo_dig_zoh_z, polo_dig_zoh_p, polo_dig_zoh_k = tf2zpk(polo_dig_zoh_n, polo_dig_zoh_d)
print ("Zeros")
print (polo_dig_zoh_z)
print ("Poles")
print (polo_dig_zoh_p)
print ("Gain")
print (polo_dig_zoh_k)


if Bode_Planta_Digital == True:
    w, magp_zoh, phasep_zoh = dbode(polo_dig_zoh, n = 10000)

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), magp_zoh, 'b')
    ax1.set_title('Digital Bode ZOH')

    ax2.set_title('Phase')    
    ax2.semilogx(w/(2*np.pi), phasep_zoh, 'b')

    plt.tight_layout()
    plt.show()


######################################
# Polos y Ceros de la planta Digital #
######################################
if Polos_Ceros_Digital == True:
    plot_argand(polo_dig_zoh)

    
#############################################
# Verifico Respuesta Escalon Planta Digital #
#############################################
tiempo_de_simulacion = 0.005
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))

tout, yout_zoh = dstep([polo_dig_zoh.num, polo_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout_zoh)
yout0 = yout1[0]
polo_yout_zoh = yout0[:tout.size]

if Escalon_Sensor_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital ZOH')
    ax.set_ylabel('Tension')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()

    ax.plot(tout, polo_yout_zoh, 'b')

    plt.tight_layout()
    plt.show()


############################################################
# Verifico Respuesta Escalon Planta Convertida a Recursiva #
############################################################
if Escalon_Sensor_Digital_Recursivo == True:
    # ZOH Polo
    b_polo = np.transpose(polo_dig_zoh.num)
    a_polo = np.transpose(polo_dig_zoh.den)

    vin_plant = np.ones(t.size)
    vout_plant_polo = np.zeros (t.size)    
    recur_polo = RecursiveTF(b_polo, a_polo)
    print('b_polo:')
    print(b_polo)
    print('a_polo:')
    print(a_polo)    
    for i in range(t.size):
        vout_plant_polo[i] = recur_polo.newOutput(vin_plant[i])


    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Recursiva ZOH')
    ax.set_ylabel('Tension')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant_polo, 'b')
    plt.tight_layout()
    plt.show()
    

print ('first 5 polo_yout_zoh:')
print (polo_yout_zoh[0:5])
print ('first 5 vout_plant_polo:')
print (vout_plant_polo[0:5])
