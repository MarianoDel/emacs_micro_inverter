//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### GPIO.C ################################
//---------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "gpio.h"
#include "hard.h"



//--- Private typedef ---//
//--- Private define ---//
//--- Private macro ---//
//--- Private variables ---//
//--- Private function prototypes ---//
//--- Private functions ---//

//-------------------------------------------//
// @brief  GPIO configure.
// @param  None
// @retval None
//------------------------------------------//
void GPIO_Config (void)
{
    unsigned long temp;

    //--- MODER ---//
    //00: Input mode (reset state)
    //01: General purpose output mode
    //10: Alternate function mode
    //11: Analog mode

    //--- OTYPER ---//
    //These bits are written by software to configure the I/O output type.
    //0: Output push-pull (reset state)
    //1: Output open-drain

    //--- ORSPEEDR ---//
    //These bits are written by software to configure the I/O output speed.
    //x0: Low speed.
    //01: Medium speed.
    //11: High speed.
    //Note: Refer to the device datasheet for the frequency.

    //--- PUPDR ---//
    //These bits are written by software to configure the I/O pull-up or pull-down
    //00: No pull-up, pull-down
    //01: Pull-up
    //10: Pull-down
    //11: Reserved

#if (defined VER_2_0)
    //--- GPIO A ---//
    if (!GPIOA_CLK)
        GPIOA_CLK_ON;

    temp = GPIOA->MODER;	//2 bits por pin
    temp &= 0x3F000CC0;		//PA0 PA1 PA2 analog input; PA4 input; PA6 alternate function; PA7 Input
    temp |= 0x402A203F;		//PA8 PA9 PA10 alternative function; PA11 Input; PA15 output
    GPIOA->MODER = temp;

    temp = GPIOA->OTYPER;	//1 bit por pin
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOA->OTYPER = temp;

    temp = GPIOA->OSPEEDR;	//2 bits por pin
    temp &= 0x3FC03FFF;
    temp |= 0x00000000;		//low speed
    GPIOA->OSPEEDR = temp;

    temp = GPIOA->PUPDR;	//2 bits por pin
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;		//pull down pin1 para pruebas
    GPIOA->PUPDR = temp;

    //Alternate Fuction for GPIOA
    // GPIOA->AFR[0] = 0x00001100;	//PA2 -> AF1; PA3 -> AF1;

    //--- GPIO B ---//
#ifdef GPIOB_ENABLE
    if (!GPIOB_CLK)
        GPIOB_CLK_ON;

    temp = GPIOB->MODER;	//2 bits por pin
    temp &= 0xFFFFCFF0;		//PB0 input; PB1 alternative; PB6 input
    temp |= 0x00000008;
    GPIOB->MODER = temp;

    temp = GPIOB->OTYPER;	//1 bit por pin
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOB->OTYPER = temp;

    temp = GPIOB->OSPEEDR;	//2 bits por pin
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;		//low speed
    GPIOB->OSPEEDR = temp;

    temp = GPIOB->PUPDR;	//2 bits por pin
    temp &= 0xFFFFCFFC;		//PB0 PB6 pull up
    temp |= 0x00001001;
    GPIOB->PUPDR = temp;

    //Alternate Fuction for GPIOB
    //GPIOB->AFR[0] = 0x00010000;	//PB4 -> AF1 enable pin on tim.c
#endif

#ifdef GPIOF_ENABLE

    //--- GPIO F ---//
    if (!GPIOF_CLK)
        GPIOF_CLK_ON;

    temp = GPIOF->MODER;
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOF->MODER = temp;

    temp = GPIOF->OTYPER;
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOF->OTYPER = temp;

    temp = GPIOF->OSPEEDR;
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOF->OSPEEDR = temp;

    temp = GPIOF->PUPDR;
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOF->PUPDR = temp;

#endif

#ifdef WITH_OVERCURRENT_SHUTDOWN
    //Interrupt en PA4 y PA5
    if (!SYSCFG_CLK)
        SYSCFG_CLK_ON;

    SYSCFG->EXTICR[1] = 0x00000000; //Select Port A & Pin4 Pin5  external interrupt
    // EXTI->IMR |= 0x00000030; 			//Corresponding mask bit for interrupts EXTI4 EXTI5
    EXTI->EMR |= 0x00000000; 			//Corresponding mask bit for events
    EXTI->RTSR |= 0x00000030; 			//Interrupt line on rising edge
    EXTI->FTSR |= 0x00000000; 			//Interrupt line on falling edge

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
    
#else
    //Interrupt en PA4
    if (!SYSCFG_CLK)
        SYSCFG_CLK_ON;

    SYSCFG->EXTICR[1] = 0x00000000; //Select Port A & Pin4 external interrupt
    // EXTI->IMR |= 0x00000010; 			//Corresponding mask bit for interrupts EXTI4
    EXTI->EMR |= 0x00000000; 			//Corresponding mask bit for events
    EXTI->RTSR |= 0x00000000; 			//Interrupt line on rising edge
    EXTI->FTSR |= 0x00000010; 			//Interrupt line on falling edge

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 2);    
#endif    

#endif    //end of ver_2_0
}


#if (defined VER_2_0)
#ifdef WITH_OVERCURRENT_SHUTDOWN
inline void EXTIOff (void)
{
    EXTI->IMR &= ~0x00000030;
}

inline void EXTIOn (void)
{
    EXTI->IMR |= 0x00000030;
}
#else
inline void EXTIOff (void)
{
    EXTI->IMR &= ~0x00000010;
}

inline void EXTIOn (void)
{
    EXTI->IMR |= 0x00000010;
}
#endif
#endif    //end of ver 2.0

//--- end of file ---//
