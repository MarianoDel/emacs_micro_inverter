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
        Usar 40V para graficos 1V para simulacion punto a punto    


"""

##########################################################################
# Cuales son los Graficos que quiero mostrar por cuestiones de velocidad #
##########################################################################
Use_Input_Distorted = True
Use_Current_Mode_Only_CCM = False
Use_Current_Modes_Changes = True

Bode_Filtro_Analog = False
Bode_Filtro_Opamp_Analog = False

Bode_Filtro_Digital = False
Bode_Filtro_Opamp_Digital = False
Polos_Ceros_Digital = False

Escalon_Filtro_Opamp_Digital = False
Escalon_Filtro_Opamp_Digital_Recursivo = False

Bode_PR_Controller_Analog = False
Bode_PR_Controller_Digital = False

Bode_OpenLoop_Digital = False
Show_System_Inputs = True
PR_Single_Stage = True

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

def Adc12Bits_Neg (sample):
    adc = np.int16(0)
    sample = sample / 3.3
    sample = sample * 4095

    if sample < 0.0:
        if sample < -4095:
            sample = -4095

    if sample > 4095:
        sample = 4095

    adc = sample
    return adc


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
# ZOH
b_planta = np.transpose(filter_opamp_dig_zoh.num)
a_planta = np.transpose(filter_opamp_dig_zoh.den)

if Escalon_Filtro_Opamp_Digital_Recursivo == True:
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
    

#################
# PR Controller #
#################
Kp = 0.01
Ki = 10.0
fbw = 2
f0 = 50

wbw = 2 * np.pi * fbw
w0 = 2 * np.pi * f0

Gs = Kp + (2 * Ki * wbw * s)/(s**2 + 2 * wbw * s + w0**2)
# PR Controller
print ("Controlador PR:")
controller_pr = Gs.simplify()
print (controller_pr)

controller_TF = sympy_to_lti(controller_pr)

if Bode_PR_Controller_Analog == True:
    wfreq = np.arange(2*np.pi, 2*np.pi*10000, 1)
    w, mag_p, phase_p = bode(controller_TF, wfreq)

    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.semilogx (w/6.28, mag_p, 'b-', linewidth="1")
    # ax1.semilogx (w/6.28, mag_s, 'g-', linewidth="1")
    ax1.set_title(f'Magnitude PR controller Kp: {Kp} Ki: {Ki}')

    ax2.semilogx (w/6.28, phase_p, 'b-', linewidth="1")
    # ax2.semilogx (w/6.28, phase_s, 'g-', linewidth="1")
    ax2.set_title('Phase')

    plt.tight_layout()
    plt.show()


Fsampling = 24000
Tsampling = 1 / Fsampling

controller_dig_zoh_n, controller_dig_zoh_d, td = cont2discrete((controller_TF.num, controller_TF.den), Tsampling, method='zoh')


#normalizo con TransferFunction
print ("Controlador PR Digital Zoh:")
controller_dig_zoh = TransferFunction(controller_dig_zoh_n, controller_dig_zoh_d, dt=td)
print (controller_dig_zoh)

# ZOH
b_pr = np.transpose(controller_dig_zoh.num)
a_pr = np.transpose(controller_dig_zoh.den)

##############################################
# Algunas confirmaciones de la parte Digital #
##############################################
if Bode_PR_Controller_Digital == True:
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



if Bode_OpenLoop_Digital == True:
    
    openloop = lti_to_sympy(filter_opamp_TF) * lti_to_sympy(controller_TF)
    openloop = sympy_to_lti(openloop)
    Fsampling = 24000
    Tsampling = 1 / Fsampling

    openloop_d_n, openloop_d_d, td = cont2discrete((openloop.num, openloop.den), Tsampling, method='zoh')

    #normalizo con TransferFunction
    openloop_d = TransferFunction(openloop_d_n, openloop_d_d, dt=td)

    w, mag_zoh, phase_zoh = dbode(openloop_d, n = 10000)

    fig, (ax1, ax2) = plt.subplots(2,1)

    ax1.semilogx(w/(2*np.pi), mag_zoh, 'y')
    ax1.set_title('OpenLoop Digital Bode ZOH')
    ax1.set_xlabel('Frequency [Hz]')
    ax1.set_ylim(ymin=-50)

    ax2.semilogx(w/(2*np.pi), phase_zoh, 'y')
    ax2.set_xlabel('Frequency [Hz]')

    plt.tight_layout()
    plt.show()


########################################
# Simulacion del sistema punto a punto #
########################################
tiempo_de_simulacion = 0.2
t = np.linspace(0, tiempo_de_simulacion, num=(tiempo_de_simulacion*Fsampling))

vsupply = np.ones(t.size) * 350

fmains = 50
line_peak = 311
vline = np.zeros(t.size)
for i in range(np.size(vline)):
    vline[i] = line_peak * np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size))


# con distorcion de entrada?
if Use_Input_Distorted == True:
    ciclos = np.size(vline) / 480
    ciclos = int (ciclos)
    print (f'distorsion de entrada, ciclos: {ciclos}')
    for k in range(ciclos):
        vline[k*480 + 95: k*480 + 145] = 295
        vline[k*480 + 240 + 95: k*480 + 240 + 145] = -295
        

ref_peak = 1800
vref = np.zeros(t.size)
for i in range(np.size(vref)):
    vref[i] = int(ref_peak * np.sin(2 * np.pi * fmains * tiempo_de_simulacion * (i/t.size)))

vout_plant_adc = np.zeros(t.size)
error = np.zeros(t.size)
d = np.zeros(t.size)
vin_plant_d = np.zeros(t.size)
vout_plant = np.zeros(t.size)
mode = np.zeros(t.size)

if Show_System_Inputs == True:
    fig, ax = plt.subplots()
    ax.set_title('Vsupply Vline Vref')
    ax.set_xlabel('t [s]')
    ax.grid()
    ax.plot(t, vsupply, 'b')
    ax.plot(t, vline, 'r')
    ax.plot(t, vref, 'y')    
    plt.tight_layout()
    plt.show()


if PR_Single_Stage == True:
    max_pwm_pts = 2000

    pr_recur = RecursiveTF(b_pr, a_pr)
    plant_recur = RecursiveTF(b_planta, a_planta)    
    for i in range(1, len(vsupply)):
        ###################################################
        # primero calculo el error, siempre punto a punto #
        ###################################################
        error[i] = vref[i] - vout_plant_adc[i-1]

        #############################
        # aplico Digital Controller #
        #############################
        d[i] = pr_recur.newOutput(error[i])

        #floor check
        if d[i] < 0:
            if d[i] < -max_pwm_pts:
                d[i] = -max_pwm_pts

        #roof check
        if d[i] > max_pwm_pts:
            d[i] = max_pwm_pts
            
        ##############################################
        # compenso la tension de salida que se opone #
        ##############################################
        if Use_Current_Mode_Only_CCM == True:
        # modo CCM continuamente
            vin_plant_d[i] = d[i] * vsupply[i] / max_pwm_pts - vline[i]

        if Use_Current_Modes_Changes == True:
            if vline[i] > 0:
                # primer cuadrante
                if vout_plant_adc[i-1] > 100:
                    # Modo CCM
                    mode[i] = 1000
                    vin_plant_d[i] = d[i] * vsupply[i] / max_pwm_pts
                    if vin_plant_d[i] > vline[i]:
                        vin_plant_d[i] = vin_plant_d[i] - vline[i]
                    else:
                        # fallback dcm
                        if d[i] != 0:
                            duty_sqr_local = d[i] / max_pwm_pts
                            duty_sqr_local = duty_sqr_local * duty_sqr_local
                            re = 2 * L1 * Fsampling / duty_sqr_local
                            rth = (Rline + Rsense) / (Rline + Rsense + re)

                            vin_plant_d[i] = (vsupply[i] - vline[i]) * rth
                        else:
                            vin_plant_d[i] = 0

                    
                    
                else:
                    # Modo DCM
                    mode[i] = 0
                    if d[i] != 0:
                        duty_sqr_local = d[i] / max_pwm_pts
                        duty_sqr_local = duty_sqr_local * duty_sqr_local
                        re = 2 * L1 * Fsampling / duty_sqr_local
                        rth = (Rline + Rsense) / (Rline + Rsense + re)

                        vin_plant_d[i] = (vsupply[i] - vline[i]) * rth
                    else:
                        vin_plant_d[i] = 0
                
            else:
                # tercer cuadrante
                if vout_plant_adc[i-1] < -100:
                    # Modo CCM
                    mode[i] = 1000
                    vin_plant_d[i] = d[i] * vsupply[i] / max_pwm_pts
                    if vin_plant_d[i] < vline[i]:
                        vin_plant_d[i] = vin_plant_d[i] - vline[i]
                    else:
                        # fallback dcm
                        if d[i] != 0:
                            duty_sqr_local = d[i] / max_pwm_pts
                            duty_sqr_local = duty_sqr_local * duty_sqr_local
                            re = 2 * L1 * Fsampling / duty_sqr_local
                            rth = (Rline + Rsense) / (Rline + Rsense + re)

                            vin_plant_d[i] = (-vsupply[i] - vline[i]) * rth
                        else:
                            vin_plant_d[i] = 0
                
                else:
                    # Modo DCM
                    mode[i] = 0
                    if d[i] != 0:
                        duty_sqr_local = d[i] / max_pwm_pts
                        duty_sqr_local = duty_sqr_local * duty_sqr_local
                        re = 2 * L1 * Fsampling / duty_sqr_local
                        rth = (Rline + Rsense) / (Rline + Rsense + re)

                        vin_plant_d[i] = (-vsupply[i] - vline[i]) * rth
                    else:
                        vin_plant_d[i] = 0
                
                
        vout_plant[i] = plant_recur.newOutput(vin_plant_d[i])        
        # vout_plant_adc[i] = Adc12Bits (vout_plant[i])
        vout_plant_adc[i] = Adc12Bits_Neg (vout_plant[i])        



if Respuesta_CloseLoop_All_Inputs_Digital == True:     
    fig, (ax1, ax2) = plt.subplots(2,1)
    ax1.set_title(f'ref_peak={ref_peak}')
    ax1.grid()
    ax1.plot(t, vref, 'r', label="sp")
    ax1.plot(t, vout_plant_adc, 'y', label="out_adc")
    ax1.plot(t, error, 'g', label="error")
    ax1.plot(t, d, 'm', label="duty")
    ax1.plot(t, mode, 'b', label="mode CCM")    
    ax1.legend(loc='upper left')

    # ax2.plot(t, vin_setpoint, 'y', label="sp")
    # ax.stem(t, vout_plant)
    ax2.plot(t, vout_plant, 'y', label="out")
    ax2.plot(t, vin_plant_d, 'm', label="in")

    # ax.set_ylim(ymin=-10, ymax=360)
    ax2.legend(loc='upper left')
    plt.tight_layout()
    plt.show()

# print (d)
# print (error)
