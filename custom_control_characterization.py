# -*- coding: utf-8 -*-
#usar python3
import numpy as np
from sympy import *
import matplotlib.pyplot as plt
from scipy.signal import lti, bode, lsim, dbode, zpk2tf, tf2zpk, step2, cont2discrete, dstep, freqz, freqs, dlti, TransferFunction
from tc_udemm import sympy_to_lti, lti_to_sympy, plot_argand


""" 
        Digital control starting with analog
        this is a phase lead, if the zero comes before the pole
        this is a phase lag, if the pole its placed before the zero

"""

########################
# Which Graphs to Show #
########################
Bode_Custom_Analog = True
Bode_Custom_Digital = True
Poles_Zeros_Digital = True
Bode_Sensor_Analog = False

######################################
# Custom Control Pole-Zero placement #
######################################
analog_zeros = [-100.0]
analog_poles = [-2000.0]
analog_const = 2.0

analog_zpk_b, analog_zpk_a = zpk2tf(analog_zeros, analog_poles, analog_const)
controller_tf = TransferFunction(analog_zpk_b, analog_zpk_a)

if Bode_Custom_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*100000, 1)
    w, mag_p, phase_p = bode(controller_tf, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    ax1.set_title('Analog Bode')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()

#################################
# Continue (Analog) to Discrete #
#################################
Fsampling = 24000
Tsampling = 1 / Fsampling
disc_zoh_n, disc_zoh_d, td = cont2discrete((controller_tf.num, controller_tf.den), Tsampling, method='zoh')

# normalize
controller_dtf = TransferFunction(disc_zoh_n, disc_zoh_d, dt=td)
print ("Digital Controller:")
print (controller_dtf)

########################
# Bode Control Digital #
########################
if Bode_Custom_Digital == True:
    w, mag_zoh, phase_zoh = dbode(controller_dtf, n = 10000)

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')    
    ax1.set_title('Digital Bode ZOH')
    # ax1.set_ylabel('Amplitude [dB]', color='y')
    # ax1.set_xlabel('Frequency [Hz]')

    ax2.semilogx(w/(2*np.pi), phase_zoh, 'y')
    ax2.set_title('Phase')    
    # ax2.set_ylabel('Phase', color='y')
    # ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


#######################
# Poles Zeros Digital #
#######################
if Poles_Zeros_Digital == True:
    plot_argand(controller_dtf)




#--- end of file ---#
