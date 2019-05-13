//-----------------------------------------------------
// #### PROYECTO MICROINVERSOR F030 - Custom Board ####
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



//--- VARIABLES EXTERNAS ---//


// ------- Externals del Puerto serie  -------
volatile unsigned char tx1buff[SIZEOF_DATA];
volatile unsigned char rx1buff[SIZEOF_DATA];
volatile unsigned char usart1_have_data = 0;

// ------- Externals del o para el ADC -------
volatile unsigned short adc_ch[ADC_CHANNEL_QUANTITY];
volatile unsigned char seq_ready = 0;


// ------- Externals para timers -------
volatile unsigned short timer_led = 0;
volatile unsigned char ac_sync_int_flag = 0;
volatile unsigned short delta_t2 = 0;

// ------- Externals para filtros -------
volatile unsigned short take_temp_sample = 0;

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

// unsigned short vbatt [SIZEOF_FILTER];
// unsigned short iboost [SIZEOF_FILTER];


// parameters_typedef param_struct;

//--- VARIABLES GLOBALES ---//
volatile unsigned char current_excess = 0;
volatile short d = 0;
short ez1 = 0;
short ez2 = 0;
// unsigned short dmax = 0;
unsigned short last_d = 0;
#define DELTA_D    2

volatile unsigned short delta_t1 = 0;

volatile unsigned short delta_t1_bar = 0;
volatile unsigned char overcurrent_shutdown = 0;
volatile unsigned char enable_internal_sync = 0;

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






//--- FUNCIONES DEL MODULO ---//
void TimingDelay_Decrement (void);


// ------- para el LM311 -------
extern void EXTI4_15_IRQHandler(void);



//--- Private Definitions ---//


//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
    unsigned char i;
    unsigned short ii;

    unsigned char undersampling = 0;
    main_state_t main_state = MAIN_INIT;
    unsigned short vin_filtered = 0;

    unsigned short dmax_lout = 0;
    unsigned short dmax_vin = 0;    

    char s_lcd [120];

#ifdef INVERTER_MODE
    ac_sync_state_t ac_sync_state = START_SYNCING;
#endif

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

    //--- Leo los parametros de memoria ---//

    // while (1)
    // {
    //  if (STOP_JUMPER)
    //  {
    //  	LED_OFF;
    //  }
    //  else
    //  {
    // 	  if (LED)
    // 	  	LED_OFF;
    // 	  else
    // 	  	LED_ON;
    //
    // 	  Wait_ms (250);
    //  }
    // }


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

    
    // TIM_1_Init ();	   //lo utilizo para mosfet Ctrol_M_B y para FB si esta definido en hard.h
    TIM_3_Init();	   //lo utilizo para mosfet Ctrol_M_A y para synchro ADC

    TIM_16_Init();
    TIM16Enable();
    TIM_17_Init();
    MA32Circular_Reset();
    
    LOW_LEFT(DUTY_NONE);
    HIGH_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);
    
    EXTIOn();
    
    // while (1)
    // {
    //     if (ac_sync_int_flag)
    //     {
    //         ac_sync_int_flag = 0;
    //         MA32Circular_Load(delta_t1);

    //         if (!timer_standby)
    //         {
    //             timer_standby = 1000;
    //             delta_t1_bar = MA32Circular_Calc();
    //             delta_t1_bar >>= 1;
    //             sprintf(s_lcd, "d_t1: %d d_t2: %d\n", delta_t1_bar, delta_t2);
    //             // sprintf(s_lcd, "d_t1: %d\n", delta_t1);
    //             Usart1Send(s_lcd);

    //             //evaluar y activar sync interno
    //             enable_internal_sync = 1;
                
                
    //         }
    //     }
    // }
        
    // EnablePreload_MosfetA;
    // EnablePreload_MosfetB;

    AdcConfig();

    //-- DMA configuration.
    DMAConfig();
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    ADC1->CR |= ADC_CR_ADSTART;

    //--- Inverter Mode ----------
#ifdef INVERTER_MODE
    
    while (1)
    {
        switch (ac_sync_state)
        {
        case START_SYNCING:
            RELAY_ON;
            delta_t2 = 9800;
            TIM17->CNT = delta_t2;
            TIM17Enable();
            ac_sync_int_flag = 0;
            ac_sync_state = WAIT_FOR_FIRST_SYNC;
            break;

        case WAIT_FOR_FIRST_SYNC:
            if (ac_sync_int_flag)
            {
                ac_sync_int_flag = 0;
                ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                ChangeLed(LED_GENERATING);

                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);
                TIM16->CNT = 0;
            }
            break;
        
        case GEN_POS:
            if (ac_sync_int_flag)
            {
                ac_sync_int_flag = 0;
                ac_sync_state = WAIT_CROSS_POS_TO_NEG;
                TIM16->CNT = 0;
                
                HIGH_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_NONE);

#ifdef USE_LED_FOR_SYNC_IN_MAIN
                LED_OFF;
#endif
            }
            break;

        case WAIT_CROSS_POS_TO_NEG:
            if (TIM16->CNT >= 200)
            {
                LOW_LEFT(DUTY_ALWAYS);
                HIGH_RIGHT(DUTY_ALWAYS);
                ac_sync_state = GEN_NEG;
            }
            break;
            
        case GEN_NEG:
            if (ac_sync_int_flag)
            {
                ac_sync_int_flag = 0;
                ac_sync_state = WAIT_CROSS_NEG_TO_POS;
                TIM16->CNT = 0;

                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);

#ifdef USE_LED_FOR_SYNC_IN_MAIN
                LED_ON;
#endif
            }
            break;

        case WAIT_CROSS_NEG_TO_POS:
            if (TIM16->CNT >= 200)
            {
                LOW_RIGHT(DUTY_ALWAYS);
                HIGH_LEFT(DUTY_ALWAYS);
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
    
#endif
    //--- End Inverter Mode ----------

                



    
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
    if (AC_SYNC_Int)
    {
        if (AC_SYNC_Int_Rising)
        {
            //reseteo tim
            delta_t2 = TIM16->CNT;
            TIM16->CNT = 0;
            AC_SYNC_Int_Rising_Reset;
            AC_SYNC_Int_Falling_Set;
            // LED_ON;
            if (enable_internal_sync)
            {
                TIM17->CNT = 0;
                TIM17->ARR = delta_t1_bar;
                TIM17Enable();
            }
        }
        else if (AC_SYNC_Int_Falling)
        {
            delta_t1 = TIM16->CNT;
            AC_SYNC_Int_Falling_Reset;
            AC_SYNC_Int_Rising_Set;
            // LED_OFF;
            ac_sync_int_flag = 1;
        }
        AC_SYNC_Ack;
    }

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
