//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F103
// ##
// #### TIM.H ################################
//---------------------------------------------
#ifndef _TIM_H_
#define _TIM_H_

//--- Incuded for help defines ---//
#include "hard.h"
#include "stm32f0xx.h"

//--- Exported types ---//
//--- Exported constants ---//
#define DUTY_NONE		0
#define DUTY_5_PERCENT		50
#define DUTY_10_PERCENT		100
#define DUTY_FOR_DMAX           450
#define DUTY_50_PERCENT		500
#define DUTY_75_PERCENT		750
#define DUTY_50_PERCENT_PLUS_ONE		501
#define DUTY_100_PERCENT        1000
#define DUTY_ALWAYS        (DUTY_100_PERCENT + 1)

#define DUTY_FB_25A    395    //esto es 1.17V que equivale a 25Apico en el primario

#define ENABLE_TIM1			TIM1->CR1 |= TIM_CR1_CEN;
#define DISABLE_TIM1			TIM1->CR1 &= ~TIM_CR1_CEN;

#define ENABLE_TIM3			TIM3->CR1 |= TIM_CR1_CEN;
#define DISABLE_TIM3			TIM3->CR1 &= ~TIM_CR1_CEN;



//--- Exported macro ---//
#define RCC_TIM1_CLK 		(RCC->APB2ENR & 0x00000800)
#define RCC_TIM1_CLK_ON 	RCC->APB2ENR |= 0x00000800
#define RCC_TIM1_CLK_OFF 	RCC->APB2ENR &= ~0x00000800

#define RCC_TIM3_CLK 		(RCC->APB1ENR & 0x00000002)
#define RCC_TIM3_CLK_ON 	RCC->APB1ENR |= 0x00000002
#define RCC_TIM3_CLK_OFF 	RCC->APB1ENR &= ~0x00000002

#define RCC_TIM6_CLK 		(RCC->APB1ENR & 0x00000010)
#define RCC_TIM6_CLK_ON 	RCC->APB1ENR |= 0x00000010
#define RCC_TIM6_CLK_OFF 	RCC->APB1ENR &= ~0x00000010

#define RCC_TIM14_CLK 		(RCC->APB1ENR & 0x00000100)
#define RCC_TIM14_CLK_ON 	RCC->APB1ENR |= 0x00000100
#define RCC_TIM14_CLK_OFF 	RCC->APB1ENR &= ~0x00000100

#define RCC_TIM15_CLK 		(RCC->APB2ENR & 0x00010000)
#define RCC_TIM15_CLK_ON 	RCC->APB2ENR |= 0x00010000
#define RCC_TIM15_CLK_OFF 	RCC->APB2ENR &= ~0x00010000

#define RCC_TIM16_CLK 		(RCC->APB2ENR & 0x00020000)
#define RCC_TIM16_CLK_ON 	RCC->APB2ENR |= 0x00020000
#define RCC_TIM16_CLK_OFF 	RCC->APB2ENR &= ~0x00020000

#define RCC_TIM17_CLK 		(RCC->APB2ENR & 0x00040000)
#define RCC_TIM17_CLK_ON 	RCC->APB2ENR |= 0x00040000
#define RCC_TIM17_CLK_OFF 	RCC->APB2ENR &= ~0x00040000


#define EnablePreload_Mosfet_HighLeft    (TIM3->CCMR1 |= TIM_CCMR1_OC2PE)
#define DisablePreload_Mosfet_HighLeft    (TIM3->CCMR1 &= ~TIM_CCMR1_OC2PE)
#define EnablePreload_Mosfet_HighRight    (TIM3->CCMR2 |= TIM_CCMR2_OC4PE)
#define DisablePreload_Mosfet_HighRight    (TIM3->CCMR2 &= ~TIM_CCMR2_OC4PE)

//--- Exported wrapped functions ---//
#ifdef WITH_TIM1_FB
#define UpdateFB(X)    Update_TIM1_CH3(X)
#endif
#ifdef WITH_TIM14_FB
#define UpdateFB(X)    Update_TIM14_CH1(X)
#endif

#define LOW_LEFT(X)     Update_TIM3_CH1(X)
#define HIGH_LEFT(X)    Update_TIM3_CH2(X)
#define LOW_RIGHT(X)     Update_TIM3_CH3(X)
#define HIGH_RIGHT(X)    Update_TIM3_CH4(X)

//--- Exported functions ---//


void TIM_1_Init(void);
void Update_TIM1_CH1 (unsigned short);
void Update_TIM1_CH3 (unsigned short);

void TIM_3_Init(void);
void TIM3_IRQHandler (void);
void Update_TIM3_CH1 (unsigned short);
void Update_TIM3_CH2 (unsigned short);
void Update_TIM3_CH3 (unsigned short);
void Update_TIM3_CH4 (unsigned short);

//Only for C8 Devices
void TIM_6_Init (void);
void TIM6Enable (void);
void TIM6Disable (void);

void TIM_14_Init(void);
void TIM14_IRQHandler (void);

void TIM_16_Init(void);
void TIM16_IRQHandler (void);
void OneShootTIM16 (unsigned short);
void TIM16Enable (void);
void TIM16Disable (void);

void TIM_17_Init(void);
void TIM17_IRQHandler (void);
void TIM17Enable (void);
void TIM17Disable (void);


void Wait_ms (unsigned short wait);
#endif
//--- End ---//


//--- END OF FILE ---//
