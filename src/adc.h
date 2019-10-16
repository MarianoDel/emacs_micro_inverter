//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### ADC.H #################################
//---------------------------------------------
#ifndef _ADC_H_
#define _ADC_H_

#include "hard.h"		//por configuracion diferentes en V1_0 y V1_1

//----------- Defines For Configuration --------------//
//----------- Some ADC Configurations ----------------//
// #define ADC_WITH_INT
#define ADC_WITH_DMA

#ifdef WITH_TEMP_CONTROL
#define ADC_WITH_TEMP_SENSE
#endif

#ifdef ADC_WITH_TEMP_SENSE
#define SIZEOF_BOARD_TEMP		8
#endif
//----------- End of ADC Configurations --------------//

#define V_Sense        adc_ch[0]
#define I_Sense_Pos    adc_ch[1]
#define I_Sense_Neg    adc_ch[2]
#define Vline_Sense    V_Sense
#define ADC_CHANNEL_QUANTITY         3
#define ADC_LAST_CHANNEL_QUANTITY    (ADC_CHANNEL_QUANTITY - 1)





#define RCC_ADC_CLK 		(RCC->APB2ENR & 0x00000200)
#define RCC_ADC_CLK_ON 		RCC->APB2ENR |= 0x00000200
#define RCC_ADC_CLK_OFF 	RCC->APB2ENR &= ~0x00000200

/* Temperature sensor calibration value address */
#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
#define VDD_CALIB ((uint16_t) (330))
#define VDD_APPLI ((uint16_t) (330))


#define ADC_IT_ADRDY                               ADC_IER_ADRDYIE
#define ADC_IT_EOSMP                               ADC_IER_EOSMPIE
#define ADC_IT_EOC                                 ADC_IER_EOCIE
#define ADC_IT_EOSEQ                               ADC_IER_EOSEQIE
#define ADC_IT_OVR                                 ADC_IER_OVRIE
#define ADC_IT_AWD                                 ADC_IER_AWDIE

#define ADC_CH0		0x00000001
#define ADC_CH1		0x00000002
#define ADC_CH2		0x00000004

#define ADC_CH16		0x00010000

#define ADC_Channel_0                              ADC_CHSELR_CHSEL0
#define ADC_Channel_1                              ADC_CHSELR_CHSEL1
#define ADC_Channel_2                              ADC_CHSELR_CHSEL2
#define ADC_Channel_3                              ADC_CHSELR_CHSEL3
#define ADC_Channel_4                              ADC_CHSELR_CHSEL4
#define ADC_Channel_5                              ADC_CHSELR_CHSEL5
#define ADC_Channel_6                              ADC_CHSELR_CHSEL6
#define ADC_Channel_7                              ADC_CHSELR_CHSEL7
#define ADC_Channel_8                              ADC_CHSELR_CHSEL8
#define ADC_Channel_9                              ADC_CHSELR_CHSEL9
#define ADC_Channel_10                             ADC_CHSELR_CHSEL10 /*!< Not available for STM32F031 devices */
#define ADC_Channel_11                             ADC_CHSELR_CHSEL11 /*!< Not available for STM32F031 devices */
#define ADC_Channel_12                             ADC_CHSELR_CHSEL12 /*!< Not available for STM32F031 devices */
#define ADC_Channel_13                             ADC_CHSELR_CHSEL13 /*!< Not available for STM32F031 devices */
#define ADC_Channel_14                             ADC_CHSELR_CHSEL14 /*!< Not available for STM32F031 devices */
#define ADC_Channel_15                             ADC_CHSELR_CHSEL15 /*!< Not available for STM32F031 devices */
#define ADC_Channel_16                             ADC_CHSELR_CHSEL16
#define ADC_Channel_17                             ADC_CHSELR_CHSEL17
#define ADC_Channel_18                             ADC_CHSELR_CHSEL18 /*!< Not available for STM32F030 devices */

#define ADC_Channel_TempSensor                     ((uint32_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                        ((uint32_t)ADC_Channel_17)
#define ADC_Channel_Vbat                           ((uint32_t)ADC_Channel_18) /*!< Not available for STM32F030 devices */

#define ADC_SampleTime_1_5Cycles                     ((uint32_t)0x00000000)
#define ADC_SampleTime_7_5Cycles                     ((uint32_t)0x00000001)
#define ADC_SampleTime_13_5Cycles                    ((uint32_t)0x00000002)
#define ADC_SampleTime_28_5Cycles                    ((uint32_t)0x00000003)
#define ADC_SampleTime_41_5Cycles                    ((uint32_t)0x00000004)
#define ADC_SampleTime_55_5Cycles                    ((uint32_t)0x00000005)
#define ADC_SampleTime_71_5Cycles                    ((uint32_t)0x00000006)
#define ADC_SampleTime_239_5Cycles                   ((uint32_t)0x00000007)

#define ADC_Resolution_12b                         ((uint32_t)0x00000000)
#define ADC_Resolution_10b                         ADC_CFGR1_RES_0
#define ADC_Resolution_8b                          ADC_CFGR1_RES_1
#define ADC_Resolution_6b                          ADC_CFGR1_RES

#define ADC_ClockMode_AsynClk                  ((uint32_t)0x00000000)   /*!< ADC Asynchronous clock mode */
#define ADC_ClockMode_SynClkDiv2               ADC_CFGR2_CKMODE_0   /*!<  Synchronous clock mode divided by 2 */
#define ADC_ClockMode_SynClkDiv4               ADC_CFGR2_CKMODE_1   /*!<  Synchronous clock mode divided by 4 */

#define ADC_ExternalTrigConvEdge_None              ((uint32_t)0x00000000)
#define ADC_ExternalTrigConvEdge_Rising            ADC_CFGR1_EXTEN_0
#define ADC_ExternalTrigConvEdge_Falling           ADC_CFGR1_EXTEN_1
#define ADC_ExternalTrigConvEdge_RisingFalling     ADC_CFGR1_EXTEN

#define ADC_ExternalTrigConv_T1_TRGO               ((uint32_t)0x00000000)
#define ADC_ExternalTrigConv_T1_CC4                ADC_CFGR1_EXTSEL_0

/* TIM2 */
#define ADC_ExternalTrigConv_T2_TRGO               ADC_CFGR1_EXTSEL_1

/* TIM3 */
#define ADC_ExternalTrigConv_T3_TRGO               ((uint32_t)(ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_EXTSEL_1))

/* TIM15 */
#define ADC_ExternalTrigConv_T15_TRGO              ADC_CFGR1_EXTSEL_2

#define ADC_DataAlign_Right                        ((uint32_t)0x00000000)
#define ADC_DataAlign_Left                         ADC_CFGR1_ALIGN

#define ADC_ScanDirection_Upward                   ((uint32_t)0x00000000)
#define ADC_ScanDirection_Backward                 ADC_CFGR1_SCANDIR

#define ADC_DMAMode_OneShot                        ((uint32_t)0x00000000)
#define ADC_DMAMode_Circular                       ADC_CFGR1_DMACFG

#define ADC_AnalogWatchdog_Channel_0                 ((uint32_t)0x00000000)
#define ADC_AnalogWatchdog_Channel_1                 ((uint32_t)0x04000000)
#define ADC_AnalogWatchdog_Channel_2                 ((uint32_t)0x08000000)
#define ADC_AnalogWatchdog_Channel_3                 ((uint32_t)0x0C000000)
#define ADC_AnalogWatchdog_Channel_4                 ((uint32_t)0x10000000)
#define ADC_AnalogWatchdog_Channel_5                 ((uint32_t)0x14000000)
#define ADC_AnalogWatchdog_Channel_6                 ((uint32_t)0x18000000)
#define ADC_AnalogWatchdog_Channel_7                 ((uint32_t)0x1C000000)
#define ADC_AnalogWatchdog_Channel_8                 ((uint32_t)0x20000000)
#define ADC_AnalogWatchdog_Channel_9                 ((uint32_t)0x24000000)
#define ADC_AnalogWatchdog_Channel_10                ((uint32_t)0x28000000) /*!< Not available for STM32F031 devices */
#define ADC_AnalogWatchdog_Channel_11                ((uint32_t)0x2C000000) /*!< Not available for STM32F031 devices */
#define ADC_AnalogWatchdog_Channel_12                ((uint32_t)0x30000000) /*!< Not available for STM32F031 devices */
#define ADC_AnalogWatchdog_Channel_13                ((uint32_t)0x34000000) /*!< Not available for STM32F031 devices */
#define ADC_AnalogWatchdog_Channel_14                ((uint32_t)0x38000000) /*!< Not available for STM32F031 devices */
#define ADC_AnalogWatchdog_Channel_15                ((uint32_t)0x3C000000) /*!< Not available for STM32F031 devices */
#define ADC_AnalogWatchdog_Channel_16                ((uint32_t)0x40000000)
#define ADC_AnalogWatchdog_Channel_17                ((uint32_t)0x44000000)
#define ADC_AnalogWatchdog_Channel_18                ((uint32_t)0x48000000)

#define CALIBRATION_TIMEOUT       ((uint32_t)0x0000F000)

//--- Exported Module Functions ------------
void AdcConfig (void);
unsigned short ReadADC1 (unsigned int);
unsigned short ReadADC1_SameSampleTime (unsigned int);
void SetADC1_SampleTime (void);
unsigned short ReadADC1Check (unsigned char);
unsigned int ADCGetCalibrationFactor (void);

#ifdef ADC_WITH_TEMP_SENSE
void UpdateTemp(void);
unsigned short GetTemp (void);
void FillTempBuffer (void);
short ConvertTemp (unsigned short);
#endif
#endif /* _ADC_H_ */
