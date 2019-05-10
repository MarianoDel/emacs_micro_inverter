//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DSP.C #################################
//---------------------------------------------

#include "dsp.h"


#include <stdlib.h>
#include <math.h>


/* Externals variables ---------------------------------------------------------*/
extern unsigned short pid_param_p;
extern unsigned short pid_param_i;
extern unsigned short pid_param_d;


/* Global variables ---------------------------------------------------------*/
//------- de los PID ---------
#ifdef USE_PID_CONTROLLERS
volatile int acc = 0;
short error_z1 = 0;
short error_z2 = 0;
short d_last = 0;
#endif

#ifdef USE_MA8_CIRCULAR
unsigned short v_ma8 [8];
unsigned short * p_ma8;
#endif
#ifdef USE_MA32_CIRCULAR
unsigned short v_ma_circular [32];
unsigned short * p_ma_circular;
#endif
/* Module Definitions ---------------------------------------------------------*/
//todos se dividen por 128
#define KPV	857			// 6.7 desde python PI_zpk_KpKi.py
#define KIV	844			// 6.6 desde python PI_zpk_KpKi.py
#define KDV	0			// 0

#define K1V (KPV + KIV + KDV)
#define K2V (KPV + KDV + KDV)
#define K3V (KDV)

/* Module functions ---------------------------------------------------------*/

unsigned short RandomGen (unsigned int seed)
{
	unsigned int random;

	//Random Generator
	srand (seed);
	random = rand();

	return (unsigned short) random;

}

#ifdef USE_MA32_CIRCULAR
//seteo de punteros del filtro circular
void MA32Circular_Start (void)
{
    p_ma_circular = &v_ma_circular[0];
}

//reset de punteros al filtro circular
void MA32Circular_Reset (void)
{
    unsigned char i;
    
    MA32Circular_Start();
    for (i = 0; i < 32; i++)
        v_ma_circular[i] = 0;
}

void MA32Circular_Load (unsigned short new_sample)
{
    *p_ma_circular = new_sample;

    if (p_ma_circular < (v_ma_circular + 31))
        p_ma_circular += 1;
    else
        p_ma_circular = &v_ma_circular[0];

}

unsigned short MA32Circular_Calc (void)
{
    unsigned int total_ma;

    total_ma = v_ma_circular[0] + v_ma_circular[1] + v_ma_circular[2] + v_ma_circular[3] +
        v_ma_circular[4] + v_ma_circular[5] + v_ma_circular[6] + v_ma_circular[7] +
        v_ma_circular[8] + v_ma_circular[9] + v_ma_circular[10] + v_ma_circular[11] +
        v_ma_circular[12] + v_ma_circular[13] + v_ma_circular[14] + v_ma_circular[15] +
        v_ma_circular[16] + v_ma_circular[17] + v_ma_circular[18] + v_ma_circular[19] +
        v_ma_circular[20] + v_ma_circular[21] + v_ma_circular[22] + v_ma_circular[23] +
        v_ma_circular[24] + v_ma_circular[25] + v_ma_circular[26] + v_ma_circular[27] +
        v_ma_circular[28] + v_ma_circular[29] + v_ma_circular[30] + v_ma_circular[31];

    return (unsigned short) (total_ma >> 5);
}

#endif

#ifdef USE_MA8_CIRCULAR
//seteo de punteros del filtro circular
void MA8Circular_Start (void)
{
    p_ma8 = &v_ma8[0];
}

//reset de punteros al filtro circular
void MA8Circular_Reset (void)
{
    unsigned char i;
    
    MA8Circular_Start();
    for (i = 0; i < 8; i++)
        v_ma8[i] = 0;
}

//Filtro circular, necesito activar previamente con MA8Circular_Start()
//MA8Circular_Reset() vacia el filtro
//recibe: new_sample
//contesta: resultado
unsigned short MA8Circular (unsigned short new_sample)
{
    unsigned int total_ma;

    *p_ma8 = new_sample;

    total_ma = v_ma8[0] + v_ma8[1] + v_ma8[2] + v_ma8[3] + v_ma8[4] + v_ma8[5] + v_ma8[6] + v_ma8[7];

    if (p_ma8 < (v_ma8 + 7))
        p_ma8 += 1;
    else
        p_ma8 = &v_ma8[0];

    return (unsigned short) (total_ma >> 3);
}
#endif

unsigned short MAFilterFast (unsigned short new_sample, unsigned short * vsample)
{
	unsigned int total_ma;

	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7

	total_ma = new_sample + *(vsample) + *(vsample + 1) + *(vsample + 2);
	*(vsample + 2) = *(vsample + 1);
	*(vsample + 1) = *(vsample);
	*(vsample) = new_sample;

	return (unsigned short) (total_ma >> 2);
}

//unsigned short MAFilter8 (unsigned short new_sample, unsigned short * vsample)
unsigned short MAFilter8 (unsigned short * vsample)
{
	unsigned int total_ma;

	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7

	total_ma = *(vsample) + *(vsample + 1) + *(vsample + 2) + *(vsample + 3) + *(vsample + 4) + *(vsample + 5) + *(vsample + 6) + *(vsample + 7);
	*(vsample + 7) = *(vsample + 6);
	*(vsample + 6) = *(vsample + 5);
	*(vsample + 5) = *(vsample + 4);
	*(vsample + 4) = *(vsample + 3);
	*(vsample + 3) = *(vsample + 2);
	*(vsample + 2) = *(vsample + 1);
	*(vsample + 1) = *(vsample);

	return (unsigned short) (total_ma >> 3);
}

unsigned short MAFilter32 (unsigned short new_sample, unsigned short * vsample)
{
	unsigned short total_ma;

	total_ma = new_sample + *(vsample) + *(vsample + 1) + *(vsample + 2) + *(vsample + 3) + *(vsample + 4) + *(vsample + 5) + *(vsample + 6);
	total_ma += *(vsample + 7) + *(vsample + 8) + *(vsample + 9) + *(vsample + 10) + *(vsample + 11) + *(vsample + 12) + *(vsample + 13) + *(vsample + 14);
	total_ma += *(vsample + 15) + *(vsample + 16) + *(vsample + 17) + *(vsample + 18) + *(vsample + 19) + *(vsample + 20) + *(vsample + 21) + *(vsample + 22);
	total_ma += *(vsample + 23) + *(vsample + 24) + *(vsample + 25) + *(vsample + 26) + *(vsample + 27) + *(vsample + 28) + *(vsample + 29) + *(vsample + 30);

	*(vsample + 30) = *(vsample + 29);
	*(vsample + 29) = *(vsample + 28);
	*(vsample + 28) = *(vsample + 27);
	*(vsample + 27) = *(vsample + 26);
	*(vsample + 26) = *(vsample + 25);
	*(vsample + 25) = *(vsample + 24);
	*(vsample + 24) = *(vsample + 23);

	*(vsample + 23) = *(vsample + 22);
	*(vsample + 22) = *(vsample + 21);
	*(vsample + 21) = *(vsample + 20);
	*(vsample + 20) = *(vsample + 19);
	*(vsample + 19) = *(vsample + 18);
	*(vsample + 18) = *(vsample + 17);
	*(vsample + 17) = *(vsample + 16);
	*(vsample + 16) = *(vsample + 15);

	*(vsample + 15) = *(vsample + 14);
	*(vsample + 14) = *(vsample + 13);
	*(vsample + 13) = *(vsample + 12);
	*(vsample + 12) = *(vsample + 11);
	*(vsample + 11) = *(vsample + 10);
	*(vsample + 10) = *(vsample + 9);
	*(vsample + 9) = *(vsample + 8);
	*(vsample + 8) = *(vsample + 7);

	*(vsample + 7) = *(vsample + 6);
	*(vsample + 6) = *(vsample + 5);
	*(vsample + 5) = *(vsample + 4);
	*(vsample + 4) = *(vsample + 3);
	*(vsample + 3) = *(vsample + 2);
	*(vsample + 2) = *(vsample + 1);
	*(vsample + 1) = *(vsample);
	*(vsample) = new_sample;

	return total_ma >> 5;
}

unsigned short MAFilter32Fast (unsigned short * vsample)
{
	unsigned int total_ma;

	total_ma = *(vsample) + *(vsample + 1) + *(vsample + 2) + *(vsample + 3) +
            *(vsample + 4) + *(vsample + 5) + *(vsample + 6) + *(vsample + 7);
        
	total_ma += *(vsample + 8) + *(vsample + 9) + *(vsample + 10) + *(vsample + 11) +
            *(vsample + 12) + *(vsample + 13) + *(vsample + 14) + *(vsample + 15);
        
	total_ma += *(vsample + 16) + *(vsample + 17) + *(vsample + 18) + *(vsample + 19) +
            *(vsample + 20) + *(vsample + 21) + *(vsample + 22) + *(vsample + 23);
        
	total_ma += *(vsample + 24) + *(vsample + 25) + *(vsample + 26) + *(vsample + 27) +
            *(vsample + 28) + *(vsample + 29) + *(vsample + 30) + *(vsample + 31);

	return (unsigned short) (total_ma >> 5);
}

//Filtro circular, recibe
//new_sample, p_vec_samples: vector donde se guardan todas las muestras
//p_vector: puntero que recorre el vector de muestras, p_sum: puntero al valor de la sumatoria de muestras
//devuelve resultado del filtro
unsigned short MAFilter32Circular (unsigned short new_sample, unsigned short * p_vec_samples, unsigned char * index, unsigned int * p_sum)
{
	unsigned int total_ma;
	unsigned short * p_vector;

	p_vector = (p_vec_samples + *index);

	//agrego la nueva muestra al total guardado
	total_ma = *p_sum + new_sample;

	//guardo el nuevo sample y actualizo el puntero
	*p_vector = new_sample;
	if (p_vector < (p_vec_samples + 31))
	{
		p_vector++;
		*index += 1;
	}
	else
	{
		p_vector = p_vec_samples;
		*index = 0;
	}

	//resto la muestra - 32 (es la apuntada por el puntero porque es circular)
	total_ma -= *p_vector;
	*p_sum = (unsigned short) total_ma;

	return total_ma >> 5;
}

#ifdef USE_PID_CONTROLLERS
short PID (short setpoint, short sample)
{
    short error = 0;
    short d = 0;

    short val_k1 = 0;
    short val_k2 = 0;
    short val_k3 = 0;

    error = setpoint - sample;

    //K1
    acc = K1V * error;		//5500 / 32768 = 0.167 errores de hasta 6 puntos
    val_k1 = acc >> 7;

    //K2
    acc = K2V * error_z1;		//K2 = no llega pruebo con 1
    val_k2 = acc >> 7;			//si es mas grande que K1 + K3 no lo deja arrancar

    //K3
    acc = K3V * error_z2;		//K3 = 0.4
    val_k3 = acc >> 7;

    d = d_last + val_k1 - val_k2 + val_k3;

    //Update variables PID
    error_z2 = error_z1;
    error_z1 = error;
    d_last = d;

    return d;
}

#define K1    (pid_param_p + pid_param_i + pid_param_d)
#define K2    (pid_param_p + pid_param_d + pid_param_d)
#define K3    (pid_param_d)

short PID_roof (short setpoint, short sample, short local_last_d, short * e_z1, short * e_z2)
{
    short error = 0;
    short d = 0;

    short val_k1 = 0;
    short val_k2 = 0;
    short val_k3 = 0;

    error = setpoint - sample;

    //K1
    acc = K1 * error;
    val_k1 = acc >> 7;

    //K2
    acc = K2 * *e_z1;		//K2 = no llega pruebo con 1
    val_k2 = acc >> 7;			//si es mas grande que K1 + K3 no lo deja arrancar

    //K3
    acc = K3 * *e_z2;		//K3 = 0.4
    val_k3 = acc >> 7;

    d = local_last_d + val_k1 - val_k2 + val_k3;

    //Update variables PID
    *e_z2 = *e_z1;
    *e_z1 = error;

    return d;
}

#endif    //USE_PID_CONTROLLERS

//--- end of file ---//
