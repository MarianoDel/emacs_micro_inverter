//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### HARD.C ################################
//---------------------------------------------

// Includes --------------------------------------------------------------------
#include "hard.h"
#include "tim.h"
#include "stm32f0xx.h"
#include "pwm_defs.h"
#include "uart.h"

#include <stdio.h>


// Externals variables ---------------------------------------------------------
extern volatile unsigned short timer_led;
// extern volatile unsigned short adc_ch[];




// Globals ---------------------------------------------------------------------
// para el led
led_state_t led_state = START_BLINKING;
unsigned char blink = 0;
unsigned char how_many_blinks = 0;


// Module Functions ------------------------------------------------------------
//cambia configuracion de bips del LED
void ChangeLed (unsigned char how_many)
{
    how_many_blinks = how_many;
    led_state = START_BLINKING;
}

//mueve el LED segun el estado del Pote
void UpdateLed (void)
{
    switch (led_state)
    {
        case START_BLINKING:
            blink = how_many_blinks;
            
            if (blink)
            {
                LED_ON;
                timer_led = 200;
                led_state++;
                blink--;
            }
            break;

        case WAIT_TO_OFF:
            if (!timer_led)
            {
                LED_OFF;
                timer_led = 200;
                led_state++;
            }
            break;

        case WAIT_TO_ON:
            if (!timer_led)
            {
                if (blink)
                {
                    blink--;
                    timer_led = 200;
                    led_state = WAIT_TO_OFF;
                    LED_ON;
                }
                else
                {
                    led_state = WAIT_NEW_CYCLE;
                    timer_led = 2000;
                }
            }
            break;

        case WAIT_NEW_CYCLE:
            if (!timer_led)
                led_state = START_BLINKING;

            break;

        default:
            led_state = START_BLINKING;
            break;
    }
}


void WelcomeCodeFeatures (char * str)
{
    // Main Program Type
#ifdef GRID_TIED_FULL_CONECTED
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(GRID_TIED_FULL_CONECTED));
    Usart1Send(str);
    Wait_ms(30);    
#endif
    
    // Features mostly on hardware
#ifdef USE_FREQ_48KHZ
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FREQ_48KHZ));
    Usart1Send(str);
    Wait_ms(30);    
#endif
#ifdef USE_FREQ_24KHZ
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FREQ_24KHZ));
    Usart1Send(str);
    Wait_ms(30);    
#endif
#ifdef USE_FREQ_16KHZ
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FREQ_16KHZ));
    Usart1Send(str);
    Wait_ms(30);    
#endif
#ifdef USE_FREQ_12KHZ
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FREQ_12KHZ));
    Usart1Send(str);
    Wait_ms(30);    
#endif
#ifdef USE_FREQ_9_6KHZ
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FREQ_9_6KHZ));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef WITH_AC_SYNC_INT
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(WITH_AC_SYNC_INT));
    Usart1Send(str);
    Wait_ms(30);    
#endif
    
#ifdef WITH_OVERCURRENT_SHUTDOWN
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(WITH_OVERCURRENT_SHUTDOWN));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef WITH_SOFT_OVERCURRENT_SHUTDOWN
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(WITH_SOFT_OVERCURRENT_SHUTDOWN));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef WITH_FEW_CYCLES_OF_50HZ
    sprintf(str,"[%s] %s cycles: %d\n", __FILE__, str_macro(WITH_FEW_CYCLES_OF_50HZ), CYCLES_OF_50HZ);
    Usart1Send(str);
    Wait_ms(30);    
#endif
    
}

//---- end of file ----//
