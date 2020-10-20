//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### TIM.C ################################
//---------------------------------------------

// Includes --------------------------------------------------------------------
#include "tim.h"
#include "pwm_defs.h"
#include "stm32f0xx.h"
#include "hard.h"
#include "sync.h"

// Externals -------------------------------------------------------------------
extern volatile unsigned short wait_ms_var;


// Globals ---------------------------------------------------------------------


// Module Private Functions ----------------------------------------------------

// Module Functions ------------------------------------------------------------
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
    if (!RCC_TIM1_CLK)
        RCC_TIM1_CLK_ON;

    //Configuracion del timer.
    //TIM1->CR1 |= TIM_CR1_OPM;        //clk int / 1; upcounting; one pulse
    TIM1->CR1 = 0x00;        //clk int / 1;
    // TIM1->CR2 |= TIM_CR2_MMS_1;        //UEV -> TRG0
    TIM1->CR2 = 0x00;
    //TIM1->SMCR |= TIM_SMCR_MSM | TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1 | TIM_SMCR_TS_1;    //link timer3
    TIM1->SMCR = 0x0000;

    TIM1->CCMR1 = 0x0060;            //CH1 output PWM mode 1 (channel active TIM1->CNT < TIM1->CCR1)    
    TIM1->CCMR2 = 0x0000;            //
    TIM1->CCER |= TIM_CCER_CC1E;
    
    TIM1->BDTR |= TIM_BDTR_MOE;
    // TIM1->ARR = 1023;                //cada tick 20.83ns
    TIM1->ARR = DUTY_100_PERCENT;                //cada tick 20.83ns

    TIM1->CNT = 0;
    TIM1->PSC = 0;

    // unsigned int temp = 0;    
    // temp = GPIOA->AFR[1];
    // temp &= 0xFFFFFFF0;
    // temp |= 0x00000002;    //PA8 -> AF2
    // GPIOA->AFR[1] = temp;
    
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
    TIM3->CR2 |= TIM_CR2_MMS_1;        //UEV -> TRG0 (for the ADC sync)
    // TIM3->CR2 = 0x0000;
    //TIM3->SMCR |= TIM_SMCR_SMS_2 |TIM_SMCR_SMS_1 | TIM_SMCR_TS_1 | TIM_SMCR_TS_0;    //reset mode
    //TIM3->SMCR |= TIM_SMCR_SMS_2;    //reset mode link timer1    OJO no anda
    // TIM3->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1;    //trigger mode link timer1
    TIM3->SMCR = 0x0000;    //
    TIM3->CCMR1 = 0x6060;            //CH1 CH2 output PWM mode 1 (channel active TIM3->CNT < TIM3->CCR1)
    TIM3->CCMR2 = 0x6060;            //CH3 CH4
    //  TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC1P;    //CH1 enable on pin active low
    TIM3->CCER |=
        TIM_CCER_CC4E |
        TIM_CCER_CC3E |
        TIM_CCER_CC2E |
        TIM_CCER_CC1E;    //CH1 CH2 CH3 CH4 enable on pin active high

    TIM3->ARR = DUTY_100_PERCENT;
    TIM3->CNT = 0;

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


void TIM_17_Init (void)
{
    if (!RCC_TIM17_CLK)
        RCC_TIM17_CLK_ON;

    //Configuracion del timer.
    TIM17->ARR = 0xFFFF;
    TIM17->CNT = 0;
    TIM17->PSC = 47;

    // Enable timer interrupt ver UDIS
    TIM17->DIER |= TIM_DIER_UIE;
    TIM17->CR1 |= TIM_CR1_URS;	//solo int cuando hay overflow y one shot

    NVIC_EnableIRQ(TIM17_IRQn);
    NVIC_SetPriority(TIM17_IRQn, 8);
}


void TIM17_IRQHandler (void)
{
    if (TIM17->SR & 0x01)
    {
        SYNC_Zero_Crossing_Handler();
        TIM17->SR = 0x00;    //flag down
    }    
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
