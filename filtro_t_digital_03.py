# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy
from pid_tf import PID_float

"""
        ESTE ARCHIVO USA UN CONTROL PID ENTRE CICLOS DE 100HZ - utiliza undersampling -
        MicroInversor - Etapa de salida de la parte Inverter
	Etapa C.
        Funcion: Generador de senial senoidal
        Modo: Voltage Mode - VM
        Feedback: por tension de salida, sensada a traves del Rsense
"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Planta_Sensor_Analog = False
Escalon_Sensor_Analog = False
Bode_Controlador_Analog = False
Bode_Sensor_OpenLoop_CloseLoop_Analog = False
Escalon_CloseLoop_Analog = False
Respuesta_CloseLoop_All_Inputs_Analog = False
Bode_Sensor_Digital = False
Escalon_Sensor_Digital = False
Escalon_Sensor_Digital_Recursivo = False
Pre_Distorted_PtP_Digital_Signals = False
Respuesta_CloseLoop_All_Inputs_Digital = True

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
    ax1.set_title('Magnitude')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()

#####################################
# Desde aca hago pruebas temporales #
#####################################
tiempo_de_simulacion = 0.01
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
ki_analog = 31.4     # cero en 1000Hz
kd_analog = 75e-9      # segundo cero en 10600Hz

#3.3, 192, 0.00086 puede ir
#1, 2300, 0 por simulacion puede ir
#4.2; 100; 0.00086 ajusta bien la bajada
#3.2; 200; 0.00086 ajusta bastante bien

controller = kp_analog + ki_analog/s + s*kd_analog
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

w, mag_ef, phase_ef = dbode(sensor_dig_ef, n = 10000)
w, mag_zoh, phase_zoh = dbode(sensor_dig_zoh, n = 10000)

if Bode_Sensor_Digital == True:
    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_ef, 'g')
    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')    
    ax1.set_title('Digital Euler-Forward Green, ZOH Yellow')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='g')
    ax1.set_xlabel('Frequency [Hz]')
    ax1.set_ylim(ymin=-40, ymax=40)

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

#############################
# SISTEMA PRE-DISTORSIONADO #
#############################
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
vin_setpoint = vin_setpoint.astype('int16')

pre_distorted_duty = vin_setpoint / vin_plant

if Pre_Distorted_PtP_Digital_Signals == True:
    fig, ax = plt.subplots()
    ax.set_title('Setpoint')
    ax.set_xlabel('t [s]')
    ax.grid()
    ax.plot(t, vin_plant, 'g')    
    ax.plot(t, vin_setpoint, 'y')
    ax.plot(t, pre_distorted_duty, 'r')
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
loops = 200
t_looped = np.linspace(0, tiempo_de_simulacion * loops, num=(tiempo_de_simulacion*Fsampling*loops))

#armo un nuevo input
vin_plant_looped = np.zeros(t_looped.size)
vin_setpoint_looped = np.zeros(t_looped.size)
vout_plant_looped = np.zeros(t_looped.size)
vin_plant_d_looped = np.zeros(t_looped.size)
d_looped = np.zeros(t_looped.size)
sensor_looped = np.zeros(t_looped.size)
error_looped = np.zeros(t_looped.size)

""" 
    Only for PI dig:
    w0 ~= ki_dig / kp_dig * Fsampling
    plateau gain ~= 20 log kp_dig

"""
ki_dig = 1 / 128
kp_dig = 0 / 128    #1.28 / 128
kd_dig = 0.0

k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

#este es el pid
b_pid = [k1, k2, k3]
a_pid = [1, -1]

vout_plant = np.zeros(t.size)
d = np.zeros(t.size)
vin_plant[0:3] = 0
vin_plant_d = vin_plant * 1.0    #fuerzo que cree un nuevo vector

sensor_adc = np.zeros(t.size)
pid_list = []
undersampling = 23
under = undersampling

for i in range(t.size):
    pid_list.append(PID_float(b_pid, a_pid))

for j in range(loops):

    # Veo el error que tuve
    if j > 0:
        for i in range(sensor_adc.size):
            sensor_adc[i] = sensor_looped[(j-1)*t.size + i]
    
    error = vin_setpoint - sensor_adc

    ###############################
    # PID y limite del duty cycle #
    ###############################
    if j > 0:
        for i in range(t.size):
            if under:
                under -= 1
                d[i] = d[i-1]
            else:
                under = undersampling

                d[i] = pid_list[i].newOutput(error[i])
                if i < 210:
                    if d[i] < 0:
                        d[i] = 0

                    if d[i] > 1000:
                        d[i] = 1000

                
    ########################################
    # aplico la transferencia de la planta #
    ########################################
    for i in range(3, len(vin_plant)):
        vin_plant_d[i] = d[i]/1000 * vin_plant[i]
        vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                        + b_sensor[1]*vin_plant_d[i-1] \
                        + b_sensor[2]*vin_plant_d[i-2] \
                        + b_sensor[3]*vin_plant_d[i-3] \
                        - a_sensor[1]*vout_plant[i-1] \
                        - a_sensor[2]*vout_plant[i-2] \
                        - a_sensor[3]*vout_plant[i-3]


        sensor_adc = vout_plant / 3.3 * 1023
        sensor_adc = np.array(sensor_adc, dtype=np.int16)
        for i in range(sensor_adc.size):
            if (sensor_adc[i] < 0):
                sensor_adc[i] = 0

            if (sensor_adc[i] > 1023):
                sensor_adc[i] = 1023


        
    # guardo valores al vector general
    for i in range (t.size):
        vin_plant_looped[i+j*t.size] = vin_plant[i]
        vin_setpoint_looped[i+j*t.size] = vin_setpoint[i]
        vout_plant_looped[i+j*t.size] = vout_plant[i]
        sensor_looped[i+j*t.size] = sensor_adc[i]        
        vin_plant_d_looped[i+j*t.size] = vin_plant_d[i]
        d_looped[i+j*t.size] = d[i]
        error_looped[i+j*t.size] = error[i]


if Respuesta_CloseLoop_All_Inputs_Digital == True:     
    fig, ax = plt.subplots()
    ax.set_title(f'Resultados en {loops} loops')
    ax.set_ylabel('Tension en Sensor')
    ax.set_xlabel('Tiempo en muestras')
    ax.grid()
    ax.plot(t_looped, d_looped, 'r', label="d")
    ax.plot(t_looped, vin_setpoint_looped, 'y', label="sp")
    # ax.stem(t, vout_plant)
    ax.plot(t_looped, vout_plant_looped, 'c', label="out")
    ax.plot(t_looped, sensor_looped, 'g', label="sensor")    
    ax.plot(t_looped, vin_plant_d_looped, 'm', label="in")
    ax.plot(t_looped, error_looped, 'k', label="error")    
    # ax.plot(t, vin_plant, 'm', label="in")    
    # ax.stem(t, vin_plant, 'm', label="in")    
    # ax.set_ylim(ymin=-10, ymax=360)
    ax.legend(loc='upper left')
    plt.tight_layout()
    plt.show()




# #####################
# # Con undersampling #
# #####################
# if undersampling != 0:
#     max_d_pwm = 1.0
#     under_index = 0
#     for i in range(2, len(vin_plant)):
#         ###################################################
#         # primero calculo el error, siempre punto a punto #
#         ###################################################
#         error[i] = vin_setpoint[i] - vout_plant[i-1]

#         ###################
#         # aplico lazo PID #
#         ###################
#         if under_index < undersampling:
#             #nada
#             under_index = under_index + 1
#             d[i] = d[i-1]
#         else:
#             under_index = 0
#             d[i] = b_pid[0] * error[i] + b_pid[1] * error[i-1] + b_pid[2] * error[i-2] - a_pid[1] * d[i-1]

#         if d[i] > max_d_pwm:
#             d[i] = max_d_pwm

#         if d[i] < 0:
#             d[i] = 0

    
#         ########################################
#         # aplico la transferencia de la planta #
#         ########################################
#         vin_plant[i] = d[i]
#         vout_plant[i] = b_sensor[0]*vin_plant[i] \
#                         + b_sensor[1]*vin_plant[i-1] \
#                         + b_sensor[2]*vin_plant[i-2] \
#                         - a_sensor[1]*vout_plant[i-1] \
#                         - a_sensor[2]*vout_plant[i-2]


# #####################
# # Sin undersampling #
# #####################
# if undersampling == 0:
#     max_d_pwm = 1.0

#     for i in range(2, len(vin_plant)):
#         ###################################################
#         # primero calculo el error, siempre punto a punto #
#         ###################################################
#         error[i] = vin_setpoint[i] - vout_plant[i-1]

#         ###################
#         # aplico lazo PID #
#         ###################
#         d[i] = b_pid[0] * error[i] + b_pid[1] * error[i-1] + b_pid[2] * error[i-2] - a_pid[1] * d[i-1]

#         if d[i] > max_d_pwm:
#             d[i] = max_d_pwm

#         if d[i] < 0:
#             d[i] = 0

    
#         ########################################
#         # aplico la transferencia de la planta #
#         ########################################
#         vin_plant[i] = d[i]
#         vout_plant[i] = b_sensor[0]*vin_plant[i] \
#                         + b_sensor[1]*vin_plant[i-1] \
#                         + b_sensor[2]*vin_plant[i-2] \
#                         - a_sensor[1]*vout_plant[i-1] \
#                         - a_sensor[2]*vout_plant[i-2]

               
# if Respuesta_CloseLoop_All_Inputs_Digital == True:     
#     fig, ax = plt.subplots()
#     ax.set_title('Respuesta Realimentada punto a punto')
#     ax.set_ylabel('Tension en Sensor')
#     ax.set_xlabel('Tiempo en muestras')
#     ax.grid()
#     ax.plot(t, d, 'r', label="d")
#     ax.plot(t, error, 'g', label="error")
#     ax.plot(t, vin_setpoint, 'y', label="sp")
#     # ax.stem(t, vout_plant)
#     ax.plot(t, vout_plant, 'c', label="out")
#     ax.legend(loc='upper left')
#     plt.tight_layout()
#     plt.show()
