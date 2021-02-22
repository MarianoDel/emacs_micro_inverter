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
        Solo filtro T con Rline Rsense y opamp solo amplificacion
        Vcc usado puede ser 1 (small signal) o 
        40 (exceso de tension en el maximo entre seniales)
        Salida:
        Tension en Rsense vs Vinput


"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Filtro_Analog = False
Bode_Filtro_Opamp_Analog = False

Bode_Filtro_Digital = False
Bode_Filtro_Opamp_Digital = False
Polos_Ceros_Digital = False

Escalon_Filtro_Opamp_Digital = False
Escalon_Filtro_Opamp_Digital_Recursivo = True


#TF without constant
s = Symbol('s')

# FILTRO T
# desde Vinput (sin Vinput) al sensor de corriente
Rline = 11.0
Rsense = 0.33
L1 = 3.4e-3
L2 = L1
C = 0.44e-6

# OPAMP gain
Amp_gain = 4.4

# VCC for low-signal graphics
Vinput = 1
# Vinput = 40

#    ---Z1---+---Z3---+
#            |        |
#           Z2       Z4
#            |        |
#    --------+--------+
Z1 = s * L1
Z2 = 1 / (s * C)
Z3 = s * L2 + Rline
Z4 = Rsense

## Thevenin en Z2
Rth = Z1 * Z2 / (Z1 + Z2)

# Filtro T 
print (f'Filtro T sin amplificar Vinput = {Vinput}V:')
Transf_Z4_Vinput = (Z2 * Z4 /((Z1 + Z2) * (Rth + Z3 + Z4)))* Vinput
Filter_out_sim = Transf_Z4_Vinput.simplify()
print (Filter_out_sim)

# Opamp
print (f'Filtro T con Opamp en Rsense Vinput = {Vinput}V: ')
Transf_Z4_Vinput_Opamp = Transf_Z4_Vinput * Amp_gain
Filter_Opamp_out_sim = Transf_Z4_Vinput_Opamp.simplify()
print (Filter_Opamp_out_sim)



################################################
# Algunas confirmaciones de la parte Analogica #
################################################
filter_TF = sympy_to_lti(Filter_out_sim)
if Bode_Filtro_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(filter_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    # ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title(f'Magnitude Input to Output Filter Analog Vinput = {Vinput}V')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    # ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()


filter_opamp_TF = sympy_to_lti(Filter_Opamp_out_sim)
if Bode_Filtro_Opamp_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(filter_opamp_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'y-', linewidth="1")
    # ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title(f'Magnitude Input to Output plus Opamp Analog Vinput = {Vinput}V')

    ax2.semilogx (w/6.28, phase_p, 'y-', linewidth="1")
    # ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()
    


########################################################
# Convierto Planta por zoh a la frecuencia de muestreo #
# para que no afecte polos o ceros                     #
########################################################
Fsampling = 24000
Tsampling = 1 / Fsampling

filter_dig_zoh_n, filter_dig_zoh_d, td = cont2discrete((filter_TF.num, filter_TF.den), Tsampling, method='zoh')
filter_opamp_dig_zoh_n, filter_opamp_dig_zoh_d, td = cont2discrete((filter_opamp_TF.num, filter_opamp_TF.den), Tsampling, method='zoh')

#normalizo con TransferFunction
print ("Filtro Digital Zoh:")
filter_dig_zoh = TransferFunction(filter_dig_zoh_n, filter_dig_zoh_d, dt=td)
print (filter_dig_zoh)

print ("Filtro y Opamp Digital Zoh:")
filter_opamp_dig_zoh = TransferFunction(filter_opamp_dig_zoh_n, filter_opamp_dig_zoh_d, dt=td)
print (filter_opamp_dig_zoh)


##############################################
# Algunas confirmaciones de la parte Digital #
##############################################
if Bode_Filtro_Digital == True:
    w, mag_zoh, phase_zoh = dbode(filter_dig_zoh, n = 10000)

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')
    ax1.set_title(f'Filter Digital Bode ZOH Vinput = {Vinput}V')
    ax1.set_xlabel('Frequency [Hz]')

    ax2.semilogx(w/(2*np.pi), phase_zoh, 'y')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


if Bode_Filtro_Opamp_Digital == True:
    w, mag_zoh, phase_zoh = dbode(filter_opamp_dig_zoh, n = 10000)

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')
    ax1.set_title(f'Filter and Opamp Digital Bode ZOH Vinput = {Vinput}V')
    ax1.set_xlabel('Frequency [Hz]')

    ax2.semilogx(w/(2*np.pi), phase_zoh, 'y')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()
    

    

######################################
# Polos y Ceros de la planta Digital #
######################################
if Polos_Ceros_Digital == True:
    plot_argand(filter_opamp_dig_zoh)

    
#####################################################
# Verifico Respuesta Escalon Filtro y Opamp Digital #
#####################################################
tiempo_de_simulacion = 0.01
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))


if Escalon_Filtro_Opamp_Digital == True:
    tout, yout_zoh = dstep([filter_opamp_dig_zoh.num, filter_opamp_dig_zoh.den, td], t=t)
    yout1 = np.transpose(yout_zoh)
    yout0 = yout1[0]
    yout_zoh = yout0[:tout.size]

    fig, ax = plt.subplots()
    ax.set_title('Step Filtro y Opamp Digital ZOH')
    ax.set_ylabel('Tension de la Planta')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()

    ax.plot(tout, yout_zoh, 'y')
    # ax.set_ylim(ymin=-20, ymax=100)

    plt.tight_layout()
    plt.show()


####################################################################
# Verifico Respuesta Escalon Filtro y Opamp Convertida a Recursiva #
####################################################################
if Escalon_Filtro_Opamp_Digital_Recursivo == True:
    # ZOH
    b_planta = np.transpose(filter_opamp_dig_zoh.num)
    a_planta = np.transpose(filter_opamp_dig_zoh.den)

    vin_plant = np.ones(t.size)
    vout_plant_method2 = np.zeros (t.size)    
    recur_planta = RecursiveTF(b_planta, a_planta)
    for i in range(t.size):
        vout_plant_method2[i] = recur_planta.newOutput(vin_plant[i])
    

    fig, ax = plt.subplots()
    ax.set_title('Step Planta Digital Recursiva ZOH')
    ax.set_ylabel('Tension del Planta')
    ax.set_xlabel('Tiempo [s]')
    ax.grid()
    ax.plot(t, vout_plant_method2, 'y')    
    plt.tight_layout()
    plt.show()
    
