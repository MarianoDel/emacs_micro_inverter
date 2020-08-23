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
Respuesta_CloseLoop_All_Inputs_Digital_PID_Float = True
Respuesta_CloseLoop_All_Inputs_Digital_PID_Int = True


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

# vin_setpoint = np.ones(t.size)
vin_setpoint = s_sen * 1023
# vin_setpoint = s_sen * 512

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


########################
# LOOP DE  ITERACIONES #
########################
""" 
    Only for PI dig:
    w0 ~= ki_dig / kp_dig * Fsampling or Fundersampling
    plateau gain ~= 20 log kp_dig

"""
# LOW GAIN PID
ki_dig = 3 / 128
# ki_dig = 0.0
kp_dig = 10 / 128
# kp_dig = 0.0
# kd_dig = 5 / 128
kd_dig = 0.0

k1_low = kp_dig + ki_dig + kd_dig
k2_low = -kp_dig - 2*kd_dig
k3_low = kd_dig

print (f'ki_low: {ki_dig} kp_low: {kp_dig} kd_low: {kd_dig}')

# HIGH GAIN PID
ki_dig = 16 / 128
# ki_dig = 0.0
kp_dig = 30 / 128
# kp_dig = 0.0
# kd_dig = 5 / 128
kd_dig = 0.0

k1_high = kp_dig + ki_dig + kd_dig
k2_high = -kp_dig - 2*kd_dig
k3_high = kd_dig

print (f'ki_high: {ki_dig} kp_high: {kp_dig} kd_high: {kd_dig}')

undersampling = 1
print (f'undersamplig freq: {Fsampling/(undersampling + 1)}')


#este es el pid
b_pid = [k1_low, k2_low, k3_low]
a_pid = [1, -1]
pid_tf_float = PID_float(b_pid, a_pid)
signal_phase = 'RISING'

vout_plant = np.zeros(t.size)
d = np.zeros(t.size)
vin_plant[0:3] = 0
vin_plant_d = np.zeros(t.size)

error = np.zeros(t.size)
sensor_adc = np.zeros(t.size)
under = undersampling

for i in range(3, len(vin_plant)):
    ########################################
    # aplico la transferencia de la planta #
    ########################################
    # muestra en el sensor adc
    sensor_adc[i] = vout_plant[i-1] / 3.3 * 1023
    sensor_adc[i] = int(sensor_adc[i])
    if (sensor_adc[i] < 0):
        sensor_adc[i] = 0

    if (sensor_adc[i] > 1023):
        sensor_adc[i] = 1023

    # Veo el error que tengo        
    error[i] = vin_setpoint[i] - sensor_adc[i]

    # aplico el pid
    if (under):
        under -= 1
        d[i] = d[i-1]
    else:
        under = undersampling

        if (signal_phase == 'RISING'):
            if (i > 48):
                signal_phase = 'MIDDLE'
                b_pid = [k1_high, k2_high, k3_high]
                pid_tf_float.changeParams(b_pid, a_pid)

        if (signal_phase == 'MIDDLE'):
            if (i > 160):
                signal_phase = 'FALLING'
                b_pid = [k1_low, k2_low, k3_low]
                pid_tf_float.changeParams(b_pid, a_pid)                
                
        if (signal_phase == 'FALLING'):
            if (i > 204):
                signal_phase = 'REVERT'
                

        d[i] = pid_tf_float.newOutput(error[i])

    # ajusto input a la planta por el efecto PWM
    d[i] = int(d[i])
    if (signal_phase != 'REVERT'):
        if (d[i] < 0):
            d[i] = 0
    else:
        if (d[i] < -100):
            d[i] = -100

    if (d[i] > 1000):
        d[i] = 1000

        
    vin_plant_d[i] = d[i]/1000 * vin_plant[i]

    vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                    + b_sensor[1]*vin_plant_d[i-1] \
                    + b_sensor[2]*vin_plant_d[i-2] \
                    + b_sensor[3]*vin_plant_d[i-3] \
                    - a_sensor[1]*vout_plant[i-1] \
                    - a_sensor[2]*vout_plant[i-2] \
                    - a_sensor[3]*vout_plant[i-3]



if Respuesta_CloseLoop_All_Inputs_Digital_PID_Float == True:     
    fig, ax = plt.subplots()
    ax.set_title(f'Resultados PID Float')
    ax.set_ylabel('Tension en Sensor')
    ax.set_xlabel('Tiempo en muestras')
    ax.grid()
    ax.plot(t, d, 'r', label="d")
    ax.plot(t, vin_setpoint, 'y', label="sp")
    # ax.stem(t, vout_plant)
    ax.plot(t, vout_plant, 'c', label="out")
    ax.plot(t, vin_plant_d, 'm', label="in")
    ax.plot(t, error, 'b', label="error")    
    ax.plot(t, sensor_adc, 'g', label="sensor")
    # ax.stem(t, vin_plant, 'm', label="in")    
    # ax.set_ylim(ymin=-10, ymax=360)
    ax.legend(loc='upper left')
    plt.tight_layout()
    plt.show()



    
# LOW GAIN PID INTEGRAL
ki_dig = 3
kp_dig = 10
kd_dig = 0

k1_low_int = kp_dig + ki_dig + kd_dig
k2_low_int = -kp_dig - 2*kd_dig
k3_low_int = kd_dig
print (f'ki_low: {ki_dig} kp_low: {kp_dig} kd_low: {kd_dig}')

# HIGH GAIN PID
ki_dig = 16
kp_dig = 30
kd_dig = 0

k1_high_int = kp_dig + ki_dig + kd_dig
k2_high_int = -kp_dig - 2*kd_dig
k3_high_int = kd_dig
print (f'ki_high: {ki_dig} kp_high: {kp_dig} kd_high: {kd_dig}')

pid_tf_int = PID_int([k1_low_int, k2_low_int, k3_low_int], a_pid, 128)
signal_phase = 'RISING'

error = np.zeros(t.size)
sensor_adc = np.zeros(t.size)
under = undersampling

for i in range(3, len(vin_plant)):
    ########################################
    # aplico la transferencia de la planta #
    ########################################
    # muestra en el sensor adc
    sensor_adc[i] = vout_plant[i-1] / 3.3 * 1023
    sensor_adc[i] = int(sensor_adc[i])
    if (sensor_adc[i] < 0):
        sensor_adc[i] = 0

    if (sensor_adc[i] > 1023):
        sensor_adc[i] = 1023

    # Veo el error que tengo        
    error[i] = vin_setpoint[i] - sensor_adc[i]

    # aplico el pid
    if (under):
        under -= 1
        d[i] = d[i-1]
    else:
        under = undersampling

        if (signal_phase == 'RISING'):
            if (i > 48):
                signal_phase = 'MIDDLE'
                b_pid = [k1_high_int, k2_high_int, k3_high_int]
                pid_tf_int.changeParams(b_pid, a_pid)

        if (signal_phase == 'MIDDLE'):
            if (i > 160):
                signal_phase = 'FALLING'
                b_pid = [k1_low_int, k2_low_int, k3_low_int]
                pid_tf_int.changeParams(b_pid, a_pid)                
                
        if (signal_phase == 'FALLING'):
            if (i > 204):
                signal_phase = 'REVERT'
                

        d[i] = pid_tf_int.newOutput(error[i])

    # ajusto input a la planta por el efecto PWM
    d[i] = int(d[i])
    if (signal_phase != 'REVERT'):
        if (d[i] < 0):
            d[i] = 0
    else:
        if (d[i] < -100):
            d[i] = -100

    if (d[i] > 1000):
        d[i] = 1000

        
    vin_plant_d[i] = d[i]/1000 * vin_plant[i]

    vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                    + b_sensor[1]*vin_plant_d[i-1] \
                    + b_sensor[2]*vin_plant_d[i-2] \
                    + b_sensor[3]*vin_plant_d[i-3] \
                    - a_sensor[1]*vout_plant[i-1] \
                    - a_sensor[2]*vout_plant[i-2] \
                    - a_sensor[3]*vout_plant[i-3]



if Respuesta_CloseLoop_All_Inputs_Digital_PID_Int == True:     
    fig, ax = plt.subplots()
    ax.set_title(f'Resultados PID Int')
    ax.set_ylabel('Tension en Sensor')
    ax.set_xlabel('Tiempo en muestras')
    ax.grid()
    ax.plot(t, d, 'r', label="d")
    ax.plot(t, vin_setpoint, 'y', label="sp")
    # ax.stem(t, vout_plant)
    ax.plot(t, vout_plant, 'c', label="out")
    ax.plot(t, vin_plant_d, 'm', label="in")
    ax.plot(t, error, 'b', label="error")    
    ax.plot(t, sensor_adc, 'g', label="sensor")
    # ax.stem(t, vin_plant, 'm', label="in")    
    # ax.set_ylim(ymin=-10, ymax=360)
    ax.legend(loc='upper left')
    plt.tight_layout()
    plt.show()


