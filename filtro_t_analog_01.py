# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy

"""
        MicroInversor - Etapa de salida de la parte Inverter
	Etapa C.
        Funcion: Generador de senial senoidal
        Modo: Voltage Mode - VM
        Feedback: por tension de salida, sensada a traves del opto
"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Planta_Sensor_Analog = True
Escalon_Sensor_Analog = True
Bode_Controlador_Analog = True
Bode_Sensor_OpenLoop_CloseLoop_Analog = True
Escalon_CloseLoop_Analog = True
Respuesta_CloseLoop_All_Inputs_Analog = True
Respuesta_CloseLoop_All_Inputs_Digital = True

#TF without constant
s = Symbol('s')

# desde Vinput (sin Vinput) al sensor de corriente
Rload = 2.0
Rsense = 0.33
L1 = 1e-3
L2 = 1e-3
C = 0.44e-6
Amp_gain = 9.2
Vinput = 175    #promedio un input entre 350 cuando no hay contra fem y 20 con contra fem máxima

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

Sensor_out = Plant_out_sim * Amp_gain * Vinput
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

wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)

w, mag_p, phase_p = bode(planta_TF, wfreq)
w, mag_s, phase_s = bode(sensor_TF, wfreq)

if Bode_Planta_Sensor_Analog == True:
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title('Magnitude')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()

#####################################
# Desde aca hago pruebas temporales #
#####################################
tiempo_de_simulacion = 0.1
t = np.linspace(0, tiempo_de_simulacion, num=2000)
u = np.ones_like(t)
t, y, x = lsim(sensor_TF, u, t)

if Escalon_Sensor_Analog == True:
    fig.clear()
    fig, ax = plt.subplots()
    ax.set_title('Respuesta de la Planta vista desde el sensor')
    ax.set_ylabel('Vsensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, y, 'g-')
    ax.plot(t, u, color='orange')

    plt.tight_layout()
    plt.show()


#########################
# Controlador Propuesto #
#########################
"""    
        PID Analogico
        PID completo Tf = Kp + Ki/s + s Kd    Tf = 1/s * (s**2 Kd + s Kp + Ki)
        muy dificil analizar, basicamente polo en origen y dos ceros
        los dos ceros, segun los parametros elegidos, pueden llegar a ser complejos conjugados

        si fuese solo PI tengo Tf = 1/s * Kp * (s + Ki/Kp)
        esto es polo en origen w = 1; cero en w = Ki/Kp; ganancia Kp

        si fuese solo PD tengo Tf = Kd * (s + Kp/Kd)
        esto es cero en w = Kp/Kd y ganancia Kd
        o la ganancia es Kp??

        Conclusion:
        elijo Kp para la ganancia media, ej 0dB Kp = 1
        elijo primer cero, ej 15.9Hz, Ki = 100
        elijo segundo cero, ej 1590Hz, Kd = 0.0001
"""
#########################
# Controlador analogico #
#########################
kp_analog = 0.005    # -46dB ganancia media
ki_analog = 0.314     # cero en 10Hz
kd_analog = 0.0    #75e-9      # segundo cero en 10600Hz

#3.3, 192, 0.00086 puede ir
#1, 2300, 0 por simulacion puede ir
#4.2; 100; 0.00086 ajusta bien la bajada
#3.2; 200; 0.00086 ajusta bastante bien

# controller = kp_analog + ki_analog/s + s*kd_analog

########################
# Or Custom Controller #
########################
controller = (6.28*0.04)/(6.28*10000*6.28*170)*(s+6.28*10000)*(s+6.28*170)
controller_TF = sympy_to_lti(controller)
print ("controller con sympy:")
print (controller_TF)


###########################################
# Respuesta en Frecuencia del Controlador #
###########################################
w, mag, phase = bode(controller_TF, wfreq)

if Bode_Controlador_Analog == True:
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/(2*np.pi), mag, 'b-', linewidth="1")
    ax1.set_title('Controller Tf - Magnitude')

    ax2.semilogx (w/(2*np.pi), phase, 'b-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()


#######################################################
# Multiplico Transferencias para OpenLoop y CloseLoop #
#######################################################
c = lti_to_sympy(controller_TF)
p = lti_to_sympy(sensor_TF)

ol = c * p
cl = ol/(1+ol)

open_loop = sympy_to_lti(ol)
open_loop = TransferFunction(open_loop.num, open_loop.den)   #normalizo ol
close_loop = sympy_to_lti(cl)
close_loop = TransferFunction(close_loop.num, close_loop.den)   #normalizo cl

w, mag_ol, phase_ol = bode(open_loop, wfreq)
w, mag_cl, phase_cl = bode(close_loop, wfreq)

if Bode_Sensor_OpenLoop_CloseLoop_Analog == True:
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag_ol, 'b')
    ax1.semilogx(w/(2*np.pi), mag_cl, 'y')
    ax1.set_title('Analog OpenLoop Blue, CloseLoop Yellow')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='b')
    ax1.set_xlabel('Frequency [Hz]')
    # ax1.set_ylim([-40, 40])

    ax2.semilogx(w/(2*np.pi), phase_ol, 'b')
    ax2.semilogx(w/(2*np.pi), phase_cl, 'y')
    ax2.set_ylabel('Phase', color='r')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


######################################
# Realimento y veo Respuesta escalon #
######################################
t = np.linspace(0, tiempo_de_simulacion, num=2000)
t, y = step2(close_loop, T=t)

if Escalon_CloseLoop_Analog == True:
    # fig.clear()
    fig, ax = plt.subplots()
    ax.set_title('Respuesta escalon Close Loop')
    ax.set_ylabel('Vout')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, y)

    plt.tight_layout()
    plt.show()

#####################################################
# Realimento y veo Respuesta a Senoidal rectificada #
#####################################################
fmains = 50
s_sen = np.zeros(t.size)

for i in range(np.size(s_sen)):
    s_sen[i] = np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size))

for i in range (np.size(s_sen)):
    if s_sen[i] < 0:
        s_sen[i] = -s_sen[i]

# vin_setpoint = np.ones(t.size)
vin_setpoint = s_sen
t, y, x = lsim(close_loop, vin_setpoint, t)

if Respuesta_CloseLoop_All_Inputs_Analog == True:
    # fig.clear()
    fig, ax = plt.subplots()
    ax.set_title('Respuesta escalon Close Loop')
    ax.set_ylabel('Vout')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, y)
    ax.plot(t, vin_setpoint, color='orange')

    plt.tight_layout()
    plt.show()





