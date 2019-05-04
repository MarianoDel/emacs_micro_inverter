//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### UART.H ################################
//---------------------------------------------
#ifndef __UART_H
#define __UART_H

//--- Exported types ---//
//--- Exported constants ---//
#define USART_9600		5000
#define USART_115200		416
#define USART_250000		192

//--- Modos de funcionamiento del USART1
#define USART_GPS_MODE		1
#define USART_GSM_MODE		2
#define USART_ASK_MODE		0

//--- Exported macro ---//
#define USART1_CLK (RCC->APB2ENR & 0x00004000)
#define USART1_CLK_ON RCC->APB2ENR |= 0x00004000
#define USART1_CLK_OFF RCC->APB2ENR &= ~0x00004000

#define USART2_CLK (RCC->APB1ENR & 0x00020000)
#define USART2_CLK_ON RCC->APB1ENR |= 0x00020000
#define USART2_CLK_OFF RCC->APB1ENR &= ~0x00020000

#define USARTx_RX_DISA	USARTx->CR1 &= 0xfffffffb
#define USARTx_RX_ENA	USARTx->CR1 |= 0x04



//--- Exported functions ---//
unsigned char Usart1Mode (unsigned char );
unsigned char ReadUsart1Buffer (unsigned char *, unsigned short);
unsigned char ReadUsart2Buffer (unsigned char *, unsigned short);

void USART1Config(void);
void USART1_IRQHandler(void);
void Usart1Send (char *);
void Usart1SendUnsigned(unsigned char *, unsigned char);
void Usart1SendSingle(unsigned char );

void USART2Config(void);
void USART2_IRQHandler(void);
void Usart2Send (char *);
void Usart2SendUnsigned(unsigned char *, unsigned char);
void Usart2SendSingle(unsigned char );


#endif
//--- END OF FILE ---//
