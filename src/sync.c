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
extern volatile unsigned short adc_ch[];

/* Global variables --------------------------------------*/
polarity_t last_polarity = POLARITY_UNKNOWN;
unsigned char sync_check_sync_pulse_start = 0;
unsigned char sync_check_sync_pulse_stop = 0;
unsigned char sync_check_pulse_polarity = 0;
unsigned char cycles_cnt = 0;
unsigned short vline_max = 0;

unsigned short delta_t2_bar = 0;
unsigned short delta_t1_half_bar = 0;



//-- For Ints Handlers
volatile unsigned char zero_crossing_now = 0;
volatile unsigned char voltage_is_good = 1;
volatile unsigned char sync_pulses_are_good = 0;
volatile unsigned char frequency_is_good = 0;
volatile unsigned short delta_t2 = 0;
volatile unsigned short delta_t1 = 0;

ma32_u16_data_obj_t delta_t1_filter;
ma32_u16_data_obj_t delta_t2_filter;

/* Module Definitions ------------------------------------*/


/* Module functions --------------------------------------*/
void SYNC_InitSetup (void)
{
    MA32_U16Circular_Reset(&delta_t1_filter);
    MA32_U16Circular_Reset(&delta_t2_filter);    
}


void SYNC_Update_Sync (void)
{
    //si empieza un pulso de synchro
    if (sync_check_sync_pulse_start)
    {
        sync_check_sync_pulse_start = 0;

        //evaluo la frecuencia
        delta_t2_bar = MA32_U16Circular(&delta_t2_filter, delta_t2);

        if (delta_t2 > 400)
        {
            if ((delta_t2 > (delta_t2_bar - 400)) &&
                (delta_t2 < (delta_t2_bar + 400)))
            {
                frequency_is_good = 1;
            }
            else
                frequency_is_good = 0;
        }
        else
            frequency_is_good = 0;
    }

    // si termina un pulso de synchro
    if (sync_check_sync_pulse_stop)
    {
        sync_check_sync_pulse_stop = 0;
        cycles_cnt++;
        
        //evaluar y activar sync interno
        delta_t1_half_bar = MA32_U16Circular(&delta_t1_filter, delta_t1);
        delta_t1_half_bar >>= 1;

        if ((delta_t1 < DELTA_T1_MAX) &&
            (delta_t1 > DELTA_T1_MIN))
            sync_pulses_are_good = 1;
        else
            sync_pulses_are_good = 0;

        //si la frecuencia es buena y los pulsos de sync tambien, chequeo polaridad
        if ((frequency_is_good) && (sync_pulses_are_good))
            sync_check_pulse_polarity = 1;
    }

    //reviso si hace rato no tengo pulsos
    if (TIM6->CNT > (delta_t2_bar + 2 * 400))
    {
        frequency_is_good = 0;
        sync_pulses_are_good = 0;
    }
}


// Una vez determinado el synchro chequea la polaridad del pulso y la tension maxima
// espera medio ciclo para hacer esto
void SYNC_Update_Polarity (void)
{
    if (sync_check_pulse_polarity)
    {
        if (TIM16->CNT > ((delta_t2_bar >> 1) + delta_t1_half_bar))
        {
            sync_check_pulse_polarity = 0;
            
#ifdef TWO_KB817
            vline_max = Vline_Sense;
            if ((vline_max > VLINE_SENSE_MIN) &&
                (vline_max < VLINE_SENSE_MAX))
                voltage_is_good = 1;
            else
                voltage_is_good = 0;
            
#endif
            
            if (Vline_Sense > VLINE_SENSE_MIN_THRESOLD)
            {
                //si uso solo un KB817 la tension la mido solo en el ciclo positivo
#ifdef ONLY_ONE_KB817
                vline_max = Vline_Sense;
                if ((vline_max > VLINE_SENSE_MIN) &&
                    (vline_max < VLINE_SENSE_MAX))
                    voltage_is_good = 1;
                else
                    voltage_is_good = 0;
#endif
                // si la anterior no era positiva
                if (last_polarity != POLARITY_POS)
                    last_polarity = POLARITY_POS;
                else
                    last_polarity = POLARITY_UNKNOWN;                
            }
            else
            {
                // si la anterior no era negativa
                if (last_polarity != POLARITY_NEG)
                    last_polarity = POLARITY_NEG;
                else
                    last_polarity = POLARITY_UNKNOWN;
            }

#ifdef USE_LED_FOR_VLINE_MAX
            if (LED)
                LED_OFF;
            else
                LED_ON;
#endif
        }
    }
}


void SYNC_Restart (void)
{
    frequency_is_good = 0;
    sync_pulses_are_good = 0;
    voltage_is_good = 0;
    
    SYNC_InitSetup ();
    last_polarity = POLARITY_UNKNOWN;
}


polarity_t SYNC_Last_Polarity_Check (void)
{
    return last_polarity;
}


void SYNC_Rising_Edge_Handler (void)
{
    //reseteo tim
    delta_t2 = TIM16->CNT;
    TIM16->CNT = 0;

    //habilito el chequeo de frecuencia
    sync_check_sync_pulse_start = 1;

    //activo el estimado del proximo cruce
    TIM17->CNT = 0;
    TIM17->ARR = delta_t1_half_bar;
    TIM17Enable();
#ifdef USE_LED_FOR_SYNC_PULSES
    LED_ON;
#endif

}


void SYNC_Falling_Edge_Handler (void)
{
    delta_t1 = TIM16->CNT;

    //habilito el chequeo de pulso de synchro
    sync_check_sync_pulse_stop = 1;

#ifdef USE_LED_FOR_SYNC_PULSES
    LED_OFF;
#endif    
}


void SYNC_Zero_Crossing_Handler (void)
{
    zero_crossing_now = 1;
    TIM17Disable();

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


unsigned char SYNC_All_Good (void)
{
    if (frequency_is_good &&
         sync_pulses_are_good &&
         voltage_is_good)
        return 1;
    else
        return 0;
}
         
unsigned short SYNC_delta_t1_half_bar (void)
{
    return delta_t1_half_bar;
}


unsigned short SYNC_delta_t2_bar (void)
{
    return delta_t2_bar;
}


unsigned short SYNC_delta_t2 (void)
{
    return delta_t2;
}


unsigned short SYNC_delta_t1 (void)
{
    return delta_t1;
}


unsigned short SYNC_vline_max (void)
{
    return vline_max;
}


unsigned char SYNC_Frequency_Check (void)
{
    return frequency_is_good;
}


unsigned char SYNC_Pulses_Check (void)
{
    return sync_pulses_are_good;
}


unsigned char SYNC_Vline_Check (void)
{
    return voltage_is_good;
}

//--- end of file ---//
