//-----------------------------------------------
// #### MICROINVERTER PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### SYNC.C ##################################
//-----------------------------------------------
#include "sync.h"
#include "dsp.h"
#include "tim.h"
#include "adc.h"

/* Externals variables -----------------------------------*/
extern volatile unsigned short timer_no_sync;
extern volatile unsigned short delta_t2;
extern volatile unsigned short delta_t1;
extern volatile unsigned short delta_t1_bar;
extern volatile unsigned short adc_ch[];

/* Global variables --------------------------------------*/
polarity_t last_polarity = POLARITY_UNKNOW;
unsigned char check_pulse_polarity = 0;

//-- For Ints Handlers
volatile unsigned char zero_crossing_now = 0;
volatile unsigned char voltage_is_good = 1;
volatile unsigned char sync_pulses_are_good = 0;
volatile unsigned char frequency_is_good = 0;
volatile unsigned char ac_sync_int_flag = 0;



/* Module Definitions ------------------------------------*/


/* Module functions --------------------------------------*/

void SYNC_Update_Sync (void)
{
    //si termino un pulso de synchro 
    if (ac_sync_int_flag)
    {
        ac_sync_int_flag = 0;
        timer_no_sync = TT_FOR_NO_SYNC;
        check_pulse_polarity = 1;

        //evaluo primero la frecuencia pulso a pulso
        if ((delta_t2 < DELTA_T2_FOR_49HZ) &&
            (delta_t2 > DELTA_T2_FOR_51HZ))
            frequency_is_good = 1;
        else
            frequency_is_good = 0;
        
        //evaluar y activar sync interno
        MA32Circular_Load(delta_t1);

        delta_t1_bar = MA32Circular_Calc();
        delta_t1_bar >>= 1;

        if ((delta_t1_bar < DELTA_T1_BAR_FOR_49HZ) &&
            (delta_t1_bar > DELTA_T1_BAR_FOR_51HZ))
            sync_pulses_are_good = 1;
        else
            sync_pulses_are_good = 0;

    }

    //reviso si hace rato no tengo pulsos
    if (!timer_no_sync)
    {
        frequency_is_good = 0;
        sync_pulses_are_good = 0;
    }
}

void SYNC_Update_Polarity (void)
{
    if ((frequency_is_good) &&
        (sync_pulses_are_good))
    {
        if ((check_pulse_polarity) &&
            (TIM16->CNT > (delta_t2 - delta_t1_bar)))
        {
            check_pulse_polarity = 0;
            if (Vline_Sense > VLINE_SENSE_MIN_THRESOLD)
                last_polarity = POLARITY_POS;
            else
                last_polarity = POLARITY_NEG;
        }
    }
    else
        last_polarity = POLARITY_UNKNOW;
    
}

polarity_t SYNC_Polarity_Check (void)
{
    return last_polarity;
}

void SYNC_Rising_Edge_Handler (void)
{
    //si tengo bien la frecuencia, tension y pulsos de sync -> activo el cruce
    if ((frequency_is_good) &&
        (sync_pulses_are_good) &&
        (voltage_is_good))
    {
        TIM17->CNT = 0;
        TIM17->ARR = delta_t1_bar;
        TIM17Enable();
    }
}

void SYNC_Falling_Edge_Handler (void)
{
    ac_sync_int_flag = 1;
    zero_crossing_now = 0;
}

void SYNC_Zero_Crossing_Handler (void)
{
    zero_crossing_now = 1;
    TIM17Disable();
}

unsigned char SYNC_Sync_Now (void)
{
    return zero_crossing_now;
}

void SYNC_Sync_Now_Reset (void)
{
    zero_crossing_now = 0;
}

//--- end of file ---//