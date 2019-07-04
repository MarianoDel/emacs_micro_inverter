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


// Externals -----------------------------------------------

// -- Externals from or for Serial Port --------------------
volatile unsigned char tx1buff [SIZEOF_DATA];
volatile unsigned char rx1buff [SIZEOF_DATA];
volatile unsigned char usart1_have_data = 0;

// -- Externals from or for the ADC ------------------------
volatile unsigned short adc_ch [ADC_CHANNEL_QUANTITY];
volatile unsigned char seq_ready = 0;

// -- Externals for the timers -----------------------------
volatile unsigned short timer_led = 0;

// -- Externals used for analog or digital filters ---------
volatile unsigned short take_temp_sample = 0;

// -- Externals for synchro --------------------------------
#ifdef INVERTER_ONLY_SYNC_AND_POLARITY
volatile unsigned short timer_no_sync = 0;
volatile unsigned short delta_t1 = 0;
volatile unsigned short delta_t1_bar = 0;
volatile unsigned short delta_t2 = 0;
#endif

// ------- Definiciones para los filtros -------
#define SIZEOF_FILTER    8
unsigned short vin_vector [SIZEOF_FILTER];
#ifdef USE_FREQ_75KHZ
#define UNDERSAMPLING_TICKS    45
#define UNDERSAMPLING_TICKS_SOFT_START    90
#endif
#ifdef USE_FREQ_48KHZ
#define UNDERSAMPLING_TICKS    10
#define UNDERSAMPLING_TICKS_SOFT_START    20
#endif


// Globals -------------------------------------------------
volatile unsigned char overcurrent_shutdown = 0;
// volatile short d = 0;
// short ez1 = 0;
// short ez2 = 0;
// // unsigned short dmax = 0;
// unsigned short last_d = 0;
// #define DELTA_D    2


// ------- de los timers -------
volatile unsigned short wait_ms_var = 0;
volatile unsigned short timer_standby;
//volatile unsigned char display_timer;
volatile unsigned short timer_meas;
volatile unsigned char timer_filters = 0;

volatile unsigned short dmax_permited = 0;
// volatile unsigned short secs = 0;
// volatile unsigned char hours = 0;
// volatile unsigned char minutes = 0;

// #define USE_SIGNAL_SINUSOIDAL
// #define USE_SIGNAL_TRIANGULAR
// #define USE_SIGNAL_CURRENT_SIN_0_5_A
#define USE_SIGNAL_CURRENT_SIN_1_A
// #define USE_SIGNAL_MODIFIED_SIN

#if (defined USE_SIGNAL_CURRENT_SIN_0_5_A) || (defined USE_SIGNAL_CURRENT_SIN_1_A)
#define SIZEOF_SIGNAL 120
#endif
#if (defined USE_SIGNAL_SINUSOIDAL) || (defined USE_SIGNAL_TRIANGULAR) || (defined USE_SIGNAL_MODIFIED_SIN)
#define SIZEOF_SIGNAL 50
#endif

#ifdef USE_SIGNAL_CURRENT_SIN_1_A
unsigned short mem_signal [SIZEOF_SIGNAL] = {26,53,80,106,133,160,186,212,238,264,
                                             290,316,341,366,391,416,440,464,488,511,
                                             534,557,579,601,622,643,664,684,704,723,
                                             742,760,777,795,811,827,843,857,872,885,
                                             899,911,923,934,945,955,964,972,980,988,
                                             994,1000,1005,1010,1014,1017,1019,1021,1022,1023,
                                             1022,1021,1019,1017,1014,1010,1005,1000,994,988,
                                             980,972,964,955,945,934,923,911,899,885,
                                             872,857,843,827,811,795,777,760,742,723,
                                             704,684,664,643,622,601,579,557,534,511,
                                             488,464,440,416,391,366,341,316,290,264,
                                             238,212,186,160,133,106,80,53,26,0};
#endif
#ifdef USE_SIGNAL_CURRENT_SIN_0_5_A
unsigned short mem_signal [SIZEOF_SIGNAL] = {13,26,40,53,66,79,93,106,119,132,
                                             145,157,170,183,195,207,219,231,243,255,
                                             266,278,289,300,311,321,331,341,351,361,
                                             370,379,388,397,405,413,421,428,435,442,
                                             449,455,461,466,472,477,481,485,489,493,
                                             496,499,502,504,506,508,509,510,510,511,
                                             510,510,509,508,506,504,502,499,496,493,
                                             489,485,481,477,472,466,461,455,449,442,
                                             435,428,421,413,405,397,388,379,370,361,
                                             351,341,331,321,311,300,289,278,266,255,
                                             243,231,219,207,195,183,170,157,145,132,
                                             119,106,93,79,66,53,40,26,13,0};
#endif
#ifdef USE_SIGNAL_SINUSOIDAL
unsigned short mem_signal [SIZEOF_SIGNAL] = {62,125,187,248,309,368,425,481,535,587,
                                             637,684,728,770,809,844,876,904,929,951,
                                             968,982,992,998,1000,998,992,982,968,951,
                                             929,904,876,844,809,770,728,684,637,587,
                                             535,481,425,368,309,248,187,125,62,0};
#endif
#ifdef USE_SIGNAL_TRIANGULAR
unsigned short mem_signal [SIZEOF_SIGNAL] = {40,80,120,160,200,240,280,320,360,400,
                                             440,480,520,560,600,640,680,720,760,800,
                                             840,880,920,960,1000,960,920,880,840,800,
                                             760,720,680,640,600,560,520,480,440,400,
                                             360,320,280,240,200,160,120,80,40,0};
#endif
#ifdef USE_SIGNAL_MODIFIED_SIN
unsigned short mem_signal [SIZEOF_SIGNAL] = {0,0,0,0,333,333,333,333,333,333,
                                             333,333,666,666,666,666,666,666,666,666,
                                             1000,1000,1000,1000,1000,1000,1000,1000,666,666,
                                             666,666,666,666,666,666,333,333,333,333,
                                             333,333,333,333,0,0,0,0,0,0};

#endif

unsigned short * p_signal;



// Module Functions ----------------------------------------
void TimingDelay_Decrement (void);
extern void EXTI4_15_IRQHandler(void);



//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
    unsigned char i;
    unsigned short ii;

    char s_lcd [120];

    ac_sync_state_t ac_sync_state = START_SYNCING;


    //GPIO Configuration.
    GPIO_Config();

    //ACTIVAR SYSTICK TIMER
    if (SysTick_Config(48000))
    {
        while (1)	/* Capture error */
        {
            if (LED)
                LED_OFF;
            else
                LED_ON;

            for (i = 0; i < 255; i++)
            {
                asm (	"nop \n\t"
                        "nop \n\t"
                        "nop \n\t" );
            }
        }
    }

//---------- Pruebas de Hardware --------//
    EXTIOff ();
    USART1Config();
    
    //---- Welcome Code ------------//
    //---- Defines from hard.h -----//
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
    MA32Circular_Reset();
    
    LOW_LEFT(DUTY_NONE);
    HIGH_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);

    //ADC and DMA configuration
    AdcConfig();
    DMAConfig();
    DMA1_Channel1->CCR |= DMA_CCR_EN;
    ADC1->CR |= ADC_CR_ADSTART;
    //end of ADC & DMA

    EXTIOn();

#ifdef INVERTER_MODE_CURRENT_FDBK
    short d = 0;
    // short last_d = 0;
    short ez1 = 0;
    short ez2 = 0;
    
    while (1)
    {
        switch (ac_sync_state)
        {
        case START_SYNCING:
            //Check voltage and polarity
            // if ((Voltage_is_Good()) && Polarity_is_Good())
            LED_OFF;            
            RELAY_ON;
            timer_standby = 200;            
            ac_sync_state++;
            break;

        case WAIT_RELAY_TO_ON:
            if (!timer_standby)
            {
                ac_sync_state++;
            }
            break;

        case WAIT_FOR_FIRST_SYNC:
            ac_sync_state = WAIT_CROSS_NEG_TO_POS;
            d = 0;
            // last_d = 0;
            ez1 = 0;
            ez2 = 0;
            HIGH_RIGHT(DUTY_NONE);
            LOW_LEFT(DUTY_NONE);
            LOW_RIGHT(DUTY_ALWAYS);
            break;
        
        case GEN_POS:
            if (sequence_ready)
            {
                sequence_ready_reset;
                //aca la senial (el ultimo punto) termina en 0
                if (p_signal < &mem_signal[(SIZEOF_SIGNAL - 1)])
                {
                    p_signal++;

                    d = PID_roof(*p_signal, I_Sense_Pos, d, &ez1, &ez2);
                    
                    if (d > 0)
                    {
                        if (d < DUTY_100_PERCENT)
                            HIGH_LEFT(d);
                        else
                        {
                            HIGH_LEFT(DUTY_100_PERCENT);
                            d = DUTY_100_PERCENT;
                        }
                    }
                    else
                    {
                        HIGH_LEFT(DUTY_NONE);
                        d = DUTY_NONE;
                    }
                }
                else
                {
                    ac_sync_state = WAIT_CROSS_POS_TO_NEG;
                    d = 0;
                    // last_d = 0;
                    ez1 = 0;
                    ez2 = 0;
                    HIGH_LEFT(DUTY_NONE);
                    LOW_RIGHT(DUTY_NONE);
                    LOW_LEFT(DUTY_ALWAYS);
                }
            }
            break;

        case WAIT_CROSS_POS_TO_NEG:
            //me quedo esperando en 0 un sample
            if (sequence_ready)
            {
                sequence_ready_reset;
                ac_sync_state = GEN_NEG;
                p_signal = mem_signal;
#ifdef USE_LED_FOR_MAIN_POLARITY
                LED_OFF;
#endif
            }
            break;
            
        case GEN_NEG:
            if (sequence_ready)
            {
                sequence_ready_reset;
                //aca la senial (el ultimo punto) termina en 0
                if (p_signal < &mem_signal[(SIZEOF_SIGNAL - 1)])
                {
                    p_signal++;

                    d = PID_roof(*p_signal, I_Sense_Neg, d, &ez1, &ez2);

                    if (d > 0)
                    {
                        if (d < DUTY_100_PERCENT)
                            HIGH_RIGHT(d);
                        else
                        {
                            HIGH_RIGHT(DUTY_100_PERCENT);
                            d = DUTY_100_PERCENT;
                        }
                    }
                    else
                    {
                        HIGH_RIGHT(DUTY_NONE);
                        d = DUTY_NONE;
                    }
                }
                else
                {
                    ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                    d = 0;
                    // last_d = 0;
                    ez1 = 0;
                    ez2 = 0;
                    HIGH_RIGHT(DUTY_NONE);
                    LOW_LEFT(DUTY_NONE);
                    LOW_RIGHT(DUTY_ALWAYS);
                }
            }
            break;

        case WAIT_CROSS_NEG_TO_POS:
            //me quedo esperando en 0 un sample
            if (sequence_ready)
            {
                sequence_ready_reset;
                ac_sync_state = GEN_POS;
                p_signal = mem_signal;
#ifdef USE_LED_FOR_MAIN_POLARITY
                LED_ON;
#endif
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

        default:
            ac_sync_state = START_SYNCING;
            break;
            
        }

        // SYNC_Update_Sync();
        // SYNC_Update_Polarity();

        // if (SYNC_Cycles_Cnt() > 100)
        // {
        //     SYNC_Cycles_Cnt_Reset();
        //     sprintf(s_lcd, "d_t1_bar: %d d_t2: %d pol: %d st: %d vline: %d\n",
        //             delta_t1_bar,
        //             delta_t2,
        //             SYNC_Last_Polarity_Check(),
        //             ac_sync_state,
        //             SYNC_Vline_Max());
        //     Usart1Send(s_lcd);            
        // }
        //Cosas que no tienen tanto que ver con las muestras o el estado del programa
        if ((STOP_JUMPER) &&
            (ac_sync_state != JUMPER_PROTECTED) &&
            (ac_sync_state != JUMPER_PROTECT_OFF) &&            
            (ac_sync_state != OVERCURRENT_ERROR))
        {
            RELAY_OFF;
            HIGH_LEFT(DUTY_NONE);
            HIGH_RIGHT(DUTY_NONE);

            LOW_RIGHT(DUTY_NONE);
            LOW_LEFT(DUTY_NONE);
            
            ChangeLed(LED_JUMPER_PROTECTED);
            Usart1Send((char *) "Protect ON\n");
            timer_standby = 1000;
            ac_sync_state = JUMPER_PROTECTED;
        }
        
        if (overcurrent_shutdown)
        {
            RELAY_OFF;
            if (overcurrent_shutdown == 1)
                ChangeLed(LED_OVERCURRENT_POS);
            else
                ChangeLed(LED_OVERCURRENT_NEG);

            timer_standby = 10000;
            overcurrent_shutdown = 0;
            ac_sync_state = OVERCURRENT_ERROR;
        }
        
#ifdef USE_LED_FOR_MAIN_STATES
        UpdateLed();
#endif        
    }
#endif    // INVERTER_MODE_CURRENT_FDBK

    
#ifdef INVERTER_ONLY_SYNC_AND_POLARITY

    while (1)
    {
        switch (ac_sync_state)
        {
        case START_SYNCING:
            //Check voltage and polarity
            // if ((Voltage_is_Good()) && Polarity_is_Good())
            LED_OFF;            
            ac_sync_state++;
            break;

        case WAIT_RELAY_TO_ON:
            ac_sync_state++;
            break;

        case WAIT_FOR_FIRST_SYNC:
            if (SYNC_Sync_Now())
            {
                if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
                    ac_sync_state = GEN_NEG;
#ifdef USE_LED_FOR_MAIN_POLARITY
                    LED_OFF;
#endif
                }
                else if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
                {
                    ac_sync_state = GEN_POS;
#ifdef USE_LED_FOR_MAIN_POLARITY                    
                    LED_ON;
#endif
                }
                else    //debe haber un error en synchro
                    ac_sync_state = START_SYNCING;
                
                SYNC_Sync_Now_Reset();
            }
            break;
        
        case GEN_POS:
            if (SYNC_Sync_Now())
            {
                ac_sync_state = WAIT_CROSS_POS_TO_NEG;
                SYNC_Sync_Now_Reset();
            }

            //TODO: poner timeout para salir aca o revisar POLARITY_UNKNOWN
            break;

        case WAIT_CROSS_POS_TO_NEG:
            if (SYNC_Last_Polarity_Check() == POLARITY_POS)
            {
                ac_sync_state = GEN_NEG;
#ifdef USE_LED_FOR_MAIN_POLARITY                
                LED_OFF;
#endif                
            }
            else    //debe haber un error de synchro
                ac_sync_state = START_SYNCING;

            break;
            
        case GEN_NEG:
            if (SYNC_Sync_Now())
            {
                ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                SYNC_Sync_Now_Reset();
            }
            
            //TODO: poner timeout para salir aca o revisar POLARITY_UNKNOWN            
            break;

        case WAIT_CROSS_NEG_TO_POS:
            if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
            {
                ac_sync_state = GEN_POS;
#ifdef USE_LED_FOR_MAIN_POLARITY                
                LED_ON;
#endif
            }
            else    //debe haber un error de synchro
                ac_sync_state = START_SYNCING;
                    
            break;

        default:
            ac_sync_state = START_SYNCING;
            break;
            
        }

        SYNC_Update_Sync();
        SYNC_Update_Polarity();

        if (SYNC_Cycles_Cnt() > 100)
        {
            SYNC_Cycles_Cnt_Reset();
            sprintf(s_lcd, "d_t1_bar: %d d_t2: %d pol: %d st: %d vline: %d\n",
                    delta_t1_bar,
                    delta_t2,
                    SYNC_Last_Polarity_Check(),
                    ac_sync_state,
                    SYNC_Vline_Max());
            Usart1Send(s_lcd);            
        }
    }
#endif     // INVERTER_ONLY_SYNC_AND_POLARITY
    
    
    // EnablePreload_MosfetA;
    // EnablePreload_MosfetB;

    //--- Inverter Mode ----------
#ifdef INVERTER_MODE
    
    while (1)
    {
        switch (ac_sync_state)
        {
        case START_SYNCING:
            RELAY_ON;
            timer_standby = 200;
            ac_sync_state = WAIT_RELAY_TO_ON;
            break;

        case WAIT_RELAY_TO_ON:
            if (!timer_standby)
            {
                SYNC_Sync_Now_Reset();
                TIM17->CNT = 0;
                TIM17->ARR = 9800;
                TIM17Enable();
                ac_sync_state = WAIT_FOR_FIRST_SYNC;
            }
            break;

        case WAIT_FOR_FIRST_SYNC:
            if (SYNC_Sync_Now())
            {
                SYNC_Sync_Now_Reset();
                ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                ChangeLed(LED_GENERATING);

                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);
                TIM16->CNT = 0;
            }
            break;
        
        case GEN_POS:
            if (SYNC_Sync_Now())
            {
                SYNC_Sync_Now_Reset();
                ac_sync_state = WAIT_CROSS_POS_TO_NEG;
                TIM16->CNT = 0;
                
                HIGH_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_NONE);

#ifdef USE_LED_FOR_MAIN_POLARITY
                LED_OFF;
#endif
            }
#ifdef INVERTER_MODE_PURE_SINUSOIDAL
            else
            {
                if (TIM16->CNT >= 200)
                {
                    TIM16->CNT = 0;
                    //aca la senial (el ultimo punto) terminaria en 0
                    if (p_signal < &mem_signal[(SIZEOF_SIGNAL - 1)])
                        p_signal++;

                    HIGH_LEFT(*p_signal);
                }
            }
#endif    // INVERTER_MODE_PURE_SINUSOIDAL
            break;

        case WAIT_CROSS_POS_TO_NEG:
            if (TIM16->CNT >= 200)
            {
                LOW_LEFT(DUTY_ALWAYS);
#ifndef INVERTER_MODE_PURE_SINUSOIDAL
                HIGH_RIGHT(DUTY_ALWAYS);
#else
                TIM16->CNT = 0;
                p_signal = mem_signal;
                HIGH_RIGHT(*p_signal);
#endif
                ac_sync_state = GEN_NEG;
            }
            break;
            
        case GEN_NEG:
            if (SYNC_Sync_Now())
            {
                SYNC_Sync_Now_Reset();
                ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                TIM16->CNT = 0;

                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);

#ifdef USE_LED_FOR_MAIN_POLARITY
                LED_ON;
#endif
            }
#ifdef INVERTER_MODE_PURE_SINUSOIDAL
            else
            {
                if (TIM16->CNT >= 200)
                {
                    TIM16->CNT = 0;
                    //aca la senial (el ultimo punto) terminaria en 0
                    if (p_signal < &mem_signal[(SIZEOF_SIGNAL - 1)])
                        p_signal++;

                    HIGH_RIGHT(*p_signal);
                }
            }
#endif    // INVERTER_MODE_PURE_SINUSOIDAL            
            break;

        case WAIT_CROSS_NEG_TO_POS:
            if (TIM16->CNT >= 200)
            {
                LOW_RIGHT(DUTY_ALWAYS);
#ifndef INVERTER_MODE_PURE_SINUSOIDAL
                HIGH_LEFT(DUTY_ALWAYS);
#else
                TIM16->CNT = 0;
                p_signal = mem_signal;
                HIGH_LEFT(*p_signal);
#endif
                ac_sync_state = GEN_POS;
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

        //Cosas que no tienen tanto que ver con las muestras o el estado del programa
        if ((STOP_JUMPER) &&
            (ac_sync_state != JUMPER_PROTECTED) &&
            (ac_sync_state != JUMPER_PROTECT_OFF) &&            
            (ac_sync_state != OVERCURRENT_ERROR))
        {
            RELAY_OFF;
            HIGH_LEFT(DUTY_NONE);
            HIGH_RIGHT(DUTY_NONE);

            LOW_RIGHT(DUTY_NONE);
            LOW_LEFT(DUTY_NONE);
            
            ChangeLed(LED_JUMPER_PROTECTED);
            Usart1Send((char *) "Protect ON\n");
            timer_standby = 1000;
            ac_sync_state = JUMPER_PROTECTED;
        }
        
        if (overcurrent_shutdown)
        {
            RELAY_OFF;
            if (overcurrent_shutdown == 1)
                ChangeLed(LED_OVERCURRENT_POS);
            else
                ChangeLed(LED_OVERCURRENT_NEG);

            timer_standby = 10000;
            overcurrent_shutdown = 0;
            ac_sync_state = OVERCURRENT_ERROR;
        }

#ifdef USE_LED_FOR_MAIN_STATES
        UpdateLed();
#endif
    }
    
#endif    // INVERTER_MODE
    
    return 0;
}

//--- End of Main ---//


void TimingDelay_Decrement(void)
{
    if (wait_ms_var)
        wait_ms_var--;

    if (timer_standby)
        timer_standby--;

    if (take_temp_sample)
        take_temp_sample--;

    if (timer_meas)
        timer_meas--;

    if (timer_led)
        timer_led--;

    if (timer_filters)
        timer_filters--;

#ifdef INVERTER_ONLY_SYNC_AND_POLARITY
    if (timer_no_sync)
        timer_no_sync--;
#endif
    
    // //cuenta de a 1 minuto
    // if (secs > 59999)	//pasaron 1 min
    // {
    // 	minutes++;
    // 	secs = 0;
    // }
    // else
    // 	secs++;
    //
    // if (minutes > 60)
    // {
    // 	hours++;
    // 	minutes = 0;
    // }


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

void EXTI4_15_IRQHandler(void)
{
#ifdef WITH_AC_SYNC_INT
    if (AC_SYNC_Int)
    {
        if (AC_SYNC_Int_Rising)
        {
            //reseteo tim
            delta_t2 = TIM16->CNT;
            TIM16->CNT = 0;
            AC_SYNC_Int_Rising_Reset;
            AC_SYNC_Int_Falling_Set;

            SYNC_Rising_Edge_Handler();
        }
        else if (AC_SYNC_Int_Falling)
        {
            delta_t1 = TIM16->CNT;
            AC_SYNC_Int_Falling_Reset;
            AC_SYNC_Int_Rising_Set;
            // ac_sync_int_flag = 1;
            
            SYNC_Falling_Edge_Handler();
        }
        AC_SYNC_Ack;
    }
#endif
    
#ifdef WITH_OVERCURRENT_SHUTDOWN
    if (OVERCURRENT_POS_Int)
    {
        HIGH_LEFT(DUTY_NONE);
        //TODO: trabar el TIM3 aca!!!
        overcurrent_shutdown = 1;
        OVERCURRENT_POS_Ack;
    }

    if (OVERCURRENT_NEG_Int)
    {
        HIGH_RIGHT(DUTY_NONE);
        //TODO: trabar el TIM3 aca!!!
        overcurrent_shutdown = 2;
        OVERCURRENT_NEG_Ack;
    }
#endif
}

//------ EOF -------//
