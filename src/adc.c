//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### ADC.C #################################
//---------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include "stm32f0xx.h"
#include "hard.h"


/* Externals ------------------------------------------------------------------*/
extern volatile unsigned short adc_ch [];


#ifdef ADC_WITH_INT
extern volatile unsigned char seq_ready;
#endif

#ifdef ADC_WITH_TEMP_SENSE
extern volatile unsigned short tt_take_temp_sample;
#endif

/* Globals ------------------------------------------------------------------*/
#ifdef ADC_WITH_INT
volatile unsigned short * p_channel;
#endif

#ifdef ADC_WITH_TEMP_SENSE
// ------- del sensor de Temperatura -------
unsigned short board_temp [SIZEOF_BOARD_TEMP];
unsigned short last_temp = 0;
unsigned char board_temp_index = 0;
unsigned char new_temp_sample = 0;
#endif


/* Module Functions -----------------------------------------------------------*/
//Single conversion mode (CONT=0)
//In Single conversion mode, the ADC performs a single sequence of conversions,
//converting all the channels once.

//Continuous conversion mode (CONT=1)
//In continuous conversion mode, when a software or hardware trigger event occurs,
//the ADC performs a sequence of conversions, converting all the channels once and then
//automatically re-starts and continuously performs the same sequence of conversions

//Discontinuous mode (DISCEN)
//In this mode (DISCEN=1), a hardware or software trigger event is required to start
//each conversion defined in the sequence. Only with (CONT=0)

void AdcConfig (void)
{
    if (!RCC_ADC_CLK)
        RCC_ADC_CLK_ON;

    // preseteo los registros a default, la mayoria necesita tener ADC apagado
    ADC1->CR = 0x00000000;
    ADC1->IER = 0x00000000;
    ADC1->CFGR1 = 0x00000000;
    ADC1->CFGR2 = 0x00000000;
    ADC1->SMPR = 0x00000000;
    ADC1->TR = 0x0FFF0000;
    ADC1->CHSELR = 0x00000000;

    //set clock
    ADC1->CFGR2 = ADC_ClockMode_SynClkDiv4;

    //set resolution, trigger & Continuos or Discontinuous
    // ADC1->CFGR1 |= ADC_Resolution_10b | ADC_ExternalTrigConvEdge_Rising | ADC_ExternalTrigConv_T3_TRGO;
    ADC1->CFGR1 |= ADC_Resolution_12b | ADC_ExternalTrigConvEdge_Rising | ADC_ExternalTrigConv_T3_TRGO;
    // ADC1->CFGR1 |= ADC_Resolution_10b | ADC_ExternalTrigConvEdge_Rising | ADC_ExternalTrigConv_T1_TRGO;
    //ADC1->CFGR1 |= ADC_Resolution_12b | ADC_CFGR1_DISCEN;
    // ADC1->CFGR1 |= ADC_Resolution_12b;
    //recordar ADC1->CR |= ADC_CR_ADSTART para activar conversiones continuas

    //set sampling time
    ADC1->SMPR |= ADC_SampleTime_71_5Cycles;
    // ADC1->SMPR |= ADC_SampleTime_41_5Cycles;
    // ADC1->SMPR |= ADC_SampleTime_28_5Cycles;
    //ADC1->SMPR |= ADC_SampleTime_7_5Cycles;
    //las dos int (usar DMA?) y pierde el valor intermedio
    //ADC1->SMPR |= ADC_SampleTime_1_5Cycles;

    //set channel selection
    ADC1->CHSELR |= ADC_All_Orer_Channels;
    
#ifdef ADC_WITH_INT        
    //set interrupts
    ADC1->IER |= ADC_IT_EOC;

    //set pointer
    p_channel = &adc_ch[0];

    NVIC_EnableIRQ(ADC1_IRQn);
    NVIC_SetPriority(ADC1_IRQn, 3);
#endif

#ifdef ADC_WITH_TEMP_SENSE
    ADC->CCR |= ADC_CCR_TSEN;
#endif

    //calibrar ADC
    ADCGetCalibrationFactor();

#ifdef ADC_WITH_DMA
    ADC1->CFGR1 |= ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG;
#endif
    
    // Enable ADC1
    ADC1->CR |= ADC_CR_ADEN;

}

#ifdef ADC_WITH_INT
void ADC1_COMP_IRQHandler (void)
{
    if (ADC1->ISR & ADC_IT_EOC)
    {
        if (ADC1->ISR & ADC_IT_EOSEQ)	//seguro que es channel4 en posicion 3 en ver_1_1, 3 y 2 en ver_1_0
        {
            p_channel = &adc_ch[ADC_LAST_CHANNEL_QUANTITY];
            *p_channel = ADC1->DR;
            p_channel = &adc_ch[0];
            seq_ready = 1;
        }
        else
        {
            *p_channel = ADC1->DR;		//
            if (p_channel < &adc_ch[ADC_LAST_CHANNEL_QUANTITY])
                p_channel++;
        }
        //clear pending
        ADC1->ISR |= ADC_IT_EOC | ADC_IT_EOSEQ;
    }
}
#endif


//Setea el sample time en el ADC
void SetADC1_SampleTime (void)
{
    uint32_t tmpreg = 0;

    /* Clear the Sampling time Selection bits */
    tmpreg &= ~ADC_SMPR1_SMPR;

    /* Set the ADC Sampling Time register */
    tmpreg |= (uint32_t)ADC_SampleTime_239_5Cycles;

    /* Configure the ADC Sample time register */
    ADC1->SMPR = tmpreg ;
}


//lee el ADC sin cambiar el sample time anterior
unsigned short ReadADC1_SameSampleTime (unsigned int channel)
{
    // Configure the ADC Channel
    ADC1->CHSELR = channel;

    // Start the conversion
    ADC1->CR |= (uint32_t)ADC_CR_ADSTART;

    // Wait until conversion completion
    while((ADC1->ISR & ADC_ISR_EOC) == 0);

    // Get the conversion value
    return (uint16_t) ADC1->DR;
}

unsigned short ReadADC1Check (unsigned char channel)
{
    if (ADC1->CR & 0x01)			//reviso ADEN
        return 0xFFFF;

    //espero que este listo para convertir
    while ((ADC1->ISR & 0x01) == 0);	//espero ARDY = 1

    if ((ADC1->CFGR1 & 0x00010000) == 0)			//reviso DISCONTINUOS = 1
        return 0xFFFF;

    if (ADC1->CFGR1 & 0x00002000)					//reviso CONT = 0
        return 0xFFFF;

    if (ADC1->CFGR1 & 0x00000C00)					//reviso TRIGGER = 00
        return 0xFFFF;

    if (ADC1->CFGR1 & 0x00000020)					//reviso ALIGN = 0
        return 0xFFFF;

    if (ADC1->CFGR1 & 0x00000018)					//reviso RES = 00
        return 0xFFFF;

    //espero que no se este convirtiendo ADCSTART = 0
    while ((ADC1->CR & 0x02) != 0);	//espero ADCSTART = 0

    ADC1->CHSELR = 0x00000001;	//solo convierto CH0

    return 1;
}

unsigned int ADCGetCalibrationFactor (void)
{
    uint32_t tmpreg = 0, calibrationcounter = 0, calibrationstatus = 0;

    /* Set the ADC calibartion */
    ADC1->CR |= (uint32_t)ADC_CR_ADCAL;

    /* Wait until no ADC calibration is completed */
    do
    {
        calibrationstatus = ADC1->CR & ADC_CR_ADCAL;
        calibrationcounter++;
    } while((calibrationcounter != CALIBRATION_TIMEOUT) && (calibrationstatus != 0x00));

    if((uint32_t)(ADC1->CR & ADC_CR_ADCAL) == RESET)
    {
        /*Get the calibration factor from the ADC data register */
        tmpreg = ADC1->DR;
    }
    else
    {
        /* Error factor */
        tmpreg = 0x00000000;
    }
    return tmpreg;
}

#ifdef ADC_WITH_TEMP_SENSE
void UpdateTemp(void)
{
    //hago update cada 1 seg
    if (!tt_take_temp_sample)
    {
        tt_take_temp_sample = 1000;

        board_temp [board_temp_index] = ReadADC1_SameSampleTime(ADC_CH16);
        //board_temp [0] = ReadADC1_SameSampleTime(ADC_CH16);

        if (board_temp_index < (SIZEOF_BOARD_TEMP - 1))
            board_temp_index++;
        else
            board_temp_index = 0;

        new_temp_sample = 1;
    }
}

//devuelve el valor promedio de la temperatura
//si existen nuevas muestras hace la cuenta, sino contesta el ultimo valor calculado
unsigned short GetTemp (void)
{
    unsigned char i;
    unsigned int t = 0;

    if (new_temp_sample)
    {
        for (i = 0; i < SIZEOF_BOARD_TEMP; i++)
            t += board_temp[i];

        last_temp = t >> 3;
        new_temp_sample = 0;
    }

    return ConvertTemp(last_temp);
}

void FillTempBuffer (void)
{
    unsigned char i;
    unsigned short dummy;

    dummy = ReadADC1_SameSampleTime(ADC_CH16);

    for (i = 0; i < SIZEOF_BOARD_TEMP; i++)
        board_temp[i] = dummy;

}

short ConvertTemp (unsigned short adc_temp)
{
    int32_t temperature; /* will contain the temperature in degree Celsius */
    //temperature = (((int32_t) ADC1->DR * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR );
    temperature = (int32_t) *TEMP30_CAL_ADDR - adc_temp;
    temperature *= 1000;
    temperature = temperature / 5336;	//4.3mV / °C
    temperature = temperature + 30;

    return (short) temperature;
}
#endif //ADC_WITH_TEMP_SENSE

//--- end of file ---//


