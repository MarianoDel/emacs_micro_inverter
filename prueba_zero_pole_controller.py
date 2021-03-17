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
        Controlador PR - proportional resonant -


"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Bode_Controlador_Analog = True

Bode_Controlador_Digital = True
Polos_Ceros_Digital = True

Escalon_Controlador_Opamp_Digital = False
Escalon_Controlador_Opamp_Digital_Recursivo = True


#TF without constant
s = Symbol('s')

# Gs = Kp + (2 . Ki . wbw . s) / (s**2 + 2 . wbw . s + w0**2)
# wbw = ancho de banda en el resonador
# w0 = frecuencia del resonador

# desde Vinput (sin Vinput) al sensor de corriente
gain = 0.3
fzero = 5
fpole = 50

wzero = 2 * np.pi * fzero
wpole = 2 * np.pi * fpole

eff_gain = gain * wpole / wzero

Gs = eff_gain * (s + wzero) / (s + wpole)

# PR Controller
print ("Controlador PR:")
controller_pr = Gs.simplify()
print (controller_pr)



################################################
# Algunas confirmaciones de la parte Analogica #
################################################
controller_TF = sympy_to_lti(controller_pr)

if Bode_Controlador_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*10000, 1)
    w, mag_p, phase_p = bode(controller_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    # ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title(f'Magnitude zero pole controller fz: {fzero} fp: {fpole}')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
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

controller_dig_zoh_n, controller_dig_zoh_d, td = cont2discrete((controller_TF.num, controller_TF.den), Tsampling, method='zoh')


#normalizo con TransferFunction
print ("Controlador PR Digital Zoh:")
controller_dig_zoh = TransferFunction(controller_dig_zoh_n, controller_dig_zoh_d, dt=td)
print (controller_dig_zoh)


##############################################
# Algunas confirmaciones de la parte Digital #
##############################################
if Bode_Controlador_Digital == True:
    w, mag_zoh, phase_zoh = dbode(controller_dig_zoh, n = 10000)

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')
    ax1.set_title('Controller Digital Bode ZOH')
    ax1.set_xlabel('Frequency [Hz]')
    ax1.set_ylim(ymin=-50)

    ax2.semilogx(w/(2*np.pi), phase_zoh, 'y')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()



######################################
# Polos y Ceros de la planta Digital #
######################################
if Polos_Ceros_Digital == True:
    plot_argand(controller_dig_zoh)

    
# #####################################################
# # Verifico Respuesta Escalon Filtro y Opamp Digital #
# #####################################################
# tiempo_de_simulacion = 0.01
# t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))


# if Escalon_Filtro_Opamp_Digital == True:
#     tout, yout_zoh = dstep([filter_opamp_dig_zoh.num, filter_opamp_dig_zoh.den, td], t=t)
#     yout1 = np.transpose(yout_zoh)
#     yout0 = yout1[0]
#     yout_zoh = yout0[:tout.size]

#     fig, ax = plt.subplots()
#     ax.set_title('Step Filtro y Opamp Digital ZOH')
#     ax.set_ylabel('Tension de la Planta')
#     ax.set_xlabel('Tiempo [s]')
#     ax.grid()

#     ax.plot(tout, yout_zoh, 'y')
#     # ax.set_ylim(ymin=-20, ymax=100)

#     plt.tight_layout()
#     plt.show()


# ####################################################################
# # Verifico Respuesta Escalon Filtro y Opamp Convertida a Recursiva #
# ####################################################################
# if Escalon_Filtro_Opamp_Digital_Recursivo == True:
#     # ZOH
#     b_planta = np.transpose(filter_opamp_dig_zoh.num)
#     a_planta = np.transpose(filter_opamp_dig_zoh.den)

#     vin_plant = np.ones(t.size)
#     vout_plant_method2 = np.zeros (t.size)    
#     recur_planta = RecursiveTF(b_planta, a_planta)
#     for i in range(t.size):
#         vout_plant_method2[i] = recur_planta.newOutput(vin_plant[i])
    

#     fig, ax = plt.subplots()
#     ax.set_title('Step Planta Digital Recursiva ZOH')
#     ax.set_ylabel('Tension del Planta')
#     ax.set_xlabel('Tiempo [s]')
#     ax.grid()
#     ax.plot(t, vout_plant_method2, 'y')    
#     plt.tight_layout()
#     plt.show()
    
