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


    //--- GPIO A ---//
    if (!GPIOA_CLK)
        GPIOA_CLK_ON;

    temp = GPIOA->MODER;	//2 bits por pin
    temp &= 0x3CC000C0;		//PA0 PA1 PA2 analog input; PA4 PA5 input; PA6 PA7 alternate function;
    temp |= 0x4128A03F;		//PA8 input; PA9 PA10 alternative function; PA12 output; PA15 output
    GPIOA->MODER = temp;

    temp = GPIOA->OTYPER;	//1 bit por pin
    temp &= 0xFFFFFFFF;
    temp |= 0x00000000;
    GPIOA->OTYPER = temp;

    temp = GPIOA->OSPEEDR;	//2 bits por pin
    temp &= 0x3CFFFFFF;
    temp |= 0x00000000;		//low speed
    GPIOA->OSPEEDR = temp;

    temp = GPIOA->PUPDR;	//2 bits por pin
    temp &= 0xFFFCFFFF;         //PA8 pull up   
    temp |= 0x00010000;	
    GPIOA->PUPDR = temp;

    //Alternate Fuction for GPIOA
    // GPIOA->AFR[0] = 0x00001100;	//PA2 -> AF1; PA3 -> AF1;

    //--- GPIO B ---//
#ifdef GPIOB_ENABLE
    if (!GPIOB_CLK)
        GPIOB_CLK_ON;

    temp = GPIOB->MODER;	//2 bits por pin
    temp &= 0xFFFFCFF0;		//PB0 PB1 alternative function; PB6 input
    temp |= 0x0000000A;
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
    temp &= 0xFFFFCFFF;		//PB6 pull up
    temp |= 0x00001000;
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

#if (defined WITH_OVERCURRENT_SHUTDOWN) || (defined WITH_AC_SYNC_INT)
    //Interrupt en PA4 y PA5 or/and PA8
    if (!SYSCFG_CLK)
        SYSCFG_CLK_ON;

#ifdef WITH_OVERCURRENT_SHUTDOWN
    SYSCFG->EXTICR[1] = 0x00000000; //Select Port A & Pin4 Pin5  external interrupt
    // EXTI->IMR |= 0x00000030; 			//Corresponding mask bit for interrupts EXTI4 EXTI5
    EXTI->EMR |= 0x00000000; 			//Corresponding mask bit for events
    EXTI->RTSR |= 0x00000030; 			//pin4 pin5 Interrupt line on rising edge
    EXTI->FTSR |= 0x00000000; 			//Interrupt line on falling edge
#endif
#ifdef WITH_AC_SYNC_INT
    SYSCFG->EXTICR[2] = 0x00000000; //Select Port A & Pin8  external interrupt
    // EXTI->IMR |= 0x00000100; 			//Corresponding mask bit for interrupts EXTI4 EXTI5
    EXTI->EMR |= 0x00000000; 			//Corresponding mask bit for events
    EXTI->RTSR |= 0x00000100; 			//pin8 Interrupt line on rising edge
    EXTI->FTSR |= 0x00000000; 			//Interrupt line on falling edge
#endif

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
    
#endif    

}

#if (defined WITH_OVERCURRENT_SHUTDOWN) && (defined WITH_AC_SYNC_INT)
inline void EXTIOff (void)
{
    EXTI->IMR &= ~0x00000130;
}

inline void EXTIOn (void)
{
    EXTI->IMR |= 0x00000130;
}
#elif defined WITH_OVERCURRENT_SHUTDOWN
inline void EXTIOff (void)
{
    EXTI->IMR &= ~0x00000030;
}

inline void EXTIOn (void)
{
    EXTI->IMR |= 0x00000030;
}
#elif defined WITH_AC_SYNC_INT
inline void EXTIOff (void)
{
    EXTI->IMR &= ~0x00000100;
}

inline void EXTIOn (void)
{
    EXTI->IMR |= 0x00000100;
}
#endif



//--- end of file ---//
