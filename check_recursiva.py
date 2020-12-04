# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand
from recursive_tf import RecursiveTF

"""
        Chequeo de la simulacion recursiva, filtros
        Polo simple RC
        Polo y cero con Opamp
"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Planta_Analog = False
Bode_Planta_Digital = False
Polos_Ceros_Digital = False
Escalon_Sensor_Digital = True
Escalon_Sensor_Digital_Recursivo = True


#TF without constant
s = Symbol('s')

# FILTRO RC
R = 1800
C = 56e-9

# FILTRO Polo Cero con Opamp
Ri = 1000
Rf = 8200
Cf = 22e-9

# Polo simple RC
transf_filtro1 = 1 / (1 + s*R*C)
transf_filtro1_sim = transf_filtro1.simplify()
print ('Planta filtro1:')
print (transf_filtro1_sim)

## Opamp
transf_opamp_gain = (Ri + Rf)/Ri
transf_opamp_pole = 1 + s*Cf*Rf
transf_opamp_zero = 1 + s*Cf*Rf*Ri/(Rf+Ri)
transf_opamp = transf_opamp_gain * transf_opamp_zero / transf_opamp_pole
transf_opamp_sim = transf_opamp.simplify()
print ('Opamp:')
print (transf_opamp_sim)


################################################
# Algunas confirmaciones de la parte Analogica #
################################################
polo_TF = sympy_to_lti(transf_filtro1_sim)
opamp_TF = sympy_to_lti(transf_opamp_sim)

if Bode_Planta_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(polo_TF, wfreq)
    w, mag_op, phase_op = bode(opamp_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    ax1.semilogx (w/6.28, mag_op, 'g-', linewidth="1")
    ax1.set_title('Magnitude Blue polo Green Opamp')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    ax2.semilogx (w/6.28, phase_op, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()



########################################################
# Convierto Planta por zoh a la frecuencia de muestreo #
# para que no afecte polos o ceros                     #
########################################################
Fsampling = 70000
Tsampling = 1 / Fsampling

polo_dig_zoh_n, polo_dig_zoh_d, td = cont2discrete((polo_TF.num, polo_TF.den), Tsampling, method='zoh')
opamp_dig_zoh_n, opamp_dig_zoh_d, td = cont2discrete((opamp_TF.num, opamp_TF.den), Tsampling, method='zoh')

#normalizo con TransferFunction
polo_dig_zoh = TransferFunction(polo_dig_zoh_n, polo_dig_zoh_d, dt=td)
print ("Polo Digital Zoh:")
print (polo_dig_zoh)

print ("Polo Digital Zoh ZPK:")
polo_dig_zoh_z, polo_dig_zoh_p, polo_dig_zoh_k = tf2zpk(polo_dig_zoh_n, polo_dig_zoh_d)
print ("Zeros")
print (polo_dig_zoh_z)
print ("Poles")
print (polo_dig_zoh_p)
print ("Gain")
print (polo_dig_zoh_k)

opamp_dig_zoh = TransferFunction(opamp_dig_zoh_n, opamp_dig_zoh_d, dt=td)
print ("Opamp Digital Zoh:")
print (opamp_dig_zoh)

print ("Opamp Digital Zoh ZPK:")
opamp_dig_zoh_z, opamp_dig_zoh_p, opamp_dig_zoh_k = tf2zpk(opamp_dig_zoh_n, opamp_dig_zoh_d)
print ("Zeros")
print (opamp_dig_zoh_z)
print ("Poles")
print (opamp_dig_zoh_p)
print ("Gain")
print (opamp_dig_zoh_k)


if Bode_Planta_Digital == True:
    w, magp_zoh, phasep_zoh = dbode(polo_dig_zoh, n = 10000)
    w, magop_zoh, phaseop_zoh = dbode(opamp_dig_zoh, n = 10000)    

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), magp_zoh, 'b')
    ax1.semilogx(w/(2*np.pi), magop_zoh, 'g')        
    ax1.set_title('Digital Bode ZOH Blue polo Green Opamp')

    ax2.set_title('Phase')    
    ax2.semilogx(w/(2*np.pi), phasep_zoh, 'b')
    ax2.semilogx(w/(2*np.pi), phaseop_zoh, 'g')        

    plt.tight_layout()
    plt.show()


######################################
# Polos y Ceros de la planta Digital #
######################################
if Polos_Ceros_Digital == True:
    plot_argand(polo_dig_zoh)
    plot_argand(opamp_dig_zoh)    

    
#############################################
# Verifico Respuesta Escalon Planta Digital #
#############################################
tiempo_de_simulacion = 0.01
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))

tout, yout_zoh = dstep([polo_dig_zoh.num, polo_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout_zoh)
yout0 = yout1[0]
polo_yout_zoh = yout0[:tout.size]

tout, yout_zoh = dstep([opamp_dig_zoh.num, opamp_dig_zoh.den, td], t=t)
yout1 = np.transpose(yout_zoh)
yout0 = yout1[0]
opamp_yout_zoh = yout0[:tout.size]


if Escalon_Sensor_Digital == True:
    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital ZOH Blue polo Green Opamp')
    ax.set_ylabel('Tension')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()

    ax.plot(tout, polo_yout_zoh, 'b')
    ax.plot(tout, opamp_yout_zoh, 'g')    
    # ax.set_ylim(ymin=-20, ymax=100)

    plt.tight_layout()
    plt.show()


############################################################
# Verifico Respuesta Escalon Planta Convertida a Recursiva #
############################################################
if Escalon_Sensor_Digital_Recursivo == True:
    # ZOH Polo
    b_polo = np.transpose(polo_dig_zoh.num)
    a_polo = np.transpose(polo_dig_zoh.den)

    vin_plant = np.ones(t.size)
    vout_plant_polo = np.zeros (t.size)    
    recur_polo = RecursiveTF(b_polo, a_polo)
    for i in range(t.size):
        vout_plant_polo[i] = recur_polo.newOutput(vin_plant[i])


    # ZOH Opamp
    b_opamp = np.transpose(opamp_dig_zoh.num)
    a_opamp = np.transpose(opamp_dig_zoh.den)

    vin_plant = np.ones(t.size)
    vout_plant_opamp = np.zeros (t.size)    
    recur_opamp = RecursiveTF(b_opamp, a_opamp)
    for i in range(t.size):
        vout_plant_opamp[i] = recur_opamp.newOutput(vin_plant[i])
        


    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Recursiva ZOH Blue polo Green Opamp')
    ax.set_ylabel('Tension')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant_polo, 'b')
    ax.plot(t, vout_plant_opamp, 'g')        
    plt.tight_layout()
    plt.show()
    

