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
polarity_t last_polarity = POLARITY_UNKNOWN;
unsigned char check_pulse_polarity = 0;
unsigned char cycles_cnt = 0;
unsigned short vline_max = 0;

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
        cycles_cnt++;
        
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
            (TIM16->CNT > ((delta_t2 >> 1) + delta_t1_bar)))
        {
            check_pulse_polarity = 0;
            vline_max = Vline_Sense;
            if (Vline_Sense > VLINE_SENSE_MIN_THRESOLD)
                last_polarity = POLARITY_POS;
            else
                last_polarity = POLARITY_NEG;

#ifdef USE_LED_FOR_VLINE_MAX
            if (LED)
                LED_OFF;
            else
                LED_ON;
#endif
        }
    }
    else
        last_polarity = POLARITY_UNKNOWN;
    
}

polarity_t SYNC_Last_Polarity_Check (void)
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
#ifdef USE_LED_FOR_SYNC_PULSES
        LED_ON;
#endif
    }
}

void SYNC_Falling_Edge_Handler (void)
{
    ac_sync_int_flag = 1;
    zero_crossing_now = 0;
#ifdef USE_LED_FOR_SYNC_PULSES
    LED_OFF;
#endif    
}

void SYNC_Zero_Crossing_Handler (void)
{
#ifdef INVERTER_ONLY_SYNC_AND_POLARITY
    zero_crossing_now = 1;
    TIM17Disable();
#endif
#ifdef INVERTER_MODE
    cycles_cnt++;
    zero_crossing_now = 1;
#endif

#ifdef USE_LED_FOR_ZERO_CROSSING
    if (LED)
        LED_OFF;
    else
        LED_ON;
#endif    
}

unsigned char SYNC_Sync_Now (void)
{
    return zero_crossing_now;
}

void SYNC_Sync_Now_Reset (void)
{
    zero_crossing_now = 0;
}

unsigned char SYNC_Cycles_Cnt (void)
{
    return cycles_cnt;
}

void SYNC_Cycles_Cnt_Reset (void)
{
    cycles_cnt = 0;
}

unsigned short SYNC_Vline_Max (void)
{
    return vline_max;
}

//--- end of file ---//
