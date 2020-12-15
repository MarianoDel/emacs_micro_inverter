# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand
from pid_tf import PID_int
from custom_tf import Custom_int
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

Bode_Sensor_Digital = True
Polos_Ceros_Digital = False
Escalon_Sensor_Digital = True
Escalon_Sensor_Digital_Recursivo = False
Bode_Controlador_Digital = True
Bode_Sensor_OpenLoop_CloseLoop_Digital = True
Polos_Ceros_CloseLoop_Digital = False
Escalon_CloseLoop_Digital = True


#TF without constant
s = Symbol('s')

# FILTRO T
# desde Vinput (sin Vinput) al sensor de corriente
Rload = 11.0
# Rload = 2.0
Rsense = 0.33
L1 = 3.6e-3
L2 = L1
C = 0.44e-6

# FILTRO DEL SENSOR (polo y opamp - polo y cero -)
Rp = 1800
Cp = 56e-9

Ri = 1000
Rf = 8200
Cf = 22e-9

Amp_gain = 9.2    #no se usa se saca como (Ri + Rf)/Ri

# VCC for low-signal graphics
vcc = 80

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


Sensor_in = Plant_out_sim * transf_polo_opamp_sim * vcc
Sensor_in_sim = Sensor_in.simplify()

print ('Entrada del ADC: ')
print (Sensor_in_sim)

Sensor_out = Sensor_in_sim
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
planta_dig_zoh_n, planta_dig_zoh_d, td = cont2discrete((planta_TF.num, planta_TF.den), Tsampling_mult, method='zoh')

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
    ax1.set_title(f'Digital Bode ZOH Yellow no Vcc, Cyan with Vcc = {vcc}V')
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
    



##################
# PID Parameters #
##################
kdiv = 256
kp_dig = 20
ki_dig = 1
kd_dig = 0

k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

b_pid = [k1, k2, k3]
a_pid = [1, -1]

show_pid = PID_int(b_pid, a_pid, kdiv)
show_pid.showParamsFromK(kp_dig, ki_dig, kd_dig, Fsampling)

kp_dig = kp_dig / kdiv
ki_dig = ki_dig / kdiv
kd_dig = kd_dig / kdiv
k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

b_pid = [k1, k2, k3]
a_pid = [1, -1]

controller_tf = TransferFunction(b_pid, a_pid, dt=td)    

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
    w, mag_ol, phase_ol = dbode(open_loop_dig, n = 100000)
    w, mag_cl, phase_cl = dbode(close_loop_dig, n = 100000)
    
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag_ol, 'b')
    ax1.semilogx(w/(2*np.pi), mag_cl, 'c')    
    ax1.set_title('Open Loop Blue - Close Loop Cyan')
    ax1.set_ylim(ymin=-50, ymax=50)

    ax2.semilogx(w/(2*np.pi), phase_ol, 'b')
    ax2.semilogx(w/(2*np.pi), phase_cl, 'c')    
    ax2.set_title('Phase')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Polos_Ceros_CloseLoop_Digital == True:
    plot_argand(close_loop_dig)

    
##########################################################
# Verifico Respuesta Escalon Planta Digital Realimentada #
##########################################################
tiempo_de_simulacion = 0.01
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

