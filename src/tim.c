//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### TIM.C ################################
//---------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "tim.h"
#include "stm32f0xx.h"
#include "hard.h"


//--- VARIABLES EXTERNAS ---//
extern volatile unsigned char timer_1seg;
extern volatile unsigned short timer_led_comm;
extern volatile unsigned short wait_ms_var;
extern volatile unsigned short delta_t2;
extern volatile unsigned char ac_sync_int_flag;

//--- VARIABLES GLOBALES ---//

volatile unsigned short timer_1000 = 0;



//--- FUNCIONES DEL MODULO ---//
// inline void UpdateTIMSync (unsigned short a)
// {
//     //primero cargo TIM1
//     TIM1->CCR1 = a;
//     TIM3->ARR = DUTY_50_PERCENT + a;    //TIM3->CCR1 es el delay entre timers
//                                         //lo cargo en el timer init
// }

// inline void UpdateTIM_MosfetA (unsigned short a)
// {
//     TIM3->ARR = DUTY_50_PERCENT + a;    
// }

// inline void UpdateTIM_MosfetB (unsigned short a)
// {
//     TIM1->CCR1 = a;
// }

// inline void EnablePreload_MosfetA (void)
// {
//     // TIM3->CCMR1 |= TIM_CCMR1_OC1PE;
//     TIM3->CR1 |= TIM_CR1_ARPE;
// }

// inline void DisablePreload_MosfetA (void)
// {
//     // TIM3->CCMR1 &= ~TIM_CCMR1_OC1PE;
//     TIM3->CR1 &= ~TIM_CR1_ARPE;    
// }

// inline void EnablePreload_MosfetB (void)
// {
//     TIM1->CCMR1 |= TIM_CCMR1_OC1PE;
// }

// inline void DisablePreload_MosfetB (void)
// {
//     TIM1->CCMR1 &= ~TIM_CCMR1_OC1PE;
// }

void Update_TIM1_CH3 (unsigned short a)
{
    TIM1->CCR3 = a;
}

void Update_TIM3_CH1 (unsigned short a)
{
    TIM3->CCR1 = a;
}

void Update_TIM3_CH2 (unsigned short a)
{
    TIM3->CCR2 = a;
}

void Update_TIM3_CH3 (unsigned short a)
{
    TIM3->CCR3 = a;
}

void Update_TIM3_CH4 (unsigned short a)
{
    TIM3->CCR4 = a;
}

void Update_TIM14_CH1 (unsigned short a)
{
    TIM14->CCR1 = a;
}

void Wait_ms (unsigned short wait)
{
    wait_ms_var = wait;

    while (wait_ms_var);
}

//-------------------------------------------//
// @brief  TIM configure.
// @param  None
// @retval None
//------------------------------------------//
void TIM_1_Init (void)
{
    unsigned int temp = 0;

    if (!RCC_TIM1_CLK)
        RCC_TIM1_CLK_ON;

    //Configuracion del timer.
    //TIM1->CR1 |= TIM_CR1_OPM;        //clk int / 1; upcounting; one pulse
    TIM1->CR1 = 0x00;        //clk int / 1;
    TIM1->CR2 |= TIM_CR2_MMS_1;        //UEV -> TRG0
    //TIM1->CR2 = 0x00;
    //TIM1->SMCR |= TIM_SMCR_MSM | TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1 | TIM_SMCR_TS_1;    //link timer3
    TIM1->SMCR = 0x0000;

#if (defined VER_2_0)
#ifdef WITH_TIM1_FB
    TIM1->CCMR1 = 0x0060;            //CH1 output PWM mode 1 (channel active TIM1->CNT < TIM1->CCR1)
    TIM1->CCMR2 = 0x0060;            //CH3 output PWM mode 1
    TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC3NE;    //el pin es TIM1_CH3N
    // TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC3NE | TIM_CCER_CC3NP;    //el pin es TIM1_CH3N    
#else    
    TIM1->CCMR1 = 0x0060;            //CH1 output PWM mode 1 (channel active TIM1->CNT < TIM1->CCR1)    
    TIM1->CCMR2 = 0x0000;            //
    TIM1->CCER |= TIM_CCER_CC1E;
#endif
#endif
    
    TIM1->BDTR |= TIM_BDTR_MOE;
    // TIM1->ARR = 1023;                //cada tick 20.83ns
    TIM1->ARR = DUTY_100_PERCENT;                //cada tick 20.83ns

    TIM1->CNT = 0;
    TIM1->PSC = 0;

#if (defined VER_2_0)
#ifdef WITH_TIM1_FB
    temp = GPIOB->AFR[0];
    temp &= 0xFFFFFF0F;
    temp |= 0x00000020;    //PB1 -> AF2
    GPIOB->AFR[0] = temp;
#endif
    temp = GPIOA->AFR[1];
    temp &= 0xFFFFFFF0;
    temp |= 0x00000002;    //PA8 -> AF2
    GPIOA->AFR[1] = temp;
#endif
    
    // Enable timer ver UDIS
    //TIM1->DIER |= TIM_DIER_UIE;
    TIM1->CR1 |= TIM_CR1_CEN;

    TIM1->CCR1 = 0;
    TIM1->CCR3 = 0;    
}


void TIM_3_Init (void)
{
    unsigned int temp = 0;

    if (!RCC_TIM3_CLK)
        RCC_TIM3_CLK_ON;

    //Configuracion del timer.
    //TIM3->CR1 = 0x0000;        //clk int / 1; upcounting;
    //TIM3->CR2 |= TIM_CR2_MMS_1;        //UEV -> TRG0
    TIM3->CR2 = 0x0000;
    //TIM3->SMCR |= TIM_SMCR_SMS_2 |TIM_SMCR_SMS_1 | TIM_SMCR_TS_1 | TIM_SMCR_TS_0;    //reset mode
    //TIM3->SMCR |= TIM_SMCR_SMS_2;    //reset mode link timer1    OJO no anda
    // TIM3->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1;    //trigger mode link timer1
    TIM3->SMCR = 0x0000;    //
    TIM3->CCMR1 = 0x6060;            //CH1 CH2 output PWM mode 1 (channel active TIM3->CNT < TIM3->CCR1)
    TIM3->CCMR2 = 0x6060;            //CH3 CH4
    //  TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC1P;    //CH1 enable on pin active low
    TIM3->CCER |= TIM_CCER_CC4E |
        TIM_CCER_CC3E |
        TIM_CCER_CC2E |
        TIM_CCER_CC1E;    //CH1 CH2 CH3 CH4 enable on pin active high

    TIM3->ARR = DUTY_100_PERCENT;
    TIM3->CNT = 0;

#if defined USE_FREQ_48KHZ
    TIM3->PSC = 0;
#elif defined USE_FREQ_24KHZ
    TIM3->PSC = 1;
#else
#error "No FREQ selected for TIM3 on hard.h"
#endif
    
    //TIM3->EGR = TIM_EGR_UG;    //generate event

    //Alternative Function Pins
    temp = GPIOA->AFR[0];
    temp &= 0x00FFFFFF;
    temp |= 0x11000000;    //PA7 & PA6 -> AF1;
    GPIOA->AFR[0] = temp;

    temp = GPIOB->AFR[0];
    temp &= 0xFFFFFF00;
    temp |= 0x00000011;    //PB1 & PB0 -> AF1;
    GPIOB->AFR[0] = temp;

    // Enable timer ver UDIS
    //TIM3->DIER |= TIM_DIER_UIE;
    TIM3->CR1 |= TIM_CR1_CEN;

    TIM3->CCR1 = 0;        
}


void TIM3_IRQHandler (void)	
{
    //bajar flag
    if (TIM3->SR & 0x01)	//bajo el flag
        TIM3->SR = 0x00;
}


void TIM_6_Init (void)
{
    if (!RCC_TIM6_CLK)
        RCC_TIM6_CLK_ON;

    //Configuracion del timer.
    TIM6->CR1 = 0x00;		//clk int / 1; upcounting
    TIM6->PSC = 47;			//tick cada 1us
    TIM6->ARR = 0xFFFF;			//para que arranque
    //TIM6->CR1 |= TIM_CR1_CEN;
}

void TIM6Enable (void)
{
    TIM6->CR1 |= TIM_CR1_CEN;
}

void TIM6Disable (void)
{
    TIM6->CR1 &= ~TIM_CR1_CEN;
}

void TIM14_IRQHandler (void)	//100uS
{

    if (TIM14->SR & 0x01)
        //bajar flag
        TIM14->SR = 0x00;
}

void TIM_14_Init (void)
{
    unsigned int temp;

    if (!RCC_TIM14_CLK)
        RCC_TIM14_CLK_ON;

    TIM14->CCMR1 = 0x0060;            //CH1 output PWM mode 1
    TIM14->CCER |= TIM_CCER_CC1E;    //CH1 enable on pin active high
    //TIM3->CCER |= TIM_CCER_CC2E | TIM_CCER_CC2P;    //CH2 enable on pin active high
    TIM14->PSC = 3;			//tick cada 83.33n
    TIM14->ARR = 1023;    //freq 11.73KHz

    //Configuracion del timer.
    TIM14->EGR |= 0x0001;

    //Configuracion Pin PB1
    temp = GPIOB->AFR[0];
    temp &= 0xFFFFFF0F;
    temp |= 0x00000000;	//PB1 -> AF0
    GPIOB->AFR[0] = temp;

    TIM14->CR1 |= TIM_CR1_CEN;

}

void TIM16_IRQHandler (void)	//es one shoot
{
    //SendDMXPacket(PCKT_UPDATE);

    if (TIM16->SR & 0x01)
        //bajar flag
        TIM16->SR = 0x00;
}

void TIM_16_Init (void)
{
    if (!RCC_TIM16_CLK)
        RCC_TIM16_CLK_ON;

    //Configuracion del timer.
    TIM16->CR1 = 0x00;		//clk int / 1; upcounting; uev
    TIM16->ARR = 0xFFFF;
    TIM16->CNT = 0;
    TIM16->PSC = 47;			//tick 1us
    TIM16->EGR = TIM_EGR_UG;

}

void OneShootTIM16 (unsigned short a)
{
    TIM16->ARR = a;
    TIM16->CR1 |= TIM_CR1_CEN;
}

void TIM16Enable (void)
{
    TIM16->CR1 |= TIM_CR1_CEN;
}

void TIM16Disable (void)
{
    TIM16->CR1 &= ~TIM_CR1_CEN;
}

void TIM17_IRQHandler (void)	//200uS
{
    if (TIM17->SR & 0x01)
    {
        TIM17->ARR = delta_t2;
        ac_sync_int_flag = 1;
        
        // //si me llego la segunda int sin que haya visto AC_SYNC, freno
        // if (TIM17->ARR > 5000)
        //     TIM17Disable();
        // else
        //     TIM17->ARR = delta_t2;w
        
#ifdef USE_LED_FOR_SYNC
        if (LED)
            LED_OFF;
        else
            LED_ON;
#endif
        
        TIM17->SR = 0x00;		//bajar flag
    }    
}

void TIM_17_Init (void)
{
    if (!RCC_TIM17_CLK)
        RCC_TIM17_CLK_ON;

    //Configuracion del timer.
    TIM17->ARR = 0xFFFF;		//400us
    TIM17->CNT = 0;
    TIM17->PSC = 47;

    // Enable timer interrupt ver UDIS
    TIM17->DIER |= TIM_DIER_UIE;
    TIM17->CR1 |= TIM_CR1_URS;	//solo int cuando hay overflow y one shot

    NVIC_EnableIRQ(TIM17_IRQn);
    NVIC_SetPriority(TIM17_IRQn, 8);
}

void TIM17Enable (void)
{
    TIM17->CR1 |= TIM_CR1_CEN;
}

void TIM17Disable (void)
{
    TIM17->CR1 &= ~TIM_CR1_CEN;
}


//--- end of file ---//
