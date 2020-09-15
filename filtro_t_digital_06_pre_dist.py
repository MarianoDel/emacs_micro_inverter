# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy
from pid_tf import PID_float, PID_int

"""
        ESTE ARCHIVO USA UN CONTROL PID ENTRE CICLOS DE 100HZ - sin undersamplig -
        MicroInversor - Etapa de salida de la parte Inverter
	Etapa C.
        Funcion: Generador de senial senoidal
        Modo: Voltage Mode - VM
        Feedback: por tension de salida, sensada a traves del Rsense

        Nuevo 22-08-2020:
        La TF Planta analogica es tension en Rsense
        La TF Sensor analogica es tension en Rsense * Opamp
        para 1V de entrada
"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Planta_Sensor_Analog = False
Bode_Sensor_Digital = False
Escalon_Sensor_Digital = False
Escalon_Sensor_Digital_Recursivo = False
Vin_Setpoint_PtP_Digital_Signals = False
Respuesta_PreDistorted_All_Inputs_Digital = True


#TF without constant
s = Symbol('s')

# desde Vinput (sin Vinput) al sensor de corriente
Rload = 2.0
Rsense = 0.33
L1 = 1.8e-3
L2 = 1.8e-3
C = 0.44e-6
Amp_gain = 9.2
Vinput = 350    #promedio un input entre 350 cuando no hay contra fem y 20 con contra fem máxima

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

wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)

w, mag_p, phase_p = bode(planta_TF, wfreq)
w, mag_s, phase_s = bode(sensor_TF, wfreq)

if Bode_Planta_Sensor_Analog == True:
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title('Sensor TF green - Planta Rsense blue')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()

###############################################################################################
# Convierto Sensor por Euler-Forward y zoh a una frecuencia mucho mas alta que la de muestreo #
# para que no afecte polos o ceros                                                            #            
###############################################################################################
Fsampling = 24000

sensor_TF = sympy_to_lti(Sensor_out_sim)
print ("sensor con sympy:")
print (sensor_TF)

Fsampling_mult = Fsampling
Tsampling_mult = 1 / Fsampling_mult
    
sensor_dig_ef_n, sensor_dig_ef_d, td = cont2discrete((sensor_TF.num, sensor_TF.den), Tsampling_mult, method='euler')
sensor_dig_zoh_n, sensor_dig_zoh_d, td = cont2discrete((sensor_TF.num, sensor_TF.den), Tsampling_mult, method='zoh')
# sensor_dig_zoh_n, sensor_dig_zoh_d, td = cont2discrete(sensor_TF, Tsampling_mult, method='zoh')

#normalizo con TransferFunction
print ("Sensor Digital Euler-Forward:")
sensor_dig_ef = TransferFunction(sensor_dig_ef_n, sensor_dig_ef_d, dt=td)
print (sensor_dig_ef)

print ("Sensor Digital Zoh:")
sensor_dig_zoh = TransferFunction(sensor_dig_zoh_n, sensor_dig_zoh_d, dt=td)
print (sensor_dig_zoh)

w, mag_ef, phase_ef = dbode(sensor_dig_ef, n = 10000)
w, mag_zoh, phase_zoh = dbode(sensor_dig_zoh, n = 10000)

if Bode_Sensor_Digital == True:
    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_ef, 'g')
    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')    
    ax1.set_title('Digital Euler-Forward Green, ZOH Yellow')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='g')
    ax1.set_xlabel('Frequency [Hz]')
    # ax1.set_ylim(ymin=-40, ymax=40)

    ax2.semilogx(w/(2*np.pi), phase_ef, 'g')
    ax2.semilogx(w/(2*np.pi), phase_zoh, 'y')    
    ax2.set_ylabel('Phase', color='g')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()

#############################################
# Verifico Respuesta Escalon Planta Digital #
#############################################
tiempo_de_simulacion = 0.01
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))
tout, yout_ef = dstep([sensor_dig_ef.num, sensor_dig_ef.den, td], t=t)
yout1 = np.transpose(yout_ef)
yout0 = yout1[0]
yout_ef = yout0[:tout.size]

tout, yout_zoh = dstep([sensor_dig_zoh.num, sensor_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout_zoh)
yout0 = yout1[0]
yout_zoh = yout0[:tout.size]


if Escalon_Sensor_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Euler-Forward Green, ZOH Yellow')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    # ax.plot(tout, yout_ef, 'g')
    ax.plot(tout, yout_zoh, 'y')
    # ax.set_ylim(ymin=-20, ymax=100)

    plt.tight_layout()
    plt.show()

############################################################
# Verifico Respuesta Escalon Planta Convertida a Recursiva #
############################################################
# t = np.linspace(0, tiempo_de_simulacion, num=int(tiempo_de_simulacion/Fsampling))

# ZOH
b_sensor = np.transpose(sensor_dig_zoh_n)
a_sensor = np.transpose(sensor_dig_zoh_d)

print("Recursiva:")
print(f"b[3]: {b_sensor[0]} b[2]: {b_sensor[1]} b[1]: {b_sensor[2]} b[0]: {b_sensor[3]}")
print(f"a[3]: {a_sensor[0]} a[2]: {a_sensor[1]} a[1]: {a_sensor[2]} a[0]: {a_sensor[3]}")

vin_plant = np.ones(t.size)
vin_plant[0:3] = 0
vout_plant = np.zeros (t.size)
for i in range(3, len(vin_plant)):
    ########################################
    # aplico la transferencia de la planta #
    ########################################
    vout_plant[i] = b_sensor[0]*vin_plant[i] \
                    + b_sensor[1]*vin_plant[i-1] \
                    + b_sensor[2]*vin_plant[i-2] \
                    + b_sensor[3]*vin_plant[i-3] \
                    - a_sensor[1]*vout_plant[i-1] \
                    - a_sensor[2]*vout_plant[i-2] \
                    - a_sensor[3]*vout_plant[i-3]

if Escalon_Sensor_Digital_Recursivo == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Recursiva ZOH Yellow')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant, 'y')
    plt.tight_layout()
    plt.show()

####################################################
# DESDE ACA SIGO CON ZOH QUE DA MEJORES RESULTADOS #
####################################################

###############################
# Entrada 1: Vinput - Voutput #
###############################
fmains = 50
s_sen = np.zeros(t.size)

for i in range(np.size(s_sen)):
    s_sen[i] = np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size))

for i in range (np.size(s_sen)):
    if s_sen[i] < 0:
        s_sen[i] = -s_sen[i]

vin_plant = 350 - s_sen * 311
# vin_plant = np.ones(t.size) * 350

#######################################################
# Entrada 2: Armo la senial que quiero en el SETPOINT #
#######################################################
fmains = 50
s_sen = np.zeros(t.size)

for i in range(np.size(s_sen)):
    s_sen[i] = np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size))

for i in range (np.size(s_sen)):
    if s_sen[i] < 0:
        s_sen[i] = -s_sen[i]

Imax = 127
# Imax = 255
# Imax = 511
# Imax = 1023
vin_setpoint = s_sen * Imax

print ('Cant de muestras vin_plant: ' + str(vin_plant.size) + ' vin_setpoint: ' + str(vin_setpoint.size))

if Vin_Setpoint_PtP_Digital_Signals == True:
    fig, ax = plt.subplots()
    ax.set_title('Vin Plant green, Setpoint [i table] yellow')
    ax.set_xlabel('t [s]')
    ax.grid()
    ax.plot(t, vin_plant, 'g')    
    ax.plot(t, vin_setpoint, 'y')
    plt.tight_layout()
    plt.show()


#########################################
# Realimento punto a punto con setpoint #
# OJO ESTO ES UN SISTEMA MISO           #
# Entrada 1: Vinput - Voutput           #
# Entrada 2: d                          #
#########################################
# Respuesta escalon de la planta punto a punto
tiempo_de_simulacion = 0.01
print('td:')
print (td)
t = np.arange(0, tiempo_de_simulacion, td)

# ZOH
b_sensor = np.transpose(sensor_dig_zoh_n)
a_sensor = np.transpose(sensor_dig_zoh_d)


##############################
# Pre-Distorted Duty-Cycle d #
##############################
d = np.zeros(t.size)

d = [ 5,  5,  5,  5,  5,  5,  8,  8,  7,  7,  5,  5,  6,  6,  7,  7,  5,  5,
      6,  6,  7,  7,  7,  7,  7,  7,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10,
      10, 10, 11, 11, 12, 12, 12, 12, 13, 13, 14, 14, 14, 14, 15, 15, 16, 16,
      16, 16, 16, 16, 16, 16, 24, 24, 25, 25, 25, 25, 25, 26, 26, 27, 27, 27,
      30, 30, 32, 32, 32, 32, 33, 33, 34, 34, 37, 37, 38, 38, 40, 40, 42, 42,
      45, 45, 46, 46, 48, 48, 51, 51, 53, 53, 56, 56, 58, 58, 60, 60, 61, 61,
      63, 63, 65, 65, 66, 66, 67, 67, 67, 67, 68, 68, 68, 68, 68, 68, 60, 60,
      60, 60, 60, 60, 55, 55, 55, 55, 50, 50, 50, 50, 45, 45, 40, 40, 36, 36,
      31, 31, 30, 30, 29, 29, 28, 28, 26, 26, 25, 25, 24, 24, 22, 22, 19, 19,
      19, 19, 18, 18, 18, 18, 14, 14, 12, 12, 10, 10, 10, 10, 8, 8,  8,  8,
      8,  8,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  2,  2,  1,  1,  1,  1,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1, -1, -2, -2,
      -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
      -3, -3, -3, -3, -4, -4,]

vin_plant_d = np.zeros(t.size)
vout_plant = np.zeros(t.size)
sensor_adc = np.zeros(t.size)

#####################################
# Ajusto d al valor de la corriente #
#####################################
d = np.array(d)
d = d * Imax / 1023
d = d.astype(dtype=np.int16)

for i in range(3, len(vin_plant)):
    ######################################
    # aplico pre-distorted d a la planta #
    ######################################

    # aplico el pre-distorted d a la planta
    vin_plant_d[i] = d[i]/1000 * vin_plant[i]

    vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                    + b_sensor[1]*vin_plant_d[i-1] \
                    + b_sensor[2]*vin_plant_d[i-2] \
                    + b_sensor[3]*vin_plant_d[i-3] \
                    - a_sensor[1]*vout_plant[i-1] \
                    - a_sensor[2]*vout_plant[i-2] \
                    - a_sensor[3]*vout_plant[i-3]

    # medicion del sensor adc
    sensor_adc[i] = vout_plant[i] / 3.3 * 1023
    sensor_adc[i] = int(sensor_adc[i])
    if (sensor_adc[i] < 0):
        sensor_adc[i] = 0

    if (sensor_adc[i] > 1023):
        sensor_adc[i] = 1023


if Respuesta_PreDistorted_All_Inputs_Digital == True:     
    fig, ax = plt.subplots()
    ax.set_title(f'Resultados Pre Distorted')
    ax.set_ylabel('Tension en Sensor')
    ax.set_xlabel('Tiempo en muestras')
    ax.grid()
    ax.plot(t, d, 'r', label="d")
    ax.plot(t, vin_setpoint, 'y', label="sp")
    ax.plot(t, vout_plant, 'c', label="out")
    ax.plot(t, vin_plant_d, 'm', label="in")
    ax.plot(t, sensor_adc, 'g', label="sensor")
    ax.legend(loc='upper left')
    plt.tight_layout()
    plt.show()


print (d)
    
