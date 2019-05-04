//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DMA.H #################################
//---------------------------------------------

#ifndef _DMA_H_
#define _DMA_H_

//--- Defines for configuration ----------------
// #define DMA_WITH_INTERRUPT

//--- Exported types ---//
#define RCC_DMA_CLK (RCC->AHBENR & RCC_AHBENR_DMAEN)
#define RCC_DMA_CLK_ON 		RCC->AHBENR |= RCC_AHBENR_DMAEN
#define RCC_DMA_CLK_OFF 	RCC->AHBENR &= ~RCC_AHBENR_DMAEN

#define sequence_ready         (DMA1->ISR & DMA_ISR_TCIF1)
#define sequence_ready_reset   (DMA1->IFCR = DMA_ISR_TCIF1)


//--- Exported constants ---//


//--- Module Functions ---//
void DMAConfig(void);
void DMAEnableInterrupt (void);
void DMADisableInterrupt (void);
void DMA1_Channel1_IRQHandler (void);




#endif /* _DMA_H_ */
