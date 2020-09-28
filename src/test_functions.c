//------------------------------------------------
// ## Internal Test Functions Module
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TEST_FUNCTIONS.C #########################
//------------------------------------------------

// Includes --------------------------------------------------------------------
#include "test_functions.h"
#include "stm32f0xx.h"
#include "hard.h"
#include "tim.h"
#include "sync.h"
#include "uart.h"
// #include "adc.h"
// #include "dma.h"
// #include "tim.h"
// #include "flash_program.h"

#include <stdio.h>
// #include <string.h>



// Externals -------------------------------------------------------------------
extern volatile unsigned short timer_led;

// Globals ---------------------------------------------------------------------


// Module Private Types & Macros -----------------------------------------------


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
// Conecta el relay y ningun mosfet para medir continuidad electrica
// entre AC_POS - AC_NEG y la salida de 220Vac
void TF_RelayConnect (void)
{
    HIGH_LEFT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);
    LOW_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);

    Wait_ms(10);

    unsigned char stopped = 0;
    while (1)
    {
        if (!STOP_JUMPER)
        {
            if (stopped)
            {
                stopped = 0;
                LED_ON;
                RELAY_ON;
                Wait_ms(500);    //evito glitches
            }
        }
        else
        {
            if (!stopped)
            {
                stopped = 1;
                LED_OFF;
                RELAY_OFF;
                Wait_ms(500);    //evito glitches
            }
        }
    }
}


//activo lado ACPOS 50% con el jumper, el relay va siempre conectado
void TF_RelayACPOS (void)
{
    HIGH_LEFT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);
    LOW_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    LED_OFF;
    
    Wait_ms(10);

    RELAY_ON;
    LOW_RIGHT(DUTY_ALWAYS);
    Wait_ms(100);

    unsigned char stopped = 0;
    while(1)
    {
        if (!STOP_JUMPER)
        {
            if (stopped)
            {
                stopped = 0;
                LED_ON;
                HIGH_LEFT(DUTY_50_PERCENT);
                Wait_ms(500);    //evito glitches
            }
        }
        else
        {
            if (!stopped)
            {
                stopped = 1;
                LED_OFF;
                HIGH_LEFT(DUTY_NONE);
                Wait_ms(500);    //evito glitches
            }                
        }
    }
}


void TF_RelayACNEG (void)
{
    HIGH_LEFT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);
    LOW_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    LED_OFF;
    
    Wait_ms(10);

    RELAY_ON;
    LOW_LEFT(DUTY_ALWAYS);
    Wait_ms(100);

    unsigned char stopped = 0;
    while(1)
    {
        if (!STOP_JUMPER)
        {
            if (stopped)
            {
                stopped = 0;
                LED_ON;
                HIGH_RIGHT(DUTY_50_PERCENT);
                Wait_ms(500);    //evito glitches
            }
        }
        else
        {
            if (!stopped)
            {
                stopped = 1;
                LED_OFF;
                HIGH_RIGHT(DUTY_NONE);
                Wait_ms(500);    //evito glitches
            }                
        }
    }
}


// genero 50Hz con 50% de duty a cada lado
void TF_RelayFiftyHz (void)
{
    HIGH_LEFT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);
    LOW_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    LED_OFF;
    
    Wait_ms(10);
    DisablePreload_Mosfet_HighLeft;
    DisablePreload_Mosfet_HighRight;

    unsigned char stopped = 0;
    unsigned char state = 0;
    while(1)
    {
        if (!STOP_JUMPER)
        {
            if (stopped)
            {
                stopped = 0;
                RELAY_ON;
                state = 0;

                Wait_ms(500);    //evito glitches
            }
        }
        else
        {
            if (!stopped)
            {
                stopped = 1;
                
                HIGH_LEFT(DUTY_NONE);
                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_NONE);

                RELAY_OFF;
                LED_OFF;
                state = 2;
                Wait_ms(500);    //evito glitches
            }                
        }

        switch (state)
        {
        case 0:
            if (!timer_led)
            {
                timer_led = 10;
                //corto ciclo anterior
                HIGH_RIGHT(DUTY_NONE);
                LOW_LEFT(DUTY_NONE);

                //nuevo ciclo
                LOW_RIGHT(DUTY_ALWAYS);
                HIGH_LEFT(DUTY_50_PERCENT);
                LED_ON;
                state = 1;
            }
            break;

        case 1:
            if (!timer_led)
            {
                timer_led = 10;
                //corto ciclo anterior
                HIGH_LEFT(DUTY_NONE);
                LOW_RIGHT(DUTY_NONE);

                //nuevo ciclo
                LOW_LEFT(DUTY_ALWAYS);
                HIGH_RIGHT(DUTY_50_PERCENT);
                LED_OFF;
                state = 0;
            }
            break;

        case 2:
            break;

        default:
            state = 0;
            break;
        }
    }
}


// #define TF_LED_FOR_SYNC
#define TF_LED_FOR_POLARITY
// #define TF_LED_FOR_DEBUG_UPDATE
#define timer_standby    timer_led
void TF_OnlySyncAndPolarity (void)
{
    char s_send [120] = { 0 };
    
    HIGH_LEFT(DUTY_NONE);
    HIGH_RIGHT(DUTY_NONE);
    LOW_LEFT(DUTY_NONE);
    LOW_RIGHT(DUTY_NONE);
    LED_OFF;
    
    Wait_ms(10);
    RELAY_OFF;
    Wait_ms(100);

    ac_sync_state_t ac_sync_state = START_SYNCING;
    unsigned char cycles_before_start = CYCLES_BEFORE_START;
    while (1)
    {
        switch (ac_sync_state)
        {
        case START_SYNCING:
            SYNC_Restart();
            cycles_before_start = CYCLES_BEFORE_START;
            ac_sync_state = SWITCH_RELAY_TO_ON;

            break;

        case SWITCH_RELAY_TO_ON:
            //Check voltage and polarity
            if (SYNC_All_Good())
            {
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
                }
                else if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
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
            if (SYNC_Sync_Now())
            {
                if (SYNC_Last_Polarity_Check() == POLARITY_POS)
                {
                    ac_sync_state = GEN_NEG;
#ifdef TF_LED_FOR_POLARITY
                    LED_OFF;
#endif
                }
                else if (SYNC_Last_Polarity_Check() == POLARITY_NEG)
                {
                    ac_sync_state = GEN_POS;
#ifdef TF_LED_FOR_POLARITY
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
#ifdef TF_LED_FOR_POLARITY
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
#ifdef TF_LED_FOR_POLARITY
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
#ifdef TF_LED_FOR_DEBUG_UPDATE
            LED_ON;
#endif
            SYNC_Cycles_Cnt_Reset();
            sprintf(s_send, "d_t1_bar: %d d_t2: %d pol: %d st: %d vline: %d\n",
                    SYNC_delta_t1_half_bar(),
                    SYNC_delta_t2_bar(),
                    SYNC_Last_Polarity_Check(),
                    ac_sync_state,
                    SYNC_Vline_Max());
            Usart1Send(s_send);
            
#ifdef TF_LED_FOR_DEBUG_UPDATE
            LED_OFF;
#endif
        }
    }
}


//--- end of file ---//
