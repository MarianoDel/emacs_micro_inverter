//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### HARD.C ################################
//---------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "hard.h"
#include "tim.h"
#include "stm32f0xx.h"
#include "adc.h"
#include "dsp.h"

#include <stdio.h>
#include "uart.h"

/* Externals variables ---------------------------------------------------------*/
extern volatile unsigned short timer_led;
extern volatile unsigned short adc_ch[];




/* Global variables ------------------------------------------------------------*/
//para el led
led_state_t led_state = START_BLINKING;
unsigned char blink = 0;
unsigned char how_many_blinks = 0;


#define STRING2(x) #x
#define STRING(x) STRING2(x)
// #define STRING_CONCAT(str1,str2) #str1 " " #str2
// #pragma message "Corriente para calculos"
// #pragma message(STRING_CONCAT("Corriente para calculos",I_FOR_CALC))
// #pragma message(STRING_CONCAT("Corriente para calculos",STRING(I_FOR_CALC)))
#pragma message(STRING(I_FOR_CALC))

/* Module Functions ------------------------------------------------------------*/


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

//proteccion para no superar el valor Vin . Ton que puede saturar al trafo
//con 6T primario
unsigned short UpdateDMAX (unsigned short a)
{
    if (a > VIN_35V)
        a = 260;
    else if (a > VIN_30V)
        a = 297;
    else if (a > VIN_25V)
        a = 347;
    else if (a > VIN_20V)
        a = 417;
    else
        a = 450;
    
    return a;
}

unsigned short UpdateDMAXSF (unsigned short a)
{
    //por saturacion en arranque cambio max D
    if (a > VIN_35V)
        a = 50;
    else if (a > VIN_30V)
        a = 70;
    else if (a > VIN_25V)
        a = 90;
    else if (a > VIN_20V)
        a = 120;
    else
        a = 150;

    return a;
}

//Calcula en funcion de la tension aplicada a la bobina Lout
//el maxido d en ticks posible. Utiliza Imax (entrada o salida), Lout, tick_pwm
unsigned short UpdateDmaxLout (unsigned short delta_voltage)
{
    unsigned int num, den;

    if (delta_voltage > 0)
    {
        // num = I_FOR_CALC * LOUT_UHY * 1000;    //cambio para no tener decimales en el preprocesador
        num =  (ILOUT * 1000) * LOUT_UHY;
        // num = I_FOR_CALC_MILLIS * LOUT_UHY;    
        den = delta_voltage * TICK_PWM_NS;
        num = num / den;

        if (num > DMAX_HARDWARE)
            num = DMAX_HARDWARE;
    }
    else
        num = DMAX_HARDWARE;

    return (unsigned short) num;
}

//Convierte el valor de ticks ADC Vout a tension
unsigned short VoutTicksToVoltage (unsigned short sample_adc)
{
    unsigned int num;

    if (sample_adc > VOUT_300V)
    {
        num = sample_adc * 350;
        num = num / VOUT_350V;
    }
    else if (sample_adc > VOUT_200V)
    {
        num = sample_adc * 300;
        num = num / VOUT_300V;
    }
    else if (sample_adc > VOUT_110V)
    {
        num = sample_adc * 200;
        num = num / VOUT_200V;
    }
    else
    {
        num = sample_adc * 110;
        num = num / VOUT_110V;
    }
    
    return (unsigned short) num;
}

//Convierte el valor de ticks ADC Vin a tension
unsigned short VinTicksToVoltage (unsigned short sample_adc)
{
    unsigned int num;

    if (sample_adc > VIN_30V)
    {
        num = sample_adc * 35;
        num = num / VIN_35V;
    }
    else if (sample_adc > VIN_25V)
    {
        num = sample_adc * 30;
        num = num / VIN_30V;
    }
    else if (sample_adc > VIN_20V)
    {
        num = sample_adc * 25;
        num = num / VIN_25V;
    }    
    else
    {
        num = sample_adc * 20;
        num = num / VIN_20V;
    }
    
    return (unsigned short) num;
}


//Con la tension de entrada y salida calcula el maximo periodo permitido
unsigned short Hard_GetDmaxLout (unsigned short vin, unsigned short vout)
{
    unsigned int delta_vout = 0;
    unsigned short normalized_vout = 0;

    delta_vout = VinTicksToVoltage(vin);
    delta_vout = (delta_vout * N_TRAFO) / 1000;

    normalized_vout = VoutTicksToVoltage(vout);
    
    if (delta_vout > normalized_vout)
        delta_vout = delta_vout - normalized_vout;
    else
        delta_vout = 0;
    
    return UpdateDmaxLout((unsigned short)delta_vout);
}

void WelcomeCodeFeatures (char * str)
{
#ifdef TEST_INT_PRGRM
    Usart1Send("Programa de Testeo INT\n");
    Wait_ms(30);
#endif
#ifdef TEST_ADC_AND_DMA
    Usart1Send("Programa de Testeo ADC -> DMA\n");
    Wait_ms(30);    
#endif
#ifdef TEST_FIXED_D
    Usart1Send("Programa de ciclo d fijo\n");
    Wait_ms(30);    
#endif
#ifdef TEST_FIXED_VOUT
    Usart1Send("Programa Vout fijo\n");
    Wait_ms(30);    
#endif
#ifdef ONLY_COMMS
    Usart1Send("Only Communications for Ver 1.0\n");
    Wait_ms(30);    
#endif
#ifdef CURRENT_MODE_VER_2_0
    Usart1Send("Current Mode for Hwd ver 2.0\n");
    Wait_ms(30);    
#endif
    
#ifdef USE_FORWARD_MODE
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FORWARD_MODE));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef USE_PUSH_PULL_MODE
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_PUSH_PULL_MODE));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef WITH_OVERCURRENT_SHUTDOWN
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(WITH_OVERCURRENT_SHUTDOWN));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef WITH_TIM14_FB
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(WITH_TIM14_FB));
    Usart1Send(str);
    Wait_ms(30);    
#endif
#ifdef WITH_TIM1_FB
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(WITH_TIM1_FB));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef USE_FREQ_75KHZ
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FREQ_75KHZ));
    Usart1Send(str);
    Wait_ms(30);    
#endif

#ifdef USE_FREQ_48KHZ
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_FREQ_48KHZ));
    Usart1Send(str);
    Wait_ms(30);    
#endif
    
#ifdef USE_LED_IN_INT
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_LED_IN_INT));
    Usart1Send(str);
    Wait_ms(30);    
#endif
    
#ifdef USE_LED_IN_PROT
    sprintf(str,"[%s] %s\n", __FILE__, str_macro(USE_LED_IN_PROT));
    Usart1Send(str);
    Wait_ms(30);    
#endif    
}

//---- end of file ----//
