# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand
from pid_tf import PID_float, PID_int

"""
        All Digital 320V alimentacion
"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Sensor_Digital = True
Polos_Ceros_Planta_Digital = True
Escalon_Sensor_Digital = True
Escalon_Sensor_Digital_Recursivo = True
Bode_Controlador_Digital = True
Bode_Sensor_CloseLoop_Digital = True
Polos_Ceros_CloseLoop_Digital = True


#TF without constant
s = Symbol('s')

# desde Vinput (sin Vinput) al sensor de corriente
Rload = 2.0
Rsense = 0.33
L1 = 1.8e-3
L2 = 1.8e-3
C = 0.44e-6
Amp_gain = 9.2
Vinput = 320    #promedio un input entre 350 cuando no hay contra fem y 20 con contra fem máxima

#    ---Z1---+---Z3---+
#            |        |
#           Z2       Z4
#            |        |
#    --------+--------+
Z1 = s * L1
Z2 = 1 / (s * C)
Z3 = s * L2 + Rload
Z4 = Rsense

## Thevenin en Z2
Rth = Z1 * Z2 / (Z1 + Z2)

# Vth = Vinput * Z2 / (Z1 + Z2)
# VZ4 = Vth * Z4 / (Rth + Z3 + Z4)
# VZ4 = (Vinput * Z2 / (Z1 + Z2)) * Z4 / (Rth + Z3 + Z4)
# VZ4/Vinput = (Z2/(Z1+Z2))*Z4/(Rth+Z3+Z4)

Transf_Z4_Vinput = Z2 * Z4 /((Z1 + Z2) * (Rth + Z3 + Z4))
Plant_out_sim = Transf_Z4_Vinput.simplify()

print ('Plant_out: ')
print (Plant_out_sim)

Sensor_out = Plant_out_sim * Amp_gain
Sensor_out_sim = Sensor_out.simplify()

print ('Sensor_out: ')
print (Sensor_out_sim)

#####################################################
# Desde aca utilizo ceros y polos que entrego sympy #
#####################################################
planta_TF = sympy_to_lti(Plant_out_sim)
print ("planta con sympy:")
print (planta_TF)

sensor_TF = sympy_to_lti(Sensor_out_sim)
print ("sensor con sympy:")
print (sensor_TF)


###########################################################################
# Convierto Sensor ZOH a una frecuencia mucho mas alta que la de muestreo #
# para que no afecte polos o ceros                                        #
###########################################################################
Fsampling = 24000

sensor_TF = sympy_to_lti(Sensor_out_sim * Vinput)
print (f"Input {Vinput}V sensor con sympy:")
print (sensor_TF)

Fsampling_mult = Fsampling
Tsampling_mult = 1 / Fsampling_mult
    
sensor_dig_zoh_n, sensor_dig_zoh_d, td = cont2discrete((sensor_TF.num, sensor_TF.den), Tsampling_mult, method='zoh')

#normalizo con TransferFunction
sensor_dig_zoh = TransferFunction(sensor_dig_zoh_n, sensor_dig_zoh_d, dt=td)

################
# Digital Bode #
################
w, mag_zoh, phase_zoh = dbode(sensor_dig_zoh, n = 10000)

if Bode_Sensor_Digital == True:
    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_zoh, 'g')
    ax1.set_title(f'Input {Vinput}V green - ZOH -')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='g')
    ax1.set_xlabel('Frequency [Hz]')
    # ax1.set_ylim(ymin=-40, ymax=40)

    ax2.semilogx(w/(2*np.pi), phase_zoh, 'g')
    ax2.set_ylabel('Phase', color='g')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Polos_Ceros_Planta_Digital == True:
    plot_argand(sensor_dig_zoh)

#############################################
# Verifico Respuesta Escalon Planta Digital #
#############################################
tiempo_de_simulacion = 0.01
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))

tout, yout_zoh = dstep([sensor_dig_zoh.num, sensor_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout_zoh)
yout0 = yout1[0]
yout_zoh = yout0[:tout.size]

if Escalon_Sensor_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title(f'Step Planta Digital {Vinput}V - ZOH -')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(tout, yout_zoh, 'g')
    # ax.set_ylim(ymin=-20, ymax=100)

    plt.tight_layout()
    plt.show()

############################################################
# Verifico Respuesta Escalon Planta Convertida a Recursiva #
############################################################
# ZOH
b_sensor = np.transpose(sensor_dig_zoh_n)
a_sensor = np.transpose(sensor_dig_zoh_d)

vin_plant = np.ones(t.size)
vin_plant[0:3] = 0
vout_plant = np.zeros (t.size)

########################################
# aplico la transferencia de la planta #
########################################
for i in range(3, len(vin_plant)):
    vout_plant[i] = b_sensor[0]*vin_plant[i] \
                       + b_sensor[1]*vin_plant[i-1] \
                       + b_sensor[2]*vin_plant[i-2] \
                       + b_sensor[3]*vin_plant[i-3] \
                       - a_sensor[1]*vout_plant[i-1] \
                       - a_sensor[2]*vout_plant[i-2] \
                       - a_sensor[3]*vout_plant[i-3]

if Escalon_Sensor_Digital_Recursivo == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Recursiva 320V - ZOH -')    
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant, 'g')
    plt.tight_layout()
    plt.show()


#################
# Digitals PIDs #
#################
""" 
    Only for PI dig:
    w0 ~= ki_dig / kp_dig * Fsampling or Fundersampling
    plateau gain ~= 20 log kp_dig

    Only for PD dig:
    w0 ~= kp_dig / kd_dig * Fsampling or Fundersampling
    plateau gain ~= 20 log kp_dig

"""
ki_dig = 0.0
kp_dig = 10 / 128
kd_dig = 0.0

k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

b_pid = [k1, k2, k3]
a_pid = [1, -1]

pid_update_freq = 4000
undersampling_td = 1 / pid_update_freq
controller_tf = TransferFunction(b_pid, a_pid, dt=undersampling_td)


###############
# OpenLoop TF #
###############
w, mag, phase = dbode(controller_tf, n = 10000)


if Bode_Controlador_Digital == True:
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag, 'g')
    ax1.set_title(f'Digital Controller, update freq: {pid_update_freq}Hz')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='g')
    ax1.set_xlabel('Frequency [Hz]')

    ax2.semilogx(w/(2*np.pi), phase, 'g')
    ax2.set_ylabel('Phase', color='c')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Bode_Sensor_CloseLoop_Digital == True:
    contr_dig = lti_to_sympy(controller_tf)
    plant_dig = lti_to_sympy(sensor_dig_zoh)
    
    ol_dig = plant_dig * contr_dig
    open_loop_dig = sympy_to_lti(ol_dig)
    close_loop_dig = sympy_to_lti(ol_dig/(1+ol_dig))

    
    #normalizo con TransferFunction
    open_loop_dig = TransferFunction(open_loop_dig.num, open_loop_dig.den, dt=td)    
    close_loop_dig = TransferFunction(close_loop_dig.num, close_loop_dig.den, dt=td)

    w, mag_ol, phase_ol = dbode(open_loop_dig, n = 10000)
    w, mag_cl, phase_cl = dbode(close_loop_dig, n = 10000)
    
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag_ol, 'g')
    ax1.semilogx(w/(2*np.pi), mag_cl, 'y')    
    ax1.set_title('Open Loop Green, Close Loop Yellow')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='b')
    ax1.set_xlabel('Frequency [Hz]')
    ax1.set_ylim(ymin=-40, ymax=40)

    ax2.semilogx(w/(2*np.pi), phase_ol, 'g')
    ax2.semilogx(w/(2*np.pi), phase_cl, 'y')    
    ax2.set_ylabel('Phase', color='b')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Polos_Ceros_CloseLoop_Digital == True:
    plot_argand(close_loop_dig)
