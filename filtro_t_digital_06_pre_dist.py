# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy
from pid_tf import PID_float, PID_int
from recursive_tf import RecursiveTF

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
Bode_Planta_Sensor_Digital = False
Escalon_Planta_Sensor_Digital = False
Escalon_Planta_Sensor_Digital_Recursivo = False
Vin_Setpoint_PtP_Digital_Signals = False
Respuesta_PreDistorted_All_Inputs_Digital = True

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

# desde Vinput (sin Vinput) al sensor de corriente
Rload = 21.0
# Rload = 6.0
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

# Sensor_out = Sensor_in_sim / 3.3 * 4095    #con adc
Sensor_out = Sensor_in_sim    #sin adc
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


if Bode_Planta_Sensor_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(planta_TF, wfreq)
    w, mag_s, phase_s = bode(sensor_TF, wfreq)
    
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'g-', linewidth="1")
    ax1.semilogx (w/6.28, mag_s, 'b-', linewidth="1")
    ax1.set_title('Analog TF -- Planta green, Sensor blue')
    ax1.set_ylabel('Magnitude')

    ax2.semilogx (w/6.28, phase_p, 'g-', linewidth="1")
    ax2.semilogx (w/6.28, phase_s, 'b-', linewidth="1")
    ax2.set_ylabel('Phase')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()

###############################################################################################
# Convierto Sensor por Euler-Forward y zoh a una frecuencia mucho mas alta que la de muestreo #
# para que no afecte polos o ceros                                                            #            
###############################################################################################
Fsampling = 24000
Tsampling = 1 / Fsampling
    
planta_dig_zoh_n, planta_dig_zoh_d, td = cont2discrete((planta_TF.num, planta_TF.den), Tsampling, method='zoh')
sensor_dig_zoh_n, sensor_dig_zoh_d, td = cont2discrete((sensor_TF.num, sensor_TF.den), Tsampling, method='zoh')

#normalizo con TransferFunction
print ("Planta Digital Zoh:")
planta_dig_zoh = TransferFunction(planta_dig_zoh_n, planta_dig_zoh_d, dt=td)
print (planta_dig_zoh)

print ("Sensor Digital Zoh:")
sensor_dig_zoh = TransferFunction(sensor_dig_zoh_n, sensor_dig_zoh_d, dt=td)
print (sensor_dig_zoh)



if Bode_Planta_Sensor_Digital == True:
    w, mag_p_zoh, phase_p_zoh = dbode(planta_dig_zoh, n = 10000)
    w, mag_s_zoh, phase_s_zoh = dbode(sensor_dig_zoh, n = 10000)
    
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag_p_zoh, 'g')
    ax1.semilogx(w/(2*np.pi), mag_s_zoh, 'b')        
    ax1.set_title('Digital ZOH TF -- Planta green, Sensor blue')
    ax1.set_ylabel('Magnitude')
    # ax1.set_xlabel('Frequency [Hz]')
    # ax1.set_ylim(ymin=-40, ymax=40)

    ax2.semilogx(w/(2*np.pi), phase_p_zoh, 'g')
    ax2.semilogx(w/(2*np.pi), phase_s_zoh, 'b')        
    ax2.set_ylabel('Phase')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()

#############################################
# Verifico Respuesta Escalon Planta Digital #
#############################################
tiempo_de_simulacion = 0.01
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))

if Escalon_Planta_Sensor_Digital == True:
    tout, yout_zoh = dstep([planta_dig_zoh.num, planta_dig_zoh.den, td], t=t)
    yout1 = np.transpose(yout_zoh)
    yout0 = yout1[0]
    yout_p_zoh = yout0[:tout.size]
    
    tout, yout_zoh = dstep([sensor_dig_zoh.num, sensor_dig_zoh.den, td], t=t)
    yout1 = np.transpose(yout_zoh)
    yout0 = yout1[0]
    yout_s_zoh = yout0[:tout.size]
    
    fig, ax = plt.subplots()
    ax.set_title('Step Digital ZOH -- Planta green, Sensor blue')
    ax.set_ylabel('Tension')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(tout, yout_p_zoh, 'g')
    ax.plot(tout, yout_s_zoh, 'b')    
    # ax.set_ylim(ymin=-20, ymax=100)

    plt.tight_layout()
    plt.show()

############################################################
# Verifico Respuesta Escalon Planta Convertida a Recursiva #
############################################################
# t = np.linspace(0, tiempo_de_simulacion, num=int(tiempo_de_simulacion/Fsampling))

if Escalon_Planta_Sensor_Digital_Recursivo == True:

    # ZOH Planta
    b_planta = np.transpose(planta_dig_zoh_n)
    a_planta = np.transpose(planta_dig_zoh_d)
    recur_planta = RecursiveTF(b_planta, a_planta)

    # ZOH Sensor
    b_sensor = np.transpose(sensor_dig_zoh_n)
    a_sensor = np.transpose(sensor_dig_zoh_d)
    recur_sensor = RecursiveTF(b_sensor, a_sensor)    

    vin_plant = np.ones(t.size)
    vout_plant = np.zeros(t.size)
    for i in range(t.size):
        vout_plant[i] = recur_planta.newOutput(vin_plant[i])

    vin_sensor = np.ones(t.size)        
    vout_sensor = np.zeros(t.size)
    for i in range(t.size):
        vout_sensor[i] = recur_sensor.newOutput(vin_sensor[i])
        
    fig, ax = plt.subplots()
    ax.set_title('Step Digital Recursiva ZOH -- Planta green Sensor blue')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant, 'g')
    ax.plot(t, vout_sensor, 'b')    
    plt.tight_layout()
    plt.show()

###############################
# Entrada 1: Vinput - Voutput #
###############################
fmains = 50
# peak_220 = 300
peak_220 = 311

# voltage_rectified = 350
voltage_rectified = 390

s_sen = np.zeros(t.size)

for i in range(np.size(s_sen)):
    s_sen[i] = np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size))

for i in range (np.size(s_sen)):
    if s_sen[i] < 0:
        s_sen[i] = -s_sen[i]

vin_plant = np.ones(t.size) * voltage_rectified
vout_line = s_sen * peak_220
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

adc_ref_top = 1871
# adc_ref_top = int(1871 * 1.1)
# adc_ref_top = 2792
# adc_ref_top = 3722
## 1Apk * 0.33 * 9.2 = 3Vpk
peak_isense = adc_ref_top / 4095 * 3.3
peak_current = peak_isense / 3
print (f"peak current: {peak_current:.2f}A peak voltage on Rsense: {peak_isense:.2f}V") 
vin_setpoint = s_sen * adc_ref_top
vin_setpoint = vin_setpoint.astype('int16')
print (vin_setpoint)

if Vin_Setpoint_PtP_Digital_Signals == True:
    fig, ax = plt.subplots()
    ax.set_title('Vin Plant green, Setpoint [i table] yellow')
    ax.set_xlabel('t [s]')
    ax.grid()
    ax.plot(t, vin_plant, 'g')    
    ax.plot(t, vin_setpoint, 'y')
    plt.tight_layout()
    plt.show()




##############################
# Pre-Distorted Duty-Cycle d #
##############################
d = np.zeros(t.size)

d = [ 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330,
      580, 580, 580, 580, 580, 580, 580, 580, 580, 580, 580, 580,
      830, 830, 830, 830, 830, 830, 830, 830, 830, 830, 830, 830,      
      
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,
      1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240,
      1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400,
      1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530,

      1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600,
      1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650,
      1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650,
      1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600, 1600,

      1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530, 1530,
      1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400,
      1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240, 1240,
      1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050, 1050,

      830, 830, 830, 830, 830, 830, 830, 830, 830, 830, 830, 830,
      580, 580, 580, 580, 580, 580, 580, 580, 580, 580, 580, 580,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,]


# ZOH Planta
b_planta = np.transpose(planta_dig_zoh_n)
a_planta = np.transpose(planta_dig_zoh_d)
recur_planta = RecursiveTF(b_planta, a_planta)

# ZOH Sensor
b_sensor = np.transpose(sensor_dig_zoh_n)
a_sensor = np.transpose(sensor_dig_zoh_d)
recur_sensor = RecursiveTF(b_sensor, a_sensor)    

vin_plant_d = np.zeros(t.size)
vout_plant = np.zeros(t.size)
vout_sensor = np.zeros(t.size)
vout_sensor_adc = np.zeros(t.size, dtype='int16')

for i in range(len(vin_plant)):
    ######################################
    # aplico pre-distorted d a la planta #
    ######################################

    # aplico el pre-distorted d a la planta
    vin_plant_d[i] = d[i]/2000 * vin_plant[i]

    # compenso la tension de salida que se opone #
    if vin_plant_d[i] > vout_line[i]:
        vin_plant_d[i] = vin_plant_d[i] - vout_line[i]
    else:
        vin_plant_d[i] = 0



    vout_plant[i] = recur_planta.newOutput(vin_plant_d[i])
    vout_sensor[i] = recur_sensor.newOutput(vin_plant_d[i])
    vout_sensor_adc[i] = Adc12Bits (vout_sensor[i])
    

if Respuesta_PreDistorted_All_Inputs_Digital == True:     
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.set_title(f'adc_ref_top={adc_ref_top} Ipk={peak_current:.2f}A voltage rectified: {voltage_rectified}V peak_220={peak_220}V')
    ax1.grid()
    ax1.plot(t, vin_setpoint, 'r', label="sp")
    ax1.plot(t, vout_sensor_adc, 'y', label="out_adc")
    # ax1.plot(t, error, 'g', label="error")
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
    
