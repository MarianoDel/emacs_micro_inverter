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


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
unsigned short RandomGen (unsigned int seed)
{
	unsigned int random;

	//Random Generator
	srand (seed);
	random = rand();

	return (unsigned short) random;

}

#ifdef USE_MA16_U16_CIRCULAR
//set de punteros y vaciado del filtro
//recibe:
// puntero a estructura de datos del filtro "ma16_u16_data_obj_t *"
void MA16_U16Circular_Reset (ma16_u16_data_obj_t * p_data)
{
    unsigned char i;
    
    for (i = 0; i < 16; i++)
        p_data->v_ma[i] = 0;

    p_data->p_ma = p_data->v_ma;
    p_data->total_ma = 0;
}

//Filtro circular, necesito activar previamente con MA16_U16Circular_Reset()
//recibe:
// puntero a estructura de datos del filtro "ma16_u16_data_obj_t *"
// nueva mustra "new_sample"
//contesta:
// resultado del filtro
unsigned short MA16_U16Circular (ma16_u16_data_obj_t *p_data, unsigned short new_sample)
{
    p_data->total_ma -= *(p_data->p_ma);
    p_data->total_ma += new_sample;
    *(p_data->p_ma) = new_sample;

    if (p_data->p_ma < ((p_data->v_ma) + 15))
        p_data->p_ma += 1;
    else
        p_data->p_ma = p_data->v_ma;

    return (unsigned short) (p_data->total_ma >> 4);    
}

unsigned short MA16_U16Circular_Only_Calc (ma16_u16_data_obj_t *p_data)
{
    return (unsigned short) (p_data->total_ma >> 4);
}

#endif    //USE_MA16_U16_CIRCULAR


#ifdef USE_MA32_U8_CIRCULAR
//set de punteros y vaciado del filtro
//recibe:
// puntero a estructura de datos del filtro "ma32_u8_data_obj_t *"
void MA32_U8Circular_Reset (ma32_u8_data_obj_t * p_data)
{
    unsigned char i;
    
    for (i = 0; i < 32; i++)
        p_data->v_ma[i] = 0;

    p_data->p_ma = p_data->v_ma;
    p_data->total_ma = 0;
}

//Filtro circular, necesito activar previamente con MA32_U8Circular_Reset()
//recibe:
// puntero a estructura de datos del filtro "ma32_u8_data_obj_t *"
// nueva mustra "new_sample"
//contesta:
// resultado del filtro
unsigned char MA32_U8Circular (ma32_u8_data_obj_t *p_data, unsigned char new_sample)
{
    p_data->total_ma -= *(p_data->p_ma);
    p_data->total_ma += new_sample;
    *(p_data->p_ma) = new_sample;

    if (p_data->p_ma < ((p_data->v_ma) + 31))
        p_data->p_ma += 1;
    else
        p_data->p_ma = p_data->v_ma;

    return (unsigned char) (p_data->total_ma >> 5);    
}

unsigned char MA32_U8Circular_Only_Calc (ma32_u8_data_obj_t *p_data)
{
    return (unsigned char) (p_data->total_ma >> 5);
}

#endif    //USE_MA32_U8_CIRCULAR


#ifdef USE_MA32_U16_CIRCULAR
//set de punteros y vaciado del filtro
//recibe:
// puntero a estructura de datos del filtro "ma32_u16_data_obj_t *"
void MA32_U16Circular_Reset (ma32_u16_data_obj_t * p_data)
{
    unsigned char i;
    
    for (i = 0; i < 32; i++)
        p_data->v_ma[i] = 0;

    p_data->p_ma = p_data->v_ma;
    p_data->total_ma = 0;
}

//Filtro circular, necesito activar previamente con MA32_U16Circular_Reset()
//recibe:
// puntero a estructura de datos del filtro "ma32_u16_data_obj_t *"
// nueva mustra "new_sample"
//contesta:
// resultado del filtro
unsigned short MA32_U16Circular (ma32_u16_data_obj_t *p_data, unsigned short new_sample)
{
    p_data->total_ma -= *(p_data->p_ma);
    p_data->total_ma += new_sample;
    *(p_data->p_ma) = new_sample;

    if (p_data->p_ma < ((p_data->v_ma) + 31))
        p_data->p_ma += 1;
    else
        p_data->p_ma = p_data->v_ma;

    return (unsigned short) (p_data->total_ma >> 5);    
}

unsigned short MA32_U16Circular_Only_Calc (ma32_u16_data_obj_t *p_data)
{
    return (unsigned short) (p_data->total_ma >> 5);
}

#endif    //USE_MA32_U16_CIRCULAR

#ifdef USE_PID_CONTROLLERS
// #define PID_CONSTANT_DIVIDER    10    //todos se dividen por 1024
// #define PID_CONSTANT_DIVIDER    8    //todos se dividen por 256
#define PID_CONSTANT_DIVIDER    7    //todos se dividen por 128
// #define PID_CONSTANT_DIVIDER    6    //todos se dividen por 64


short PID (pid_data_obj_t * p)
{
    int acc = 0;
    short error = 0;
    short d = 0;

    unsigned short k1 = 0;
    unsigned short k2 = 0;
    unsigned short k3 = 0;
    
    short val_k1 = 0;
    short val_k2 = 0;
    short val_k3 = 0;

    k1 = p->kp + p->ki + p->kd;
    k2 = p->kp + p->kd + p->kd;
    k3 = p->kd;
    
    error = p->setpoint - p->sample;

    //K1
    acc = k1 * error;
    val_k1 = acc / 128;
    // val_k1 = acc / 1024;

    //K2
    acc = k2 * p->error_z1;
    val_k2 = acc / 128;
    // val_k2 = acc / 1024;

    //K3
    acc = k3 * p->error_z2;
    val_k3 = acc / 128;
    // val_k3 = acc / 1024;

    d = p->last_d + val_k1 - val_k2 + val_k3;

    //Update PID variables
    p->error_z2 = p->error_z1;
    p->error_z1 = error;
    p->last_d = d;

    return d;
}

void PID_Flush_Errors (pid_data_obj_t * p)
{
    p->last_d = 0;
    p->error_z1 = 0;
    p->error_z2 = 0;
}

short PID_Small_Ki (pid_data_obj_t * p)
{
    int acc = 0;
    short error = 0;
    short d = 0;

    unsigned short k1_small = 0;
    unsigned short k2 = 0;
    unsigned short k3 = 0;
    
    short val_k1 = 0;
    short val_k2 = 0;
    short val_k3 = 0;

    // k1 = p->kp + p->ki + p->kd;
    k2 = p->kp + p->kd + p->kd;
    k3 = p->kd;
    
    error = p->setpoint - p->sample;

    //K1 -- desarmo K1 para mayor definicion del integral
    p->ki_accumulator += p->ki * error;
    k1_small = p->kp + p->kd;
    acc = k1_small * error + p->ki_accumulator;

    if ((p->ki_accumulator > 127) || (p->ki_accumulator < -127))
        p->ki_accumulator = 0;
    
    val_k1 = acc / 128;

    //K2
    acc = k2 * p->error_z1;
    val_k2 = acc / 128;

    //K3
    acc = k3 * p->error_z2;
    val_k3 = acc / 128;

    d = p->last_d + val_k1 - val_k2 + val_k3;

    //Update PID variables
    p->error_z2 = p->error_z1;
    p->error_z1 = error;
    p->last_d = d;

    return d;
}


void PID_Small_Ki_Flush_Errors (pid_data_obj_t * p)
{
    p->last_d = 0;
    p->ki_accumulator = 0;
    p->error_z1 = 0;
    p->error_z2 = 0;
}


#endif    //USE_PID_CONTROLLERS

//--- end of file ---//
