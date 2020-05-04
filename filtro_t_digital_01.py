# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand

"""
        MicroInversor - Etapa de salida de la parte Inverter
	Etapa C.
        Funcion: Generador de senial senoidal
        Modo: Voltage Mode - VM
        Feedback: por tension de salida, sensada a traves del Rsense
        Incluyo la tension de alimentacion de 350V

"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Sensor_Digital = False
Polos_Ceros_Digital = False
Escalon_Sensor_Digital = False
Escalon_Sensor_Digital_Recursivo = False
Bode_Controlador_Digital = True
Bode_Sensor_OpenLoop_CloseLoop_Digital = False
Polos_Ceros_CloseLoop_Digital = False
Escalon_CloseLoop_Digital = False
Vinput_PtP_Digital = False
Setpoint_PtP_Digital = False
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


##################################################################################
# Convierto Planta por Euler-Forward y zoh a una frecuencia mucho mas alta que la de muestreo #
# para que no afecte polos o ceros                                               #                         
##################################################################################
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

print ("Sensor Digital Zoh ZPK:")
sensor_dig_zoh_z, sensor_dig_zoh_p, sensor_dig_zoh_k = tf2zpk(sensor_dig_zoh_n, sensor_dig_zoh_d)
print ("Zeros")
print (sensor_dig_zoh_z)
print ("Poles")
print (sensor_dig_zoh_p)
print ("Gain")
print (sensor_dig_zoh_k)

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


######################################
# Polos y Ceros de la planta Digital #
######################################
if Polos_Ceros_Digital == True:
    plot_argand(sensor_dig_zoh)

    
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


vin_plant = np.ones(t.size)
vin_plant[0:3] = 0
vout_plant = np.zeros (t.size)
b_params = b_sensor
a_params = a_sensor
b_size = b_sensor.size
a_size = a_sensor.size
if b_size > a_size:
    ba_max_size = b_size
else:
    ba_max_size = a_size

print (f"b_params {b_params} a_params {a_params}")
print (f"b_size {b_size} a_size {a_size}")
print (f"ba_max_size {ba_max_size}")
for i in range((ba_max_size - 1), len(vin_plant)):
    vout_plant[i] = 0
    for b_index in range(b_size):
        vout_plant[i] += b_params[b_index] * vin_plant[i - b_index]

    for a_index in range(1, a_size):
        vout_plant[i] -= a_params[a_index] * vout_plant[i - a_index]


# vin_plant = np.ones(t.size)
# vin_plant[0:3] = 0
# vout_plant = np.zeros (t.size)
# for i in range (3, len(vin_plant)):
#     vout_plant[i] = 0.054 * vin_plant[i-1] \
#                     + 0.12 * vin_plant[i-2] \
#                     + 0.052 * vin_plant[i-3] \
#                     - 0.89 * vout_plant[i-1] \
#                     + 0.8 * vout_plant[i-2] \
#                     + 0.91 * vout_plant[i-3]
    
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

########################
# Ecuacion PID Digital #
########################
# Use these from Analog to Digital PID
# kp = kp_analog
# ki = ki_analog
# kd = kd_analog
# kp = 1.0 / 128
# ki = 1.0
# kd = 0.0
# ki_dig = ki / Fsampling
# kp_dig = kp - ki_dig / 2
# kd_dig = kd * Fsampling

# Use these for only Digital PID
""" 
    Only for PID dig:
    w0 ~= ki_dig * Fsampling / kp_dig
    plateau gain ~= 20 log kp_dig
    w1 ~= kp_dig / (kd_dig * Fsampling) * 10    el 10 no se de donde sale???

"""
ki_dig = 0.00134
kp_dig = 0.02
kd_dig = 0.0

f0 = ki_dig * Fsampling / (kp_dig * 6.28)
if kd_dig != 0:
    f1 = kp_dig * 10 / (kd_dig * Fsampling * 6.28)
else:
    f1 = 'none'
    
print(f"f0 = {f0} f1 = {f1}")

if kp_dig < 0:
    kp_dig = 0
    
k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

#este es el pid
b_pid = [k1, k2, k3]
a_pid = [1, -1]
print ("kp_dig: " + str(kp_dig) + " ki_dig: " + str(ki_dig) + " kd_dig: " + str(kd_dig))
b_cont = b_pid
a_cont = a_pid
controller_tf = TransferFunction(b_cont, a_cont, dt=td)


############################
# SI USO CUSTOM CONTROLLER #
############################
# cont_zeros = [-0.89+0.29j, -0.89-0.29j]
# cont_poles = [0.]
# cont_const = 0.01
# cont_zpk_b, cont_zpk_a = zpk2tf(cont_zeros, cont_poles, cont_const)

# controller_tf = TransferFunction(cont_zpk_b, cont_zpk_a, dt=td)
# print ("Digital Custom Controller:")
# print (controller_tf)

b_cont = controller_tf.num
a_cont = controller_tf.den
print("Controller Parameters:")
print(f"b[2]: {b_cont[0]} b[1]: {b_cont[1]} b[0]: {b_cont[2]}")
print(f"a[1]: {a_cont[0]} a[0]: {a_cont[1]}")


if Bode_Controlador_Digital == True:
    w, mag, phase = dbode(controller_tf, n = 10000)
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag, 'c')
    ax1.set_title('Digital Controller')
    ax1.set_ylabel('Amplitude [dB]', color='c')
    ax1.set_xlabel('Frequency [Hz]')

    ax2.semilogx(w/(2*np.pi), phase, 'c')
    ax2.set_ylabel('Phase', color='c')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()

    
contr_dig = lti_to_sympy(controller_tf)
plant_dig = lti_to_sympy(sensor_dig_zoh)
ol_dig = contr_dig * plant_dig

open_loop_dig = sympy_to_lti(ol_dig)
close_loop_dig = sympy_to_lti(ol_dig/(1+ol_dig))
    
#normalizo con TransferFunction
open_loop_dig = TransferFunction(open_loop_dig.num, open_loop_dig.den, dt=td)    
close_loop_dig = TransferFunction(close_loop_dig.num, close_loop_dig.den, dt=td)


if Bode_Sensor_OpenLoop_CloseLoop_Digital == True:    
    w, mag_ol, phase_ol = dbode(open_loop_dig, n = 10000)
    w, mag_cl, phase_cl = dbode(close_loop_dig, n = 10000)
    
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag_ol, 'b')
    ax1.semilogx(w/(2*np.pi), mag_cl, 'c')    
    ax1.set_title('V Input Blue')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='b')
    ax1.set_xlabel('Frequency [Hz]')
    ax1.set_ylim(ymin=-50, ymax=50)

    ax2.semilogx(w/(2*np.pi), phase_ol, 'b')
    ax2.semilogx(w/(2*np.pi), phase_cl, 'c')    
    ax2.set_ylabel('Phase', color='b')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Polos_Ceros_CloseLoop_Digital == True:
    plot_argand(close_loop_dig)

    
##########################################################
# Verifico Respuesta Escalon Planta Digital Realimentada #
##########################################################
tiempo_de_simulacion = 0.05
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))
tout, yout_zoh = dstep([close_loop_dig.num, close_loop_dig.den, td], t=t)
yout1 = np.transpose(yout_zoh)
yout0 = yout1[0]
yout_zoh = yout0[:tout.size]


if Escalon_CloseLoop_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Realimentada')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    # ax.plot(tout, yout_ef, 'g')
    ax.plot(tout, yout_zoh, 'y')
    # ax.set_ylim(ymin=-20, ymax=100)

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

vout_plant = np.zeros(t.size)
###############################
# Entrada 1: Vinput - Voutput #
###############################
vin_plant = np.zeros(t.size)

fmains = 50
s_sen = np.zeros(t.size)

for i in range(np.size(s_sen)):
    s_sen[i] = np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size))

for i in range (np.size(s_sen)):
    if s_sen[i] < 0:
        s_sen[i] = -s_sen[i]

# vin_plant = 1 - s_sen * 311/350    #si la transferencia incluye los 350V de Vinput
vin_plant = 350 - s_sen * 311    #si la transferencia no tiene 350V de Vinput
# vin_plant = np.ones(t.size)

if Vinput_PtP_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Entrada 1: Vinput')
    ax.set_xlabel('t [s]')
    ax.grid()
    ax.plot(t, vin_plant, 'y')
    plt.tight_layout()
    plt.show()

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
vin_setpoint = s_sen

if Setpoint_PtP_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Setpoint')
    ax.set_xlabel('t [s]')
    ax.grid()
    ax.plot(t, vin_setpoint, 'y')
    plt.tight_layout()
    plt.show()


d = np.zeros(t.size)
error = np.zeros(t.size)
max_d_pwm = 1.0
vin_plant[0:3] = 0
vin_plant_d = np.zeros(t.size)

for i in range(3, len(vin_plant)):
    ###################################################
    # primero calculo el error, siempre punto a punto #
    ###################################################
    error[i] = vin_setpoint[i] - vout_plant[i-1]

    #############################
    # aplico Digital Controller #
    #############################
    d[i] = b_cont[0] * error[i] + b_cont[1] * error[i-1] + b_cont[2] * error[i-2] - a_cont[1] * d[i - 1]

    if d[i] > max_d_pwm:
        d[i] = max_d_pwm

    if d[i] < 0:
        d[i] = 0

    # d[i] = 1.0/350    # trampa para probar estabilidad del sistema

    
    ########################################
    # aplico la transferencia de la planta #
    ########################################
    vin_plant_d[i] = d[i] * vin_plant[i]
    vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                    + b_sensor[1]*vin_plant_d[i-1] \
                    + b_sensor[2]*vin_plant_d[i-2] \
                    + b_sensor[3]*vin_plant_d[i-3] \
                    - a_sensor[1]*vout_plant[i-1] \
                    - a_sensor[2]*vout_plant[i-2] \
                    - a_sensor[3]*vout_plant[i-3]

               
if Respuesta_CloseLoop_All_Inputs_Digital == True:     
    fig, ax = plt.subplots()
    ax.set_title('Respuesta Realimentada punto a punto')
    ax.set_ylabel('Tension en Sensor')
    ax.set_xlabel('Tiempo en muestras')
    ax.grid()
    ax.plot(t, d*100, 'r', label="d x 100")
    ax.plot(t, error, 'g', label="error")
    ax.plot(t, vin_setpoint, 'y', label="sp")
    # ax.stem(t, vout_plant)
    ax.plot(t, vout_plant, 'c', label="out")
    ax.plot(t, vin_plant_d, 'm', label="in")
    # ax.stem(t, vin_plant, 'm', label="in")    
    # ax.set_ylim(ymin=-10, ymax=360)
    ax.legend(loc='upper left')
    plt.tight_layout()
    plt.show()


#######################################
# PID con cambio de ganancia dinamico #
#######################################
# Use these for only Digital PID
""" 
    Only for PID dig:
    w0 ~= ki_dig * Fsampling / kp_dig
    plateau gain ~= 20 log kp_dig
    w1 ~= kp_dig / (kd_dig * Fsampling) * 10    el 10 no se de donde sale???

"""
ki_dig2 = 0.0134
kp_dig2 = 0.02
kd_dig2 = 0.0

k12 = kp_dig2 + ki_dig2 + kd_dig2
k22 = -kp_dig2 - 2*kd_dig2
k32 = kd_dig2

#este es el pid
b_pid2 = [k12, k22, k32]
a_pid2 = [1, -1]
b_cont2 = b_pid2
a_cont2 = a_pid2


d = np.zeros(t.size)
error = np.zeros(t.size)
max_d_pwm = 1.0
min_d_pwm = -0.005
vin_plant[0:3] = 0
vin_plant_d = np.zeros(t.size)

for i in range(3, len(vin_plant)):
    ###################################################
    # primero calculo el error, siempre punto a punto #
    ###################################################
    error[i] = vin_setpoint[i] - vout_plant[i-1]

    #############################################################
    # aplico Digital Controller dinamico en funcion de setpoint #
    #############################################################
    if i > 210:
        # puedo aplicar tensiones negativas chicas
        d[i] = b_cont[0] * error[i] + b_cont[1] * error[i-1] + b_cont[2] * error[i-2] - a_cont[1] * d[i - 1]
        if d[i] > max_d_pwm:
            d[i] = max_d_pwm

        if d[i] < 0:
            if d[i] < min_d_pwm:
                d[i] = min_d_pwm
    else:
        if vin_setpoint[i] < 0.5:
            d[i] = b_cont[0] * error[i] + b_cont[1] * error[i-1] + b_cont[2] * error[i-2] - a_cont[1] * d[i - 1]
        else:
            d[i] = b_cont2[0] * error[i] + b_cont2[1] * error[i-1] + b_cont2[2] * error[i-2] - a_cont2[1] * d[i - 1]

        if d[i] > max_d_pwm:
            d[i] = max_d_pwm

        if d[i] < 0:
            d[i] = 0

    
    ########################################
    # aplico la transferencia de la planta #
    ########################################
    vin_plant_d[i] = d[i] * vin_plant[i]
    vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                    + b_sensor[1]*vin_plant_d[i-1] \
                    + b_sensor[2]*vin_plant_d[i-2] \
                    + b_sensor[3]*vin_plant_d[i-3] \
                    - a_sensor[1]*vout_plant[i-1] \
                    - a_sensor[2]*vout_plant[i-2] \
                    - a_sensor[3]*vout_plant[i-3]

               
if Respuesta_CloseLoop_All_Inputs_Digital == True:     
    fig, ax = plt.subplots()
    ax.set_title('Respuesta Realimentada punto a punto')
    ax.set_ylabel('Tension en Sensor')
    ax.set_xlabel('Tiempo en muestras')
    ax.grid()
    ax.plot(t, d*100, 'r', label="d x 100")
    ax.plot(t, error, 'g', label="error")
    ax.plot(t, vin_setpoint, 'y', label="sp")
    # ax.stem(t, vout_plant)
    ax.plot(t, vout_plant, 'c', label="out")
    ax.plot(t, vin_plant_d, 'm', label="in")
    # ax.stem(t, vin_plant, 'm', label="in")    
    # ax.set_ylim(ymin=-10, ymax=360)
    ax.legend(loc='upper left')
    plt.tight_layout()
    plt.show()

