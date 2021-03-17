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
#include "pwm_defs.h"

#include "core_cm0.h"
#include "adc.h"
#include "dma.h"
#include "flash_program.h"
#include "dsp.h"
#include "it.h"
#include "sync.h"
#include "test_functions.h"
#include "gen_signal.h"

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


// Select Current Signal
#ifndef USE_SIGNAL_CONTROL_PRE_DISTORTED
// #define USE_SIGNAL_CURRENT_005_A
// #define USE_SIGNAL_CURRENT_01_A
// #define USE_SIGNAL_CURRENT_02_A
// #define USE_SIGNAL_CURRENT_03_A
// #define USE_SIGNAL_CURRENT_04_A
#define USE_SIGNAL_CURRENT_05_A
// #define USE_SIGNAL_CURRENT_06_A
// #define USE_SIGNAL_CURRENT_075_A
// #define USE_SIGNAL_CURRENT_1_A
#endif

// Set of Peak Current in ADC points, 12bits
// with Aopamp = 9.2
// Voltage in I_Sense is 3 . Ipeak
// ADC points = 3 . Ipeak . 4095 / 3.3
// with Aopamp = 4.4
// Voltage in I_Sense is 1.45 . Ipeak
// ADC points = 1.45 . Ipeak . 4095 / 3.3
// #define AOPAMP_9_2
#define AOPAMP_4_4

#ifdef AOPAMP_4_4
#ifdef USE_SIGNAL_CURRENT_005_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    90
#endif

#ifdef USE_SIGNAL_CURRENT_01_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    180
#endif

#ifdef USE_SIGNAL_CURRENT_02_A
#define KI_SIGNAL_PEAK_MULTIPLIER    360
#endif

#ifdef USE_SIGNAL_CURRENT_03_A
#define KI_SIGNAL_PEAK_MULTIPLIER    540
#endif

#ifdef USE_SIGNAL_CURRENT_04_A
#define KI_SIGNAL_PEAK_MULTIPLIER    720
#endif

#ifdef USE_SIGNAL_CURRENT_05_A
#define KI_SIGNAL_PEAK_MULTIPLIER    900
#endif

#ifdef USE_SIGNAL_CURRENT_06_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1080
#endif

#ifdef USE_SIGNAL_CURRENT_075_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1350
#endif

#ifdef USE_SIGNAL_CURRENT_1_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1800
#endif
#endif    //AOPAMP_4_4

#ifdef AOPAMP_9_2
#ifdef USE_SIGNAL_CURRENT_005_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    170
#endif

#ifdef USE_SIGNAL_CURRENT_01_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    373
#endif

#ifdef USE_SIGNAL_CURRENT_02_A
#define KI_SIGNAL_PEAK_MULTIPLIER    746
#endif

#ifdef USE_SIGNAL_CURRENT_03_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1119
#endif

#ifdef USE_SIGNAL_CURRENT_04_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1492
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
#endif    //AOPAMP_9_2

//for pre distorted tests
#ifdef USE_SIGNAL_CONTROL_PRE_DISTORTED
#define KI_SIGNAL_PEAK_MULTIPLIER    1384
#endif


#ifdef WITH_FEW_CYCLES_OF_50HZ
unsigned short d_dump [SIZEOF_SIGNAL] = { 0 };
unsigned short i_dump [SIZEOF_SIGNAL] = { 0 };
unsigned short r_dump [SIZEOF_SIGNAL] = { 0 };
#endif

// Module Private Functions ----------------------------------------------------
void SysTickError (void);
void SoftOverCurrentShutdown (unsigned char, unsigned short);
void PWM_Off (void);
void TimingDelay_Decrement (void);
void EXTI4_15_IRQHandler(void);
void CalcCurrentMode (unsigned short duty);
unsigned char GetCurrentMode (void);


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
    // TF_OnlySyncAndPolarity();
    // TF_Check_Sequence_Ready();
            
    // Main Program - Grid Tied Mode -
#ifdef GRID_TIED_FULL_CONECTED

    unsigned short cycles_before_start = CYCLES_BEFORE_START;
    short d = 0;


#ifdef WITH_FEW_CYCLES_OF_50HZ
    unsigned char cycles_50hz = CYCLES_OF_50HZ;
#endif
    
    while (1)
    {
        switch (ac_sync_state)
        {
        case START_SYNCING:
            PWM_Off();
            LED_OFF;

            // EnablePreload_Mosfet_HighLeft;
            // EnablePreload_Mosfet_HighRight;

            SYNC_Restart();
            cycles_before_start = CYCLES_BEFORE_START;
            ac_sync_state = SWITCH_RELAY_TO_ON;
#ifdef WITH_FEW_CYCLES_OF_50HZ
            cycles_50hz = CYCLES_OF_50HZ;
            for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
            {
                d_dump[i] = 0;
                i_dump[i] = 0;
                r_dump[i] = 0;
            }
#endif            
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
#ifdef USE_LED_FOR_MAIN_POLARITY_BEFORE_GEN
                    //polarity is positive now light the LED
                    LED_ON;
#endif
                }
                else if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
#ifdef USE_LED_FOR_MAIN_POLARITY_BEFORE_GEN
                    //polarity is negative now light off the LED
                    LED_OFF;
#endif

                    //check if we are ready to start
                    if (cycles_before_start)
                        cycles_before_start--;
                    else
                        ac_sync_state = WAIT_FOR_FIRST_SYNC;
                }
                else    //it must be an error in synchro
                    ac_sync_state = START_SYNCING;
                
                SYNC_Sync_Now_Reset();
            }            
            break;
            
        case WAIT_FOR_FIRST_SYNC:
            //for security reasons always start with positive
            if (SYNC_Sync_Now())
            {
                if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
                {
                    ChangeLed(LED_GENERATING);
                    ac_sync_state = WAIT_CROSS_NEG_TO_POS;

#ifndef GRID_TIED_ONLY_SYNC_AND_POLARITY
                    HIGH_RIGHT(DUTY_NONE);
                    LOW_LEFT(DUTY_NONE);
                    LOW_RIGHT(DUTY_ALWAYS);
#endif

#ifdef USE_SIGNAL_CONTROL_SINUS
                    GenSignalSinusResetCntrs();
#endif
#ifdef USE_SIGNAL_CONTROL_SINUS2
                    GenSignalSinus2ResetCntrs();
#endif
#ifdef USE_SIGNAL_CONTROL_TRIANG
                    GenSignalTriangResetCntrs();
#endif
                    
                    sequence_ready_reset;
                }
                else if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
                    //do nothing
                    //start by positive
                }
                else    //it must be an error in synchro
                    ac_sync_state = START_SYNCING;
                
                SYNC_Sync_Now_Reset();
            }
            break;
        
        case GEN_POS:
            if (sequence_ready)
            {
                sequence_ready_reset;

#ifdef USE_LED_FOR_PID_CALCS
                LED_ON;
#endif
                
#ifdef WITH_SOFT_OVERCURRENT_SHUTDOWN
                if (I_Sense_Pos > SOFT_OVERCURRENT_THRESHOLD)
                {
                    SoftOverCurrentShutdown(0, I_Sense_Pos);
                    timer_standby = TT_FOR_CURRENT_ERROR;
                    ac_sync_state = OVERCURRENT_ERROR;
                    break;
                }
#endif
                gen_signal_e resp = SIGNAL_RUNNING;
#ifdef USE_SIGNAL_CONTROL_BY_PID
                resp = GenSignal(I_Sense_Pos, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif
#ifdef USE_SIGNAL_CONTROL_PRE_DISTORTED
                resp = GenSignalPreDistorted(I_Sense_Pos, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif
#ifdef USE_SIGNAL_CONTROL_SINUS
                resp = GenSignalSinus(I_Sense_Pos, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif
#ifdef USE_SIGNAL_CONTROL_SINUS2
                resp = GenSignalSinus2(I_Sense_Pos, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif
#ifdef USE_SIGNAL_CONTROL_TRIANG
                resp = GenSignalTriang(I_Sense_Pos, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif

                if (resp == SIGNAL_FINISH)
                {
                    //end of generation, wait for a sync pulse
#ifndef GRID_TIED_ONLY_SYNC_AND_POLARITY
                    HIGH_LEFT(DUTY_NONE);
#endif
                }
                else if (resp == SIGNAL_RUNNING)
                {
#ifndef GRID_TIED_ONLY_SYNC_AND_POLARITY
                    HIGH_LEFT(d);
#endif
                }
                else
                {
                }
                    
                
#ifdef USE_LED_FOR_PID_CALCS
                LED_OFF;
#endif
            }    //end of sequence_ready

            //check all the time for a new sync pulse
            if (SYNC_Sync_Now())
            {
                ac_sync_state = WAIT_CROSS_POS_TO_NEG;
                SYNC_Sync_Now_Reset();
#ifndef GRID_TIED_ONLY_SYNC_AND_POLARITY
                HIGH_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_ALWAYS);
#endif
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

#if (defined WITH_FEW_CYCLES_OF_50HZ) && (defined WITH_FEW_CYCLES_OF_50HZ_POS)
                if (cycles_50hz)
                    cycles_50hz--;
                else
                {
                    ac_sync_state = FEW_CYCLES_DUMP_DATA;
                    break;
                }
#endif
                
                if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
#ifdef USE_SIGNAL_CONTROL_BY_PID
                    GenSignalReset();
#endif
#ifdef USE_SIGNAL_CONTROL_PRE_DISTORTED
                    GenSignalPreDistortedReset();
#endif
#ifdef USE_SIGNAL_CONTROL_SINUS
                    GenSignalSinusReset();
#endif
#ifdef  USE_SIGNAL_CONTROL_SINUS2
                    GenSignalSinus2Reset();
#endif
#ifdef USE_SIGNAL_CONTROL_TRIANG
                    GenSignalTriangReset();
#endif
                    ac_sync_state = GEN_NEG;
                    
#ifdef USE_LED_FOR_MAIN_POLARITY                
                    LED_OFF;
#endif                
                }
                else    //it must be an error in synchro
                    ac_sync_state = START_SYNCING;
            }
            break;
            
        case GEN_NEG:
            if (sequence_ready)
            {
                sequence_ready_reset;

#ifdef USE_LED_FOR_PID_CALCS
                LED_ON;
#endif

#ifdef WITH_SOFT_OVERCURRENT_SHUTDOWN
                if (I_Sense_Neg > SOFT_OVERCURRENT_THRESHOLD)
                {
                    SoftOverCurrentShutdown(1, I_Sense_Neg);
                    timer_standby = TT_FOR_CURRENT_ERROR;
                    ac_sync_state = OVERCURRENT_ERROR;
                    break;
                }
#endif
                
                gen_signal_e resp = SIGNAL_RUNNING;
#ifdef USE_SIGNAL_CONTROL_BY_PID
                resp = GenSignal(I_Sense_Neg, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif
#ifdef USE_SIGNAL_CONTROL_PRE_DISTORTED
                resp = GenSignalPreDistorted(I_Sense_Neg, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif
#ifdef USE_SIGNAL_CONTROL_SINUS
                resp = GenSignalSinus(I_Sense_Neg, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif                
#ifdef USE_SIGNAL_CONTROL_SINUS2
                resp = GenSignalSinus2(I_Sense_Neg, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif                
#ifdef USE_SIGNAL_CONTROL_TRIANG
                resp = GenSignalTriang(I_Sense_Pos, KI_SIGNAL_PEAK_MULTIPLIER, &d);
#endif

                if (resp == SIGNAL_FINISH)
                {
                    //end of generation, wait for a sync pulse
#ifndef GRID_TIED_ONLY_SYNC_AND_POLARITY
                    HIGH_RIGHT(DUTY_NONE);
#endif
                }
                else if (resp == SIGNAL_RUNNING)
                {
#ifndef GRID_TIED_ONLY_SYNC_AND_POLARITY
                    HIGH_RIGHT(d);
#endif
                }
                else
                {
                }
                
#ifdef USE_LED_FOR_PID_CALCS
                LED_OFF;
#endif                
            }    //end of sequence_ready

            //check all the time for a new sync pulse
            if (SYNC_Sync_Now())
            {
                ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                SYNC_Sync_Now_Reset();
#ifndef GRID_TIED_ONLY_SYNC_AND_POLARITY
                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_ALWAYS);
#endif
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
            //wait for sequence_ready to get proper values in the pwm
            if (sequence_ready)
            {
                sequence_ready_reset;
#if (defined WITH_FEW_CYCLES_OF_50HZ) && (defined WITH_FEW_CYCLES_OF_50HZ_NEG)
                if (cycles_50hz)
                    cycles_50hz--;
                else
                {
                    ac_sync_state = FEW_CYCLES_DUMP_DATA;
                    break;
                }
#endif
                if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
                {
#ifdef USE_SIGNAL_CONTROL_BY_PID
                    GenSignalReset();
#endif
#ifdef USE_SIGNAL_CONTROL_PRE_DISTORTED
                    GenSignalPreDistortedReset();
#endif
#ifdef USE_SIGNAL_CONTROL_SINUS
                    GenSignalSinusReset();
#endif
#ifdef  USE_SIGNAL_CONTROL_SINUS2
                    GenSignalSinus2Reset();
#endif
#ifdef USE_SIGNAL_CONTROL_TRIANG
                    GenSignalTriangReset();
#endif
                    ac_sync_state = GEN_POS;
                
#ifdef USE_LED_FOR_MAIN_POLARITY                
                    LED_ON;
#endif
                }
                else    //it must be an error in synchro
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

        case FEW_CYCLES_DUMP_DATA:
#ifdef WITH_FEW_CYCLES_OF_50HZ
            RELAY_OFF;
            PWM_Off();
            ac_sync_state = FEW_CYCLES_DUMP_DATA_1;
#endif
            break;

        case FEW_CYCLES_DUMP_DATA_1:
#ifdef WITH_FEW_CYCLES_OF_50HZ
#ifdef WITH_FEW_CYCLES_OF_50HZ_POS
            Usart1Send("d_dump data positive:\n");            
#endif
#ifdef WITH_FEW_CYCLES_OF_50HZ_NEG
            Usart1Send("d_dump data negative:\n");
#endif
            Wait_ms(20);
            for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
            {
                sprintf(s_lcd, "%03d -> r: %d i: %d d: %d\n",
                        i,
                        *(r_dump + i),
                        *(i_dump + i),                        
                        *(d_dump + i));

                Usart1Send(s_lcd);
                Wait_ms(40);
                        
            }
            timer_standby = TT_FEW_CYCLES_DUMP;
            ac_sync_state = FEW_CYCLES_DUMP_DATA_2;
#endif
            break;

        case FEW_CYCLES_DUMP_DATA_2:
#ifdef WITH_FEW_CYCLES_OF_50HZ
            if (!timer_standby)
            {
                ChangeLed(LED_STANDBY);
                ac_sync_state = START_SYNCING;
            }
#endif
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

    char s_send [50];

    sprintf(s_send, "Soft overcurrent: %d", current);
    Usart1Send(s_send);
    if (!side)
        Usart1Send(" POS side\n");
    else
        Usart1Send(" NEG side\n");
}


void PWM_Off (void)
{
    // DisablePreload_Mosfet_HighLeft;
    // DisablePreload_Mosfet_HighRight;

    HIGH_RIGHT(DUTY_NONE);
    HIGH_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    LOW_LEFT(DUTY_NONE);

    // EnablePreload_Mosfet_HighLeft;
    // EnablePreload_Mosfet_HighRight;
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
        // DisablePreload_Mosfet_HighLeft;
        HIGH_LEFT(DUTY_NONE);
        //TODO: trabar el TIM3 aca!!!
        overcurrent_shutdown = 1;
        OVERCURRENT_POS_Ack;
    }

    if (OVERCURRENT_NEG_Int)
    {
        // DisablePreload_Mosfet_HighRight;
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
