# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand
from pid_tf import PID_int
from recursive_tf import RecursiveTF

"""
        MicroInversor - Etapa de salida de la parte Inverter
	Etapa C.
        Funcion: Transferencia digital desde vinput al ADC del micro 12bits
        Trato de igualar lo que sucede con tests.c
        Entradas y salidas de la planta son float, duty y pid discrete

        Igual a filtro_t_digital_07 pero agrego sensado a traves de un filtro
        pasabajos (polo) y el opamp (polo y cero)

"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Planta_Analog = False
Bode_Polo_Opamp_Analog = False
Bode_Sensor_Analog = False

Bode_Sensor_Digital = False
Polos_Ceros_Digital = False
Escalon_Sensor_Digital = False
Escalon_Sensor_Digital_Recursivo = False
Bode_Controlador_Digital = False
Bode_Sensor_OpenLoop_CloseLoop_Digital = False
Polos_Ceros_CloseLoop_Digital = False
Escalon_CloseLoop_Digital = False
Vinput_PtP_Digital = False
Setpoint_PtP_Digital = False
PID_Multiple_Stages = False
PID_Multiple_Stages_3 = False
PID_Multiple_Stages_5 = False
PID_Multiple_Stages_Underdampling = False
PID_Single_Stage = True
PID_Single_Stage_Udersamplig = False
Respuesta_CloseLoop_All_Inputs_Digital = True

def Adc12Bits (sample):
    adc = np.int16(0)
    sample = sample / 3.3
    sample = sample * 4095
    if sample < 0.0:
        sample = 0

    if sample > 4095:
        sample = 4095

    adc = sample
    return adc


#TF without constant
s = Symbol('s')

# FILTRO T
# desde Vinput (sin Vinput) al sensor de corriente
Rload = 11.0
# Rload = 2.0
Rsense = 0.33
L1 = 1.8e-3
L2 = 1.8e-3
C = 0.44e-6

# FILTRO DEL SENSOR (polo y opamp - polo y cero -)
Rp = 1800
Cp = 56e-9

Ri = 1000
Rf = 8200
Cf = 22e-9

Amp_gain = 9.2    #no se usa se saca como (Ri + Rf)/Ri


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
print ('Planta sin amplificar ni filtrar: ')
print (Plant_out_sim)

## Polo
transf_polo = 1/(1 + s*Cp*Rp)
transf_polo_sim = transf_polo.simplify()
print ('Polo en Rsense: ')
print (transf_polo_sim)

## Opamp
transf_opamp_gain = (Ri + Rf)/Ri
transf_opamp_pole = 1 + s*Cf*Rf
transf_opamp_zero = 1 + s*Cf*Rf*Ri/(Rf+Ri)
transf_opamp = transf_opamp_gain * transf_opamp_zero / transf_opamp_pole
transf_opamp_sim = transf_opamp.simplify()
print ('Opamp en Rsense: ')
print (transf_opamp_sim)

## Polo + Opamp
transf_polo_opamp = transf_polo * transf_opamp
transf_polo_opamp_sim = transf_polo_opamp.simplify()
print ('Polo y Opamp en Rsense: ')
print (transf_polo_opamp_sim)


Sensor_in = Plant_out_sim * transf_polo_opamp_sim
Sensor_in_sim = Sensor_in.simplify()

print ('Entrada del ADC: ')
print (Sensor_in_sim)

# Sensor_out = Sensor_in_sim / 3.3 * 4095    #con adc
Sensor_out = Sensor_in_sim    #sin adc
Sensor_out_sim = Sensor_out.simplify()

print ('Salida del ADC: ')
print (Sensor_out_sim)


################################################
# Algunas confirmaciones de la parte Analogica #
################################################
planta_TF = sympy_to_lti(Plant_out_sim)

if Bode_Planta_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(planta_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    # ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title('Magnitude Input to Output Plant')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    # ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()


filters_TF = sympy_to_lti(transf_polo_opamp_sim)
if Bode_Polo_Opamp_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(filters_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'y-', linewidth="1")
    # ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title('Magnitude Only Sensor')

    ax2.semilogx (w/6.28, phase_p, 'y-', linewidth="1")
    # ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()


sensor_TF = sympy_to_lti(Sensor_in_sim)
if Bode_Sensor_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(sensor_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'g-', linewidth="1")
    # ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title('Magnitude Input to Output Sensor')

    ax2.semilogx (w/6.28, phase_p, 'g-', linewidth="1")
    # ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()
    


########################################################
# Convierto Planta por zoh a la frecuencia de muestreo #
# para que no afecte polos o ceros                     #
########################################################
Fsampling = 24000

sensor_TF = sympy_to_lti(Sensor_out_sim)
print ("sensor con sympy:")
print (sensor_TF)

Fsampling_mult = Fsampling
Tsampling_mult = 1 / Fsampling_mult
    
sensor_dig_zoh_n, sensor_dig_zoh_d, td = cont2discrete((sensor_TF.num, sensor_TF.den), Tsampling_mult, method='zoh')

#normalizo con TransferFunction
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

if Bode_Sensor_Digital == True:
    w, mag_zoh, phase_zoh = dbode(sensor_dig_zoh, n = 10000)

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')    
    ax1.set_title('Digital Bode ZOH')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='y')
    ax1.set_xlabel('Frequency [Hz]')
    # ax1.set_ylim(ymin=-40, ymax=40)

    ax2.semilogx(w/(2*np.pi), phase_zoh, 'y')    
    ax2.set_ylabel('Phase', color='y')
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
tout, yout_zoh = dstep([sensor_dig_zoh.num, sensor_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout_zoh)
yout0 = yout1[0]
yout_zoh = yout0[:tout.size]


if Escalon_Sensor_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital ZOH')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()

    ax.plot(tout, yout_zoh, 'y')
    # ax.set_ylim(ymin=-20, ymax=100)

    plt.tight_layout()
    plt.show()


############################################################
# Verifico Respuesta Escalon Planta Convertida a Recursiva #
############################################################
if Escalon_Sensor_Digital_Recursivo == True:
    # ZOH
    b_sensor = np.transpose(sensor_dig_zoh.num)
    a_sensor = np.transpose(sensor_dig_zoh.den)

    vin_plant = np.ones(t.size)
    vout_plant_method2 = np.zeros (t.size)    
    recur_sensor = RecursiveTF(b_sensor, a_sensor)
    for i in range(t.size):
        vout_plant_method2[i] = recur_sensor.newOutput(vin_plant[i])
    

    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Recursiva ZOH')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant_method2, 'y')    
    plt.tight_layout()
    plt.show()
    

########################
# Ecuacion PID Digital #
########################
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

# peak_220 = 265
# peak_220 = 285
peak_220 = 311
vin_plant = 380 - s_sen * peak_220    #si la transferencia no tiene 350V de Vinput
# vin_plant = 331 - s_sen * peak_220    #si se cae la tension de entrada
# vin_plant = 315 - s_sen * peak_220    #si se cae la tension de entrada
# vin_plant = np.ones(t.size) * 350    #solo la entrada de dc
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

# adc_ref_top = 1861
adc_ref_top = 1871
# adc_ref_top = 2792
# adc_ref_top = 3722
## 1Apk * 0.33 * 9.2 = 3Vpk
peak_isense = adc_ref_top / 4095 * 3.3
peak_current = peak_isense / 3
print (f"peak current: {peak_current:.2f}A peak voltage on Rsense: {peak_isense:.2f}V") 
vin_setpoint = s_sen * adc_ref_top
vin_setpoint = vin_setpoint.astype('int16')
print (vin_setpoint)

if Setpoint_PtP_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Setpoint')
    ax.set_xlabel('t [s]')
    ax.grid()
    ax.plot(t, vin_setpoint, 'y')
    plt.tight_layout()
    plt.show()


d = np.zeros(t.size, dtype='int16')
error = np.zeros(t.size, dtype='int16')
vin_plant[0:3] = 0
vin_plant_d = np.zeros(t.size)
vout_plant_adc = np.zeros(t.size, dtype='int16')


# kp_dig = 24    #primera parte
# ki_dig = 5
# kd_dig = 0.0

kp_dig = 10    #primera parte
ki_dig = 3
kd_dig = 0

k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

b_pid_int = [k1, k2, k3]
a_pid_int = [1, -1]
pid_tf_int = PID_int(b_pid_int, a_pid_int, 128)


if PID_Multiple_Stages == True:
    max_pwm_pts = 2000
    done = 0
    for i in range(3, len(vin_plant)):
        ###################################################
        # primero calculo el error, siempre punto a punto #
        ###################################################
        error[i] = vin_setpoint[i] - vout_plant_adc[i-1]

        #############################
        # aplico Digital Controller #
        #############################
        if i >= 0 and done == 0:
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 0')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 1

        elif i >= 48 and done == 1:
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 1')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 2
            
        elif i >= 96 and done == 2:
            kp_dig = 128
            ki_dig = 1
            kd_dig = 0
            print('Stage 2')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 3

        elif i >= 120 and done == 3:
            kp_dig = 64
            ki_dig = 1
            kd_dig = 0
            print('Stage 3')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 4

        elif i >= 144 and done == 4:
            #cambio parametros pid
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 4')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 5
        
        elif i >= 192 and done == 5:
            #cambio parametros pid
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 5')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 6

        
        d[i] = pid_tf_int.newOutput(error[i])

        if d[i] < 0:
            d[i] = 0

        if d[i] > max_pwm_pts:
            d[i] = max_pwm_pts

        # si no estoy en la etapa 6 no permito descargas aceleradas
        if done != 6:
            if d[i] < 0:
                d[i] = 0

        ########################################
        # aplico la transferencia de la planta #
        ########################################
        vin_plant_d[i] = d[i] * vin_plant[i] / max_pwm_pts
        vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                        + b_sensor[1]*vin_plant_d[i-1] \
                        + b_sensor[2]*vin_plant_d[i-2] \
                        + b_sensor[3]*vin_plant_d[i-3] \
                        - a_sensor[1]*vout_plant[i-1] \
                        - a_sensor[2]*vout_plant[i-2] \
                        - a_sensor[3]*vout_plant[i-3]

        vout_plant_adc[i] = Adc12Bits (vout_plant[i])


if PID_Multiple_Stages_3 == True:
    max_pwm_pts = 2000
    done = 0

    recur_s3 = RecursiveTF(b_sensor, a_sensor)
    for i in range(1, len(vin_plant)):
        ###################################################
        # primero calculo el error, siempre punto a punto #
        ###################################################
        error[i] = vin_setpoint[i] - vout_plant_adc[i-1]

        #############################
        # aplico Digital Controller #
        #############################
        if i >= 0 and done == 0:
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 0')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 1

        elif i >= 60 and done == 1:
            kp_dig = 50
            ki_dig = 0
            kd_dig = 0
            print('Stage 1')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 2
            
        elif i >= 180 and done == 2:
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 2')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 3
        
        elif i >= 204 and done == 3:
            done = 4

        
        d[i] = pid_tf_int.newOutput(error[i])

        if d[i] < 0:
            d[i] = 0
            pid_tf_int.resetFilter()

        if d[i] > max_pwm_pts:
            d[i] = max_pwm_pts

        # si no estoy en la etapa 6 no permito descargas aceleradas
        if done == 4:
            d[i] = 0

        ########################################
        # aplico la transferencia de la planta #
        ########################################
        vin_plant_d[i] = d[i] * vin_plant[i] / max_pwm_pts

        vout_plant[i] = recur_s3.newOutput(vin_plant_d[i])
        # vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
        #                 + b_sensor[1]*vin_plant_d[i-1] \
        #                 + b_sensor[2]*vin_plant_d[i-2] \
        #                 + b_sensor[3]*vin_plant_d[i-3] \
        #                 - a_sensor[1]*vout_plant[i-1] \
        #                 - a_sensor[2]*vout_plant[i-2] \
        #                 - a_sensor[3]*vout_plant[i-3]

        vout_plant_adc[i] = Adc12Bits (vout_plant[i])
        

if PID_Multiple_Stages_5 == True:
    max_pwm_pts = 2000
    done = 0
    for i in range(3, len(vin_plant)):
        ###################################################
        # primero calculo el error, siempre punto a punto #
        ###################################################
        error[i] = vin_setpoint[i] - vout_plant_adc[i-1]

        #############################
        # aplico Digital Controller #
        #############################
        if i >= 0 and done == 0:
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 0')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 1

        elif i >= 48 and done == 1:
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 1')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 2
            
        elif i >= 96 and done == 2:
            kp_dig = 64
            ki_dig = 2
            kd_dig = 0
            print('Stage 2')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 3

        elif i >= 144 and done == 3:
            kp_dig = 32
            ki_dig = 3
            kd_dig = 0
            print('Stage 3')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 4

        elif i >= 192 and done == 4:
            #cambio parametros pid
            kp_dig = 10
            ki_dig = 3
            kd_dig = 0
            print('Stage 4')
            pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
            pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)
            done = 5
        
        
        d[i] = pid_tf_int.newOutput(error[i])

        if d[i] > max_pwm_pts:
            d[i] = max_pwm_pts

        if done != 5:
            if d[i] < 0:
                d[i] = 0


        ########################################
        # aplico la transferencia de la planta #
        ########################################
        vin_plant_d[i] = d[i] * vin_plant[i] / max_pwm_pts
        vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
                        + b_sensor[1]*vin_plant_d[i-1] \
                        + b_sensor[2]*vin_plant_d[i-2] \
                        + b_sensor[3]*vin_plant_d[i-3] \
                        - a_sensor[1]*vout_plant[i-1] \
                        - a_sensor[2]*vout_plant[i-2] \
                        - a_sensor[3]*vout_plant[i-3]

        vout_plant_adc[i] = Adc12Bits (vout_plant[i])
        


if PID_Single_Stage == True:
    kp_dig = 10
    ki_dig = 3
    kd_dig = 0
    max_pwm_pts = 2000
    print('Stage 0')
    print(f'pwm points: {max_pwm_pts}')
    pid_tf_int.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)
    pid_tf_int.changeParamsFromK(kp_dig, ki_dig, kd_dig)

    recur_s3 = RecursiveTF(b_sensor, a_sensor)
    for i in range(3, len(vin_plant)):
        ###################################################
        # primero calculo el error, siempre punto a punto #
        ###################################################
        error[i] = vin_setpoint[i] - vout_plant_adc[i-1]

        #############################
        # aplico Digital Controller #
        #############################
        d[i] = pid_tf_int.newOutput(error[i])

        if d[i] < 0:
            d[i] = 0
            pid_tf_int.resetFilter()

        if d[i] > max_pwm_pts:
            d[i] = max_pwm_pts
            
        ########################################
        # aplico la transferencia de la planta #
        ########################################
        vin_plant_d[i] = d[i] * vin_plant[i] / max_pwm_pts
        
        vout_plant[i] = recur_s3.newOutput(vin_plant_d[i])
        
        # vout_plant[i] = b_sensor[0]*vin_plant_d[i] \
        #                 + b_sensor[1]*vin_plant_d[i-1] \
        #                 + b_sensor[2]*vin_plant_d[i-2] \
        #                 + b_sensor[3]*vin_plant_d[i-3] \
        #                 - a_sensor[1]*vout_plant[i-1] \
        #                 - a_sensor[2]*vout_plant[i-2] \
        #                 - a_sensor[3]*vout_plant[i-3]

        vout_plant_adc[i] = Adc12Bits (vout_plant[i])


        
               
if Respuesta_CloseLoop_All_Inputs_Digital == True:     
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.set_title(f'adc_ref_top={adc_ref_top} Ipk={peak_current:.2f} peak_220={peak_220}')
    ax1.grid()
    ax1.plot(t, vin_setpoint, 'r', label="sp")
    ax1.plot(t, vout_plant_adc, 'y', label="out_adc")
    ax1.plot(t, error, 'g', label="error")
    ax1.plot(t, d, 'm', label="duty")
    ax1.legend(loc='upper left')

    # ax2.plot(t, vin_setpoint, 'y', label="sp")
    # ax.stem(t, vout_plant)
    ax2.plot(t, vout_plant, 'y', label="out")
    ax2.plot(t, vin_plant_d, 'm', label="in")

    # ax.set_ylim(ymin=-10, ymax=360)
    ax2.legend(loc='upper left')
    plt.tight_layout()
    plt.show()

print (d)
# print (error)
    


