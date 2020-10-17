//-----------------------------------------------------
// #### MICROINVERTER PROJECT  F030 - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### MAIN.C ########################################
//-----------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gpio.h"
#include "tim.h"
#include "uart.h"
#include "hard.h"

#include "core_cm0.h"
#include "adc.h"
#include "dma.h"
#include "flash_program.h"
#include "dsp.h"
#include "it.h"
#include "sync.h"
#include "test_functions.h"

// Externals -------------------------------------------------------------------

// -- Externals from or for Serial Port --------------------
volatile unsigned char tx1buff [SIZEOF_DATA];
volatile unsigned char rx1buff [SIZEOF_DATA];
volatile unsigned char usart1_have_data = 0;

// -- Externals from or for the ADC ------------------------
volatile unsigned short adc_ch [ADC_CHANNEL_QUANTITY];
volatile unsigned char seq_ready = 0;

// -- Externals for the timers -----------------------------
volatile unsigned short wait_ms_var = 0;
volatile unsigned short timer_led = 0;


// Globals ---------------------------------------------------------------------
volatile unsigned char overcurrent_shutdown = 0;
volatile unsigned short timer_standby;



#ifdef USE_FREQ_12KHZ
// Select Current Signal
#define USE_SIGNAL_CURRENT_1A
#define SIZEOF_SIGNAL    120
#endif

#ifdef  USE_FREQ_24KHZ
// Select Current Signal
// #define USE_SIGNAL_CURRENT_01_A
#define USE_SIGNAL_CURRENT_05_A
// #define USE_SIGNAL_CURRENT_075_A
// #define USE_SIGNAL_CURRENT_1_A
#define SIZEOF_SIGNAL    240
#endif

#if (SIZEOF_SIGNAL == 120)
unsigned short sin_half_cycle [SIZEOF_SIGNAL] = {107,214,321,428,534,640,746,851,955,1059,
                                                 1163,1265,1366,1467,1567,1665,1762,1859,1953,2047,
                                                 2139,2230,2319,2406,2492,2577,2659,2740,2818,2895,
                                                 2970,3043,3113,3182,3248,3312,3374,3434,3491,3546,
                                                 3598,3648,3696,3740,3783,3823,3860,3894,3926,3955,
                                                 3981,4005,4026,4044,4059,4072,4082,4089,4093,4095,
                                                 4093,4089,4082,4072,4059,4044,4026,4005,3981,3955,
                                                 3926,3894,3860,3823,3783,3740,3696,3648,3598,3546,
                                                 3491,3434,3374,3312,3248,3182,3113,3043,2970,2895,
                                                 2818,2740,2659,2577,2492,2406,2319,2230,2139,2047,
                                                 1953,1859,1762,1665,1567,1467,1366,1265,1163,1059,
                                                 955,851,746,640,534,428,321,214,107,0};


#elif (SIZEOF_SIGNAL == 240)
unsigned short sin_half_cycle [SIZEOF_SIGNAL] = {53,107,160,214,267,321,374,428,481,534,
                                                 587,640,693,746,798,851,903,955,1007,1059,
                                                 1111,1163,1214,1265,1316,1366,1417,1467,1517,1567,
                                                 1616,1665,1714,1762,1811,1859,1906,1953,2000,2047,
                                                 2093,2139,2185,2230,2275,2319,2363,2406,2450,2492,
                                                 2535,2577,2618,2659,2700,2740,2779,2818,2857,2895,
                                                 2933,2970,3007,3043,3078,3113,3148,3182,3215,3248,
                                                 3281,3312,3344,3374,3404,3434,3463,3491,3519,3546,
                                                 3572,3598,3624,3648,3672,3696,3718,3740,3762,3783,
                                                 3803,3823,3841,3860,3877,3894,3910,3926,3941,3955,
                                                 3969,3981,3994,4005,4016,4026,4035,4044,4052,4059,
                                                 4066,4072,4077,4082,4086,4089,4091,4093,4094,4095,
                                                 4094,4093,4091,4089,4086,4082,4077,4072,4066,4059,
                                                 4052,4044,4035,4026,4016,4005,3994,3981,3969,3955,
                                                 3941,3926,3910,3894,3877,3860,3841,3823,3803,3783,
                                                 3762,3740,3718,3696,3672,3648,3624,3598,3572,3546,
                                                 3519,3491,3463,3434,3404,3374,3344,3312,3281,3248,
                                                 3215,3182,3148,3113,3078,3043,3007,2970,2933,2895,
                                                 2857,2818,2779,2740,2700,2659,2618,2577,2535,2492,
                                                 2450,2406,2363,2319,2275,2230,2185,2139,2093,2047,
                                                 2000,1953,1906,1859,1811,1762,1714,1665,1616,1567,
                                                 1517,1467,1417,1366,1316,1265,1214,1163,1111,1059,
                                                 1007,955,903,851,798,746,693,640,587,534,
                                                 481,428,374,321,267,214,160,107,53,0};


#else
#error "Select SIZEOF_SIGNAL in main.c"
#endif

// Set of Peak Current in ADC points, 12bits
// Voltage in I_Sense is 3 . Ipeak
// ADC points = 3 . Ipeak . 4095 / 3.3
#ifdef USE_SIGNAL_CURRENT_01_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    373
#endif

#ifdef USE_SIGNAL_CURRENT_05_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1861
#endif

#ifdef USE_SIGNAL_CURRENT_075_A
#define KI_SIGNAL_PEAK_MULTIPLIER    2792
#endif

#ifdef USE_SIGNAL_CURRENT_1_A
#define KI_SIGNAL_PEAK_MULTIPLIER    3722
#endif



#define INDEX_TO_MIDDLE    47
#define INDEX_TO_FALLING    156
#define INDEX_TO_REVERT    204
    
unsigned short * p_current_ref;
pid_data_obj_t current_pid;

// Module Private Functions ----------------------------------------------------
void SysTickError (void);
void SoftOverCurrentShutdown (unsigned char, unsigned short);
void PWM_Off (void);
unsigned short CurrentLoop (unsigned short, unsigned short);
void CurrentLoop_Change_to_LowGain (void);
void CurrentLoop_Change_to_HighGain (void);
void TimingDelay_Decrement (void);
extern void EXTI4_15_IRQHandler(void);



//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
    char s_lcd [120];
    ac_sync_state_t ac_sync_state = START_SYNCING;

    // Gpio Configuration.
    GPIO_Config();

    // Systick Timer Activation
    if (SysTick_Config(48000))
        SysTickError();

    // Peripherals Activation
    EXTIOff ();
    USART1Config();
    
    // Welcome Code and Features
#ifdef HARD
    Usart1Send((char *) HARD);
    Wait_ms(100);
#else
#error	"No Hardware defined in hard.h file"
#endif

#ifdef SOFT
    Usart1Send((char *) SOFT);
    Wait_ms(100);
#else
#error	"No Soft Version defined in hard.h file"
#endif

#ifdef FEATURES
    WelcomeCodeFeatures(s_lcd);
#endif

    
    TIM_3_Init();    //Used for mosfet channels control and ADC synchro

    TIM_16_Init();    //free running with tick: 1us
    TIM16Enable();
    TIM_17_Init();    //with int, tick: 1us

    // Initial Setup for the synchro module
    SYNC_InitSetup();
    
    PWM_Off();

    // ADC and DMA configuration
    AdcConfig();
    DMAConfig();
    DMA1_Channel1->CCR |= DMA_CCR_EN;
    ADC1->CR |= ADC_CR_ADSTART;
    //end of ADC & DMA

    EXTIOn();

    // Test Functions
    // TF_RelayConnect();
    // TF_RelayACPOS();
    // TF_RelayACNEG();
    // TF_RelayFiftyHz();
    TF_OnlySyncAndPolarity();
    // TF_Check_Sequence_Ready();
            
    // Main Program - Grid Tied Mode -
#ifdef GRID_TIED_FULL_CONECTED

    // Initial Setup for PID Controller
    PID_Flush_Errors(&current_pid);
    CurrentLoop_Change_to_LowGain();

    typedef enum {
        SIGNAL_RISING = 0,
        SIGNAL_MIDDLE,
        SIGNAL_FALLING,
        SIGNAL_REVERT
        
    } signal_state_e;

    signal_state_e signal_state = SIGNAL_RISING;
    unsigned short d = 0;
    unsigned short cycles_before_start = CYCLES_BEFORE_START;
    
    while (1)
    {
        switch (ac_sync_state)
        {
        case START_SYNCING:
            PWM_Off();
            LED_OFF;

            EnablePreload_Mosfet_HighLeft;
            EnablePreload_Mosfet_HighRight;
            PID_Flush_Errors(&current_pid);

            SYNC_Restart();
            cycles_before_start = CYCLES_BEFORE_START;
            ac_sync_state = SWITCH_RELAY_TO_ON;
            break;

        case SWITCH_RELAY_TO_ON:
            //Check voltage and polarity
            if (SYNC_All_Good())
            {
                RELAY_ON;
                timer_standby = 200;
                ac_sync_state = WAIT_RELAY_TO_ON;
            }
            break;
            
        case WAIT_RELAY_TO_ON:
            if (!timer_standby)
            {
                SYNC_Sync_Now_Reset();
                ac_sync_state = WAIT_SYNC_FEW_CYCLES_BEFORE_START;
            }
            break;

            // few cycles before actual begin
        case WAIT_SYNC_FEW_CYCLES_BEFORE_START:
            if (SYNC_Sync_Now())
            {
                if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
                {
                    //ahora es positiva la polaridad, prendo el led
#ifdef USE_LED_FOR_MAIN_POLARITY_BEFORE_GEN
                    LED_ON;
#endif
                }
                else if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
                    //ahora es negativa la polaridad, apago el led
#ifdef USE_LED_FOR_MAIN_POLARITY_BEFORE_GEN
                    LED_OFF;
#endif

                    //reviso si debo empezar a generar
                    if (cycles_before_start)
                        cycles_before_start--;
                    else
                        ac_sync_state = WAIT_FOR_FIRST_SYNC;
                }
                else    //debe haber un error en synchro
                    ac_sync_state = START_SYNCING;
                
                SYNC_Sync_Now_Reset();
            }            
            break;
            
        case WAIT_FOR_FIRST_SYNC:
            //por cuestiones de seguridad empiezo siempre por positivo
            if (SYNC_Sync_Now())
            {
                if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
                {
                    ChangeLed(LED_GENERATING);
                    ac_sync_state = WAIT_CROSS_NEG_TO_POS;

                    HIGH_RIGHT(DUTY_NONE);
                    LOW_LEFT(DUTY_NONE);
                    LOW_RIGHT(DUTY_ALWAYS);
                    sequence_ready_reset;
                }
                else if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
                    //no hago nada
                    //quiero empezar siempre por positivo
                }
                else    //debe haber un error en synchro
                    ac_sync_state = START_SYNCING;
                
                SYNC_Sync_Now_Reset();
            }
            break;
        
        case GEN_POS:
            if (sequence_ready)
            {
                sequence_ready_reset;

#ifdef WITH_SOFT_OVERCURRENT_SHUTDOWN
                if (I_Sense_Pos > SOFT_OVERCURRENT_THRESHOLD)
                {
                    SoftOverCurrentShutdown(0, I_Sense_Pos);
                    timer_standby = TT_FOR_CURRENT_ERROR;
                    ac_sync_state = OVERCURRENT_ERROR;
                    break;
                }
#endif
                
#ifdef USE_LED_FOR_PID_CALCS
                LED_ON;
#endif
                //Adelanto la seniales de corriente,
                if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
                {
                    unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
                    //loop de corriente
                    unsigned int calc = *p_current_ref * KI_SIGNAL_PEAK_MULTIPLIER;
                    calc = calc >> 10;

                    switch (signal_state)
                    {
                    case SIGNAL_RISING:
                        d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
                        HIGH_LEFT(d);

                        if (signal_index > INDEX_TO_MIDDLE)
                        {
                            CurrentLoop_Change_to_HighGain();
                            signal_state = SIGNAL_MIDDLE;
                        }
                        break;

                    case SIGNAL_MIDDLE:
                        d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
                        HIGH_LEFT(d);

                        if (signal_index > INDEX_TO_FALLING)
                        {
                            CurrentLoop_Change_to_LowGain();
                            signal_state = SIGNAL_FALLING;
                        }
                        break;

                    case SIGNAL_FALLING:
                        d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
                        HIGH_LEFT(d);

                        if (signal_index > INDEX_TO_REVERT)
                        {
                            // CurrentLoop_Change_to_LowGain();
                            signal_state = SIGNAL_REVERT;
                            HIGH_LEFT(DUTY_NONE);
                        }
                        break;

                    case SIGNAL_REVERT:
                        // d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
                        // HIGH_LEFT(d);

                        // if (signal_index > 204)
                        // {
                        //     CurrentLoop_Change_to_LowGain();
                        //     signal_state = SIGNAL_REVERT;
                        // }
                        break;
                        
                    }
                    
                    p_current_ref++;
                }
                else
                    //termino de generar la senoidal, corto el mosfet
                    LOW_RIGHT(DUTY_NONE);
                
#ifdef USE_LED_FOR_PID_CALCS
                LED_OFF;
#endif
            }    //end of sequence_ready
            
            if (SYNC_Sync_Now())
            {
                ac_sync_state = WAIT_CROSS_POS_TO_NEG;
                SYNC_Sync_Now_Reset();

                HIGH_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_ALWAYS);
                sequence_ready_reset;
            }
            else if (!SYNC_All_Good())
            {
                PWM_Off();
#ifdef USE_LED_FOR_PROTECTIONS
                LED_ON;
#endif
                RELAY_OFF;
                ac_sync_state = START_SYNCING;

                //aviso el tipo de error
                if (!SYNC_Frequency_Check())
                    sprintf(s_lcd, "bad freq pos: %d\n", SYNC_delta_t2());

                else if (!SYNC_Pulses_Check())
                    sprintf(s_lcd, "bad sync pulses pos: %d\n", SYNC_delta_t1());

                else if (!SYNC_Vline_Check())
                    sprintf(s_lcd, "bad vline voltage pos: %d\n", SYNC_vline_max());
                else
                    sprintf(s_lcd, "unknow error in pos\n");
                
                Usart1Send(s_lcd);
#ifdef USE_LED_FOR_PROTECTIONS
                LED_OFF;
#endif                
                SYNC_Sync_Now_Reset();
            }
            break;

        case WAIT_CROSS_POS_TO_NEG:
            //espero un sequence_ready para asegurar valores conocidos en el pwm
            if (sequence_ready)
            {
                sequence_ready_reset;
                if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
                    PID_Flush_Errors(&current_pid);
                    p_current_ref = sin_half_cycle;
                    CurrentLoop_Change_to_LowGain();
                    signal_state = SIGNAL_RISING;
                    ac_sync_state = GEN_NEG;
                    
#ifdef USE_LED_FOR_MAIN_POLARITY                
                    LED_OFF;
#endif                
                }
                else    //debe haber un error de synchro
                    ac_sync_state = START_SYNCING;
            }
            break;
            
        case GEN_NEG:
            if (sequence_ready)
            {
                sequence_ready_reset;

#ifdef WITH_SOFT_OVERCURRENT_SHUTDOWN
                if (I_Sense_Neg > SOFT_OVERCURRENT_THRESHOLD)
                {
                    SoftOverCurrentShutdown(1, I_Sense_Neg);
                    timer_standby = TT_FOR_CURRENT_ERROR;
                    ac_sync_state = OVERCURRENT_ERROR;
                    break;
                }
#endif
                
#ifdef USE_LED_FOR_PID_CALCS
                LED_ON;
#endif
                //Adelanto la senial de corriente,
                if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
                {
                    unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
                    //loop de corriente
                    unsigned int calc = *p_current_ref * KI_SIGNAL_PEAK_MULTIPLIER;
                    calc = calc >> 10;

                    switch (signal_state)
                    {
                    case SIGNAL_RISING:
                        d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
                        HIGH_RIGHT(d);

                        if (signal_index > INDEX_TO_MIDDLE)
                        {
                            CurrentLoop_Change_to_HighGain();
                            signal_state = SIGNAL_MIDDLE;
                        }
                        break;

                    case SIGNAL_MIDDLE:
                        d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
                        HIGH_RIGHT(d);

                        if (signal_index > INDEX_TO_FALLING)
                        {
                            CurrentLoop_Change_to_LowGain();
                            signal_state = SIGNAL_FALLING;
                        }
                        break;

                    case SIGNAL_FALLING:
                        d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
                        HIGH_RIGHT(d);

                        if (signal_index > INDEX_TO_REVERT)
                        {
                            // CurrentLoop_Change_to_LowGain();
                            signal_state = SIGNAL_REVERT;
                            HIGH_RIGHT(DUTY_NONE);
                        }
                        break;

                    case SIGNAL_REVERT:
                        // d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
                        // HIGH_RIGHT(d);

                        // if (signal_index > 204)
                        // {
                        //     CurrentLoop_Change_to_LowGain();
                        //     signal_state = SIGNAL_REVERT;
                        // }
                        break;
                        
                    }
                    
                    p_current_ref++;
                }
                else
                    //termino de generar la senoidal, corto el mosfet
                    LOW_LEFT(DUTY_NONE);
                
#ifdef USE_LED_FOR_PID_CALCS
                LED_OFF;
#endif                
            }    //end of sequence_ready

            //reviso todo el tiempo si debo cambiar de ciclo o si todo sigue bien
            if (SYNC_Sync_Now())
            {
                ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                SYNC_Sync_Now_Reset();

                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_ALWAYS);
                sequence_ready_reset;
            }
            else if (!SYNC_All_Good())
            {
                PWM_Off();
#ifdef USE_LED_FOR_PROTECTIONS
                LED_ON;
#endif                
                RELAY_OFF;
                ac_sync_state = START_SYNCING;

                //aviso el tipo de error
                if (!SYNC_Frequency_Check())
                    sprintf(s_lcd, "bad freq neg: %d\n", SYNC_delta_t2());

                else if (!SYNC_Pulses_Check())
                    sprintf(s_lcd, "bad sync pulses neg: %d\n", SYNC_delta_t1());

                else if (!SYNC_Vline_Check())
                    sprintf(s_lcd, "bad vline voltage neg: %d\n", SYNC_vline_max());
                else
                    sprintf(s_lcd, "unknow error in neg\n");
                
                Usart1Send(s_lcd);
#ifdef USE_LED_FOR_PROTECTIONS
                LED_OFF;
#endif                
                SYNC_Sync_Now_Reset();
            }
            
            break;

        case WAIT_CROSS_NEG_TO_POS:
            //espero un sequence_ready para asegurar valores conocidos en el pwm
            if (sequence_ready)
            {
                sequence_ready_reset;
                if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
                {
                    PID_Flush_Errors(&current_pid);
                    p_current_ref = sin_half_cycle;
                    CurrentLoop_Change_to_LowGain();
                    signal_state = SIGNAL_RISING;
                    ac_sync_state = GEN_POS;
                
#ifdef USE_LED_FOR_MAIN_POLARITY                
                    LED_ON;
#endif
                }
                else    //debe haber un error de synchro
                    ac_sync_state = START_SYNCING;
            }
            break;
            
        case JUMPER_PROTECTED:
            if (!timer_standby)
            {
                if (!STOP_JUMPER)
                {
                    ac_sync_state = JUMPER_PROTECT_OFF;
                    timer_standby = 400;
                }
            }                
            break;

        case JUMPER_PROTECT_OFF:
            if (!timer_standby)
            {
                //vuelvo a INIT
                ac_sync_state = START_SYNCING;
                Usart1Send((char *) "Protect OFF\n");                    
            }                
            break;            

        case OVERCURRENT_ERROR:
            if (!timer_standby)
            {
                ChangeLed(LED_STANDBY);
                ac_sync_state = START_SYNCING;
            }
            break;

        }

        SYNC_Update_Sync();
        SYNC_Update_Polarity();

        if (SYNC_Cycles_Cnt() > 100)
        {
            SYNC_Cycles_Cnt_Reset();
            sprintf(s_lcd, "d_t1_bar: %d d_t2: %d pol: %d st: %d vline: %d\n",
                    SYNC_delta_t1_half_bar(),
                    SYNC_delta_t2_bar(),
                    SYNC_Last_Polarity_Check(),
                    ac_sync_state,
                    SYNC_Vline_Max());
            Usart1Send(s_lcd);            
        }

        //Cosas que no tienen tanto que ver con las muestras o el estado del programa
        if ((STOP_JUMPER) &&
            (ac_sync_state != JUMPER_PROTECTED) &&
            (ac_sync_state != JUMPER_PROTECT_OFF) &&            
            (ac_sync_state != OVERCURRENT_ERROR))
        {
            RELAY_OFF;
            PWM_Off();
            
            ChangeLed(LED_JUMPER_PROTECTED);
            Usart1Send((char *) "Protect ON\n");
            timer_standby = TT_FOR_JUMPER_PROT;
            ac_sync_state = JUMPER_PROTECTED;
        }
        
        if (overcurrent_shutdown)
        {
            RELAY_OFF;
            if (overcurrent_shutdown == 1)
            {
                Usart1Send("Overcurrent POS\n");
                ChangeLed(LED_OVERCURRENT_POS);
            }
            else
            {
                Usart1Send("Overcurrent NEG\n");
                ChangeLed(LED_OVERCURRENT_NEG);
            }
            overcurrent_shutdown = 0;

            timer_standby = TT_FOR_CURRENT_ERROR;
            ac_sync_state = OVERCURRENT_ERROR;
        }

#ifdef USE_LED_FOR_MAIN_STATES
        UpdateLed();
#endif
    }
    
#endif    // GRID_TIE_FULL_CONNECTED
    
    return 0;
}

//--- End of Main ---//


void SoftOverCurrentShutdown (unsigned char side, unsigned short current)
{
    PWM_Off();
    RELAY_OFF;

    char s_send [100];

    sprintf(s_send, "Soft overcurrent: %d", current);
    Usart1Send(s_send);
    if (!side)
        Usart1Send(" POS side\n");
    else
        Usart1Send(" NEG side\n");
}


void PWM_Off (void)
{
    DisablePreload_Mosfet_HighLeft;
    DisablePreload_Mosfet_HighRight;

    LOW_LEFT(DUTY_NONE);
    HIGH_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);

    EnablePreload_Mosfet_HighLeft;
    EnablePreload_Mosfet_HighRight;
}


unsigned short CurrentLoop (unsigned short setpoint, unsigned short new_sample)
{
    short d = 0;
    
    current_pid.setpoint = setpoint;
    current_pid.sample = new_sample;
    d = PID(&current_pid);
                    
    if (d > 0)
    {
        if (d > DUTY_100_PERCENT)
        {
            d = DUTY_100_PERCENT;
            current_pid.last_d = DUTY_100_PERCENT;
        }
    }
    else
    {
        d = DUTY_NONE;
        current_pid.last_d = DUTY_NONE;
    }

    return (unsigned short) d;
}


void CurrentLoop_Change_to_HighGain (void)
{
    current_pid.kp = 10;
    current_pid.ki = 3;
    current_pid.kd = 0;
}


void CurrentLoop_Change_to_LowGain (void)
{
    current_pid.kp = 10;
    current_pid.ki = 3;
    current_pid.kd = 0;    
}


void TimingDelay_Decrement(void)
{
    if (wait_ms_var)
        wait_ms_var--;

    if (timer_standby)
        timer_standby--;

    if (timer_led)
        timer_led--;

}


#define AC_SYNC_Int        (EXTI->PR & 0x00000100)
#define AC_SYNC_Set        (EXTI->IMR |= 0x00000100)
#define AC_SYNC_Reset      (EXTI->IMR &= ~0x00000100)
#define AC_SYNC_Ack        (EXTI->PR |= 0x00000100)

#define AC_SYNC_Int_Rising          (EXTI->RTSR & 0x00000100)
#define AC_SYNC_Int_Rising_Set      (EXTI->RTSR |= 0x00000100)
#define AC_SYNC_Int_Rising_Reset    (EXTI->RTSR &= ~0x00000100)

#define AC_SYNC_Int_Falling          (EXTI->FTSR & 0x00000100)
#define AC_SYNC_Int_Falling_Set      (EXTI->FTSR |= 0x00000100)
#define AC_SYNC_Int_Falling_Reset    (EXTI->FTSR &= ~0x00000100)

#define OVERCURRENT_POS_Int        (EXTI->PR & 0x00000010)
#define OVERCURRENT_POS_Ack        (EXTI->PR |= 0x00000010)
#define OVERCURRENT_NEG_Int        (EXTI->PR & 0x00000020)
#define OVERCURRENT_NEG_Ack        (EXTI->PR |= 0x00000020)

// #define TIM16_HYSTERESIS    100
void EXTI4_15_IRQHandler(void)
{
#ifdef WITH_AC_SYNC_INT
    if (AC_SYNC_Int)
    {
        if (AC_SYNC_Int_Rising)
        {
            AC_SYNC_Int_Rising_Reset;
            AC_SYNC_Int_Falling_Set;

            SYNC_Rising_Edge_Handler();
#ifdef USE_LED_FOR_AC_PULSES
            LED_ON;
#endif
        }
        else if (AC_SYNC_Int_Falling)
        {
            AC_SYNC_Int_Falling_Reset;
            AC_SYNC_Int_Rising_Set;
            
            SYNC_Falling_Edge_Handler();
#ifdef USE_LED_FOR_AC_PULSES            
            LED_OFF;
#endif
        }
        AC_SYNC_Ack;
    }
#endif
    
#ifdef WITH_OVERCURRENT_SHUTDOWN
    if (OVERCURRENT_POS_Int)
    {
        DisablePreload_Mosfet_HighLeft;
        HIGH_LEFT(DUTY_NONE);
        //TODO: trabar el TIM3 aca!!!
        overcurrent_shutdown = 1;
        OVERCURRENT_POS_Ack;
    }

    if (OVERCURRENT_NEG_Int)
    {
        DisablePreload_Mosfet_HighRight;
        HIGH_RIGHT(DUTY_NONE);
        //TODO: trabar el TIM3 aca!!!
        overcurrent_shutdown = 2;
        OVERCURRENT_NEG_Ack;
    }
#endif
}


void SysTickError (void)
{
    //Capture systick error...
    while (1)
    {
        if (LED)
            LED_OFF;
        else
            LED_ON;

        for (unsigned char i = 0; i < 255; i++)
        {
            asm ("nop \n\t"
                 "nop \n\t"
                 "nop \n\t" );
        }
    }
}

//--- end of file ---//
