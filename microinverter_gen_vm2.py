# -*- coding: utf-8 -*-
#usar python3
import numpy as np
import matplotlib.pyplot as plt
from sympy import *
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy

"""
        MicroInversor - Etapa de salida de la parte Inverter
	Etapa C.
        Funcion: Generador de senial senoidal
        Modo: Voltage Mode - VM
        Feedback: por tension de salida, sensada a traves del opto
"""

#######################################
# Elementos de Hardware               #
# Caracteristica del filtro de salida #
#######################################
Lout1 = 930e-6
Lout2 = 930e-6
Cout = 0.44e-6
Rload = 2200
opto_TF = 0.0066    # alrededor de la tension a usar

Lout = Lout1 + Lout2
Vinput = 250

# Etapa de potencia respecto de la salida
s = Symbol('s')
Y2 = 1 / Rload + s * Cout
Z2 = 1 / Y2
Z1 = s * Lout
Zout = Z2 / (Z1 + Z2)
Plant_out = Vinput * Zout
Plant_out_sim = Plant_out.simplify()

print ('Plant_out: ')
print (Plant_out_sim)

Sensor_out = Plant_out * opto_TF
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
t = np.linspace(0, 0.02, num=2000)
u = np.ones_like(t)
t, y, x = lsim(sensor_TF, u, t)

fig.clear()
fig, ax = plt.subplots()
ax.set_title('Respuesta de la Planta vista desde el sensor')
ax.set_ylabel('Vopto')
ax.set_xlabel('Tiempo [s]')
ax.grid()
ax.plot(t, y, 'g-')
ax.plot(t, u, color='orange')

plt.tight_layout()
plt.show()

#######################################
# Convierto Planta y Filtro a Digital #
# por Tustin                          #
#######################################
Fsampling = 48000
Tsampling = 1 / Fsampling
sensor_dig_tustin_n, sensor_dig_tustin_d, td = cont2discrete((sensor_TF.num, sensor_TF.den), Tsampling, method='tustin')

#normalizo con TransferFunction
print ("Sensor Digital:")
sensor_dig_tustin = TransferFunction(sensor_dig_tustin_n, sensor_dig_tustin_d, dt=td)
print (sensor_dig_tustin)

wd_freq = np.arange(2*np.pi*Tsampling, 2*np.pi*1000*Tsampling, 1)
# w, mag, phase = dbode(sensor_dig_tustin, w=wd_freq)
w, mag, phase = dbode(sensor_dig_tustin, n = 10000)

fig, (ax1, ax2) = plt.subplots(2,1)

ax1.semilogx(w/(2*np.pi), mag, 'g')
ax1.set_title('Sensor Digital Tustin')
ax1.set_ylabel('Amplitude P D2 [dB]', color='g')
ax1.set_xlabel('Frequency [Hz]')

ax2.semilogx(w/(2*np.pi), phase, 'g')
ax2.set_ylabel('Phase', color='g')
ax2.set_xlabel('Frequency [Hz]')

plt.tight_layout()
plt.show()


########################
# Ecuacion PID Digital #
########################
# kp = 3.96    #ziegler-nichols
# ki = 3960
# kd = 0.001
kp = 0
ki = 30
kd = 0
ki_dig = ki / Fsampling
kp_dig = kp - ki_dig / 2
kd_dig = kd * Fsampling

if kp_dig < 0:
    kp_dig = 0
    
k1 = kp_dig + ki_dig + kd_dig
k2 = -kp_dig - 2*kd_dig
k3 = kd_dig

#este es el pid
b_pid = [k1, k2, k3]
a_pid = [1, -1]
print ("")
print ("kp_dig: " + str(kp_dig) + " ki_dig: " + str(ki_dig) + " kd_dig: " + str(kd_dig))
print ("")
#este es custom controller
# b_pid = [1.03925, -0.96075, 0]
# a_pid = [1, -1]

pid_dig = TransferFunction(b_pid, a_pid, dt=td)
print ("PID Digital:")
print (pid_dig)

w, mag, phase = dbode(pid_dig, n = 10000)

fig, (ax1, ax2) = plt.subplots(2,1)

ax1.semilogx(w/(2*np.pi), mag, 'c')
ax1.set_title('PID Digital')
ax1.set_ylabel('Amplitude P D2 [dB]', color='c')
ax1.set_xlabel('Frequency [Hz]')

ax2.semilogx(w/(2*np.pi), phase, 'c')
ax2.set_ylabel('Phase', color='c')
ax2.set_xlabel('Frequency [Hz]')

plt.tight_layout()
plt.show()

############################################
# Multiplico Transferencias para OpenLoop  #
############################################
c = lti_to_sympy(pid_dig)
p = lti_to_sympy(sensor_dig_tustin)

ol = c * p

open_loop = sympy_to_lti(ol)
open_loop = TransferFunction(open_loop.num, open_loop.den, dt=td)   #normalizo

w, mag, phase = dbode(open_loop, n = 10000)

fig, (ax1, ax2) = plt.subplots(2,1)

ax1.semilogx(w/(2*np.pi), mag, 'b')
ax1.set_title('Digital OpenLoop')
ax1.set_ylabel('Amplitude P D2 [dB]', color='b')
ax1.set_xlabel('Frequency [Hz]')
ax1.set_ylim([-20, 40])

ax2.semilogx(w/(2*np.pi), phase, 'r')
ax2.set_ylabel('Phase', color='r')
ax2.set_xlabel('Frequency [Hz]')

plt.tight_layout()
plt.show()


#########################################
# Realimento punto a punto con setpoint #
#########################################
# Respuesta escalon de la planta punto a punto
tiempo_de_simulacion = 0.1
print('td:')
print (td)
t = np.arange(0, tiempo_de_simulacion, td)

# Tustin
b_sensor = np.transpose(sensor_dig_tustin_n)
a_sensor = np.transpose(sensor_dig_tustin_d)

vin_plant = np.zeros(t.size)
vout_plant = np.zeros(t.size)


############################################
# Armo la senial que quiero en el SETPOINT #
############################################
fmains = 50
s_sen = np.zeros(t.size)

for i in range(np.size(s_sen)):
    s_sen[i] = np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size))

for i in range (np.size(s_sen)):
    if s_sen[i] < 0:
        s_sen[i] = -s_sen[i]

# vin_setpoint = np.ones(t.size)
vin_setpoint = s_sen

# muestro el setpoint
# fig, ax = plt.subplots()
# ax.set_title('Setpoint')
# ax.set_xlabel('t [s]')
# ax.grid()
# ax.plot(t, vin_setpoint, 'y')
# plt.tight_layout()
# plt.show()


d = np.zeros(t.size)
error = np.zeros(t.size)


max_d_pwm = 1.0    
for i in range(2, len(vin_plant)):
    ###################################################
    # primero calculo el error, siempre punto a punto #
    ###################################################
    error[i] = vin_setpoint[i] - vout_plant[i-1]

    ###################
    # aplico lazo PID #
    ###################
    d[i] = b_pid[0] * error[i] + b_pid[1] * error[i-1] + b_pid[2] * error[i-2] - a_pid[1] * d[i-1]

    ###########################################
    # ajusto los maximos permitidos en el pwm #
    ###########################################
    if d[i] > max_d_pwm:
        d[i] = max_d_pwm

    if d[i] < 0:
        d[i] = 0

    ########################################
    # aplico la transferencia de la planta #
    ########################################
    vin_plant[i] = d[i]
    vout_plant[i] = b_sensor[0]*vin_plant[i] \
                    + b_sensor[1]*vin_plant[i-1] \
                    + b_sensor[2]*vin_plant[i-2] \
                    - a_sensor[1]*vout_plant[i-1] \
                    - a_sensor[2]*vout_plant[i-2]
    
               
        
fig, ax = plt.subplots()
ax.set_title('Respuesta Realimentada punto a punto')
ax.set_ylabel('Corriente')
ax.set_xlabel('Tiempo en muestras')
ax.grid()
# ax.plot(t, d)
ax.plot(t, error, 'g')
ax.plot(t, vin_setpoint, 'y')
# ax.stem(t, vout_plant)
ax.plot(t, vout_plant, 'c')
plt.tight_layout()
plt.show()

#respuesta escalon
# t = np.arange (0, 0.1, td)
# tout, yout = dstep([planta_d1.num, planta_d1.den, td], t=t)
# yout1 = np.transpose(yout)
# yout0 = yout1[0]
# yout = yout0[:tout.size]

# plt.figure(1)
# plt.clf()
# plt.title('digital Step Response')

# plt.stem(tout,yout)
# plt.show()

# ###otro mas ajustado, agrega zero en 1 y compensa ganancia
# num_d2 = [0.091, 0.091]
# den_d2 = [1, -0.8861]

# #normalizo con lti
# planta_d2 = lti(num_d2, den_d2)

# print ('Numerador Digital sys 2')
# print (planta_d2.num)
# print ('Denominador Digital sys 2')
# print (planta_d2.den)

# tout, yout = dstep([planta_d2.num, planta_d2.den, td], t=t)
# yout1 = np.transpose(yout)
# yout0 = yout1[0]
# yout = yout0[:tout.size]


# plt.figure(1)
# plt.clf()
# plt.title('digital Step Response')

# # print (yout)
# plt.stem(tout,yout)
# plt.show()

# # en frecuencia segundo funcion transferencia dgital
# # w, h = freqz(num_d, den_d,worN=np.logspace(0, 4, 1000))
# w, h = freqz(planta_d1.num, planta_d1.den)
# fig, (ax1, ax2) = plt.subplots(2,1)

# ax1.semilogx(w/(2*pi)*Fsampling, 20 * np.log10(abs(h)), 'b')
# ax1.set_title('Planta Euler')
# ax1.set_ylabel('Amplitude P D1 [dB]', color='b')
# ax1.set_xlabel('Frequency [Hz]')

# angles = np.unwrap(np.angle(h))
# ax2.semilogx (w/(2*pi)*Fsampling, angles*180/pi, 'r-', linewidth="1")
# ax2.set_title('Angle')

# plt.tight_layout()
# plt.show(block=False)

# # en frecuencia
# # w, h = freqz(num_d, den_d,worN=np.logspace(0, 4, 1000))
# w, h = freqz(planta_d2.num, planta_d2.den)
# fig, (ax1, ax2) = plt.subplots(2,1)

# ax1.semilogx(w/(2*pi)*Fsampling, 20 * np.log10(abs(h)), 'b')
# ax1.set_title('Planta Euler + ajuste')
# ax1.set_ylabel('Amplitude P D2 [dB]', color='b')
# ax1.set_xlabel('Frequency [Hz]')

# angles = np.unwrap(np.angle(h))
# ax2.semilogx (w/(2*pi)*Fsampling, angles*180/pi, 'r-', linewidth="1")
# ax2.set_title('Angle')

# plt.tight_layout()
# plt.show()
