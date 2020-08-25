# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand
from pid_tf import PID_float, PID_int

"""
        Reviso respuestas del sistema en 320V y 25V para estimar
        mejores parametros PI
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

###########################################################################
# Convierto Sensor ZOH a una frecuencia mucho mas alta que la de muestreo #
# para que no afecte polos o ceros                                        #
###########################################################################
Fsampling = 24000

sensor320_TF = sympy_to_lti(Sensor_out_sim * 320)
sensor25_TF = sympy_to_lti(Sensor_out_sim * 25)
print ("Input 320V sensor con sympy:")
print (sensor320_TF)
print ("Input 25V sensor con sympy:")
print (sensor25_TF)

Fsampling_mult = Fsampling
Tsampling_mult = 1 / Fsampling_mult
    
sensor320_dig_zoh_n, sensor320_dig_zoh_d, td = cont2discrete((sensor320_TF.num, sensor320_TF.den), Tsampling_mult, method='zoh')
sensor25_dig_zoh_n, sensor25_dig_zoh_d, td = cont2discrete((sensor25_TF.num, sensor25_TF.den), Tsampling_mult, method='zoh')


#normalizo con TransferFunction
sensor320_dig_zoh = TransferFunction(sensor320_dig_zoh_n, sensor320_dig_zoh_d, dt=td)
sensor25_dig_zoh = TransferFunction(sensor25_dig_zoh_n, sensor25_dig_zoh_d, dt=td)

w, mag320_zoh, phase320_zoh = dbode(sensor320_dig_zoh, n = 10000)
w, mag25_zoh, phase25_zoh = dbode(sensor25_dig_zoh, n = 10000)

if Bode_Sensor_Digital == True:
    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag320_zoh, 'g')
    ax1.semilogx(w/(2*np.pi), mag25_zoh, 'y')    
    ax1.set_title('Input 320V green, 25V yellow - ZOH -')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='g')
    ax1.set_xlabel('Frequency [Hz]')
    # ax1.set_ylim(ymin=-40, ymax=40)

    ax2.semilogx(w/(2*np.pi), phase320_zoh, 'g')
    ax2.semilogx(w/(2*np.pi), phase25_zoh, 'y')    
    ax2.set_ylabel('Phase', color='g')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()

#############################################
# Verifico Respuesta Escalon Planta Digital #
#############################################
tiempo_de_simulacion = 0.01
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))

tout, yout320_zoh = dstep([sensor320_dig_zoh.num, sensor320_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout320_zoh)
yout0 = yout1[0]
yout320_zoh = yout0[:tout.size]

tout, yout25_zoh = dstep([sensor25_dig_zoh.num, sensor25_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout25_zoh)
yout0 = yout1[0]
yout25_zoh = yout0[:tout.size]


if Escalon_Sensor_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital 320V green 25V yellow - ZOH -')
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(tout, yout320_zoh, 'g')
    ax.plot(tout, yout25_zoh, 'y')    
    # ax.set_ylim(ymin=-20, ymax=100)

    plt.tight_layout()
    plt.show()

############################################################
# Verifico Respuesta Escalon Planta Convertida a Recursiva #
############################################################
# ZOH
b_sensor320 = np.transpose(sensor320_dig_zoh_n)
a_sensor320 = np.transpose(sensor320_dig_zoh_d)

b_sensor25 = np.transpose(sensor25_dig_zoh_n)
a_sensor25 = np.transpose(sensor25_dig_zoh_d)

vin_plant = np.ones(t.size)
vin_plant[0:3] = 0
vout_plant320 = np.zeros (t.size)
vout_plant25 = np.zeros (t.size)

########################################
# aplico la transferencia de la planta #
########################################
for i in range(3, len(vin_plant)):
    vout_plant320[i] = b_sensor320[0]*vin_plant[i] \
                       + b_sensor320[1]*vin_plant[i-1] \
                       + b_sensor320[2]*vin_plant[i-2] \
                       + b_sensor320[3]*vin_plant[i-3] \
                       - a_sensor320[1]*vout_plant320[i-1] \
                       - a_sensor320[2]*vout_plant320[i-2] \
                       - a_sensor320[3]*vout_plant320[i-3]

########################################
# aplico la transferencia de la planta #
########################################
for i in range(3, len(vin_plant)):
    vout_plant25[i] = b_sensor25[0]*vin_plant[i] \
                      + b_sensor25[1]*vin_plant[i-1] \
                      + b_sensor25[2]*vin_plant[i-2] \
                      + b_sensor25[3]*vin_plant[i-3] \
                      - a_sensor25[1]*vout_plant25[i-1] \
                      - a_sensor25[2]*vout_plant25[i-2] \
                      - a_sensor25[3]*vout_plant25[i-3]

if Escalon_Sensor_Digital_Recursivo == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Recursiva 320V green 25V yellow - ZOH -')    
    ax.set_ylabel('Tension del Sensor')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant320, 'g')
    ax.plot(t, vout_plant25, 'y')    
    plt.tight_layout()
    plt.show()


#################
# Digitals PIDs #
#################
ki_dig = 3 / 128
kp_dig = 10 / 128
kd_dig = 0.0

k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

b320_pid = [k1, k2, k3]

ki_dig = 16 / 128
kp_dig = 30 / 128
kd_dig = 0.0

k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

b25_pid = [k1, k2, k3]

a_pid = [1, -1]

pid_update_freq = 4000
undersampling_td = 1 / pid_update_freq
controller320_tf = TransferFunction(b320_pid, a_pid, dt=undersampling_td)
controller25_tf = TransferFunction(b25_pid, a_pid, dt=undersampling_td)

###############
# OpenLoop TF #
###############
w, mag320, phase320 = dbode(controller320_tf, n = 10000)
w, mag25, phase25 = dbode(controller25_tf, n = 10000)

if Bode_Controlador_Digital == True:
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag320, 'g')
    ax1.semilogx(w/(2*np.pi), mag25, 'y')    
    ax1.set_title('Digital Controller')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='c')
    ax1.set_xlabel('Frequency [Hz]')

    ax2.semilogx(w/(2*np.pi), phase320, 'g')
    ax2.semilogx(w/(2*np.pi), phase25, 'y')    
    ax2.set_ylabel('Phase', color='c')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Bode_Sensor_CloseLoop_Digital == True:
    contr320_dig = lti_to_sympy(controller320_tf)
    plant_dig = lti_to_sympy(sensor320_dig_zoh)
    
    ol320_dig = plant_dig * contr320_dig
    open320_loop_dig = sympy_to_lti(ol320_dig)
    close320_loop_dig = sympy_to_lti(ol320_dig/(1+ol320_dig))

    contr25_dig = lti_to_sympy(controller25_tf)
    plant_dig = lti_to_sympy(sensor25_dig_zoh)
    
    ol25_dig = plant_dig * contr25_dig
    open25_loop_dig = sympy_to_lti(ol25_dig)
    close25_loop_dig = sympy_to_lti(ol25_dig/(1+ol25_dig))
    
    #normalizo con TransferFunction
    open320_loop_dig = TransferFunction(open320_loop_dig.num, open320_loop_dig.den, dt=td)    
    close320_loop_dig = TransferFunction(close320_loop_dig.num, close320_loop_dig.den, dt=td)

    open25_loop_dig = TransferFunction(open25_loop_dig.num, open25_loop_dig.den, dt=td)    
    close25_loop_dig = TransferFunction(close25_loop_dig.num, close25_loop_dig.den, dt=td)
    
    w, mag320_ol, phase320_ol = dbode(open320_loop_dig, n = 10000)
    w, mag320_cl, phase320_cl = dbode(close320_loop_dig, n = 10000)

    w, mag25_ol, phase25_ol = dbode(open25_loop_dig, n = 10000)
    w, mag25_cl, phase25_cl = dbode(close25_loop_dig, n = 10000)
    
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx(w/(2*np.pi), mag320_ol, 'g')
    ax1.semilogx(w/(2*np.pi), mag320_cl, 'g')    
    ax1.semilogx(w/(2*np.pi), mag25_ol, 'y')
    ax1.semilogx(w/(2*np.pi), mag25_cl, 'y')    
    ax1.set_title('Open Loop Blue, Close Loop Cyan')
    ax1.set_ylabel('Amplitude P D2 [dB]', color='b')
    ax1.set_xlabel('Frequency [Hz]')
    ax1.set_ylim(ymin=-40, ymax=40)

    ax2.semilogx(w/(2*np.pi), phase320_ol, 'g')
    ax2.semilogx(w/(2*np.pi), phase320_cl, 'g')    
    ax2.semilogx(w/(2*np.pi), phase25_ol, 'y')
    ax2.semilogx(w/(2*np.pi), phase25_cl, 'y')    
    ax2.set_ylabel('Phase', color='b')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Polos_Ceros_CloseLoop_Digital == True:
    plot_argand(close320_loop_dig)
