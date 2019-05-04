//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### HARD.H ################################
//---------------------------------------------
#ifndef _HARD_H_
#define _HARD_H_

//----------- Defines For Configuration -------------
//----------- Hardware Board Version -------------
#define VER_2_0    //usa control por tension mas proteccion de corriente pico


#define VOUT_SETPOINT    VOUT_200V
#define VOUT_HIGH_MODE_CHANGE    VOUT_205V
#define VOUT_LOW_MODE_CHANGE    VOUT_195V

#define DUTY_TO_CHANGE_CURRENT_MODE    25
#define DUTY_TO_CHANGE_VOLTAGE_MODE    18

#define VOUT_OVERVOLTAGE_THRESHOLD_TO_DISCONNECT    VOUT_400V
#define VOUT_OVERVOLTAGE_THRESHOLD_TO_RECONNECT    VOUT_350V

// #define VIN_UNDERVOLTAGE_THRESHOLD_TO_DISCONNECT    VIN_17V
#define VIN_UNDERVOLTAGE_THRESHOLD_TO_DISCONNECT    VIN_10V
#define VIN_UNDERVOLTAGE_THRESHOLD_TO_RECONNECT    VIN_12V

//---- Configuration for Hardware Versions -------
#ifdef VER_2_0
#define HARDWARE_VERSION_2_0
#define SOFTWARE_VERSION_2_0
#endif


//---- Features Configuration ----------------
//features are activeted here and annouced in hard.c
#define FEATURES

// SOFT para VERSIONES 2.0
#ifdef VER_2_0
//-- Types of programs ----------
// #define USE_FORWARD_MODE
#define USE_PUSH_PULL_MODE

#ifdef USE_PUSH_PULL_MODE
// #define IN_PUSH_PULL_SET_FIXED_D        //setea d a DUTY_FOR_DMAX - usareste valor chico!! -
#define IN_PUSH_PULL_GROW_TO_FIXED_D    //setea d a DUTY_FOR_DMAX pero incrementando de a poco cada 2ms
// #define IN_PUSH_PULL_VM                 //el d esta definido por la Vout
#endif
//-- Types of led indications ----------
// #define USE_LED_IN_INT
#define USE_LED_IN_PROT

//-- Other configs features ----------
// #define WITH_OVERCURRENT_SHUTDOWN
// #define WITH_TIM14_FB
#define WITH_TIM1_FB

//-- Frequency selection ----------
#define USE_FREQ_75KHZ
// #define USE_FREQ_48KHZ

//-- Types of used Hardware ----------
#ifdef USE_FORWARD_MODE
#define USE_ONLY_MOSFET_A
#endif
#ifdef USE_PUSH_PULL_MODE
#define USE_MOSFET_A_AND_B
#endif

#endif    //ver2.0



//------ Configuration for Firmware-Channels -----


//---- End of Features Configuration ----------

//--- Stringtify Utils -----------------------
#define STRING_CONCAT(str1,str2) #str1 " " #str2
#define STRING_CONCAT_NEW_LINE(str1,str2) xstr(str1) #str2 "\n"
#define xstr_macro(s) str_macro(s)
#define str_macro(s) #s

//--- Hardware Welcome Code ------------------//
#ifdef HARDWARE_VERSION_2_0
#define HARD "Hardware V: 2.0\n"
#endif

//--- Software Welcome Code ------------------//
#ifdef SOFTWARE_VERSION_2_0
#define SOFT "Software V: 2.0\n"
#endif


//-------- Others Configurations depending on the formers ------------
//-------- Hysteresis Conf ------------------------

//-------- PWM Conf ------------------------

//-------- End Of Defines For Configuration ------

#define VIN_35V    986
#define VIN_30V    845
#define VIN_25V    704
#define VIN_20V    561    //1.81V
#define VIN_17V    477
#define VIN_15V    423
#define VIN_12V    338
#define VIN_10V    282


// #define VOUT_200V    415
#define VOUT_110V    151    //ajustado 05-08-18
#define VOUT_200V    386    //ajustado 24-07-18
#define VOUT_205V    399    
#define VOUT_195V    373
#define VOUT_300V    660    //ajustado 24-07-18
#define VOUT_350V    802    //ajustado 24-07-18
#define VOUT_400V    917    //

//Caracteristicas de la bobina de salida
// #define LOUT_UHY    130    //DINL2
// #define LOUT_UHY    330    //doble bobina amarilla
// #define ILOUT       3      //doble bobina amarilla
// #define LOUT_UHY    2100    //POL12050 bobinado primario
// #define ILOUT       1      //POL12050
#define LOUT_UHY    6400    //nueva bobina ale 6.4mHy
#define ILOUT       1      //nueva ale
#define TICK_PWM_NS 21
#define N_TRAFO     18300
#define IMAX_INPUT  25
#define MAX_VOUT    830    //830 -> 362V tension maxima que sale del trafo en puntos ADC

#if ((ILOUT * N_TRAFO) > (IMAX_INPUT * 1000))
#define I_FOR_CALC_MILLIS (IMAX_INPUT * 1000 * 1000 / N_TRAFO)
#define I_FOR_CALC (IMAX_INPUT * 1000 / N_TRAFO)
#else
#define I_FOR_CALC_MILLIS (ILOUT * 1000)
#define I_FOR_CALC (IMAX_INPUT * 1000)
#endif

#define VOUT_SOFT_START    VOUT_110V

#define DMAX_HARDWARE    450

#ifdef TEST_FIXED_D
#define D_FOR_FIXED    20
#endif

//------- PIN CONFIG ----------------------
#ifdef VER_2_0
//GPIOA pin0	Vin_Sense
//GPIOA pin1	Vout_Sense
//GPIOA pin2	I_Sense

//GPIOA pin3	NC

//GPIOA pin4	
#define PROT_MOS	((GPIOA->IDR & 0x0010) == 0)

//GPIOA pin5
#define PROT_MOS_2 ((GPIOA->IDR & 0x0020) == 0)

//GPIOA pin6	para TIM3_CH1 (MOSFET_A)

//GPIOA pin7	SENSE_MOSFET_A
#define SENSE_MOSFET_A ((GPIOA->IDR & 0x0080) != 0)

//GPIOB pin0    NC

//GPIOB pin1	para TIM14_CH1    Output max peak current

//GPIOA pin8	para TIM1_CH1 (MOSFET_B)

//GPIOA pin9
//GPIOA pin10	usart1 tx rx

//GPIOA pin11	SENSE_MOSFET_B
#define SENSE_MOSFET_B ((GPIOA->IDR & 0x0800) != 0)

//GPIOA pin12	NC
//GPIOA pin13	NC
//GPIOA pin14	NC

//GPIOA pin15
#define LED ((GPIOA->ODR & 0x8000) != 0)
#define LED_ON	GPIOA->BSRR = 0x00008000
#define LED_OFF GPIOA->BSRR = 0x80000000

//GPIOB pin3	NC
//GPIOB pin4	NC
//GPIOB pin5	NC

//GPIOB pin6
#define STOP_JUMPER ((GPIOB->IDR & 0x0040) == 0)

//GPIOB pin7	NC
#endif	//VER_2_0

//------- END OF PIN CONFIG -------------------



//ESTADOS DEL PROGRAMA PRINCIPAL
typedef enum
{
    MAIN_INIT = 0,
    MAIN_SOFT_START,
    MAIN_VOLTAGE_MODE,
    MAIN_CURRENT_MODE,
    MAIN_OVERCURRENT,
    MAIN_JUMPER_PROTECTED,
    MAIN_JUMPER_PROTECT_OFF,    
    MAIN_GO_TO_FAILURE,
    MAIN_OVERVOLTAGE,
    MAIN_UNDERVOLTAGE,        
    MAINS_FAILURE

} main_state_t;

//ESTADOS DEL LED
typedef enum
{    
    START_BLINKING = 0,
    WAIT_TO_OFF,
    WAIT_TO_ON,
    WAIT_NEW_CYCLE
} led_state_t;


//Estados Externos de LED BLINKING
#define LED_NO_BLINKING               0
#define LED_STANDBY                   1
#define LED_GENERATING                2
#define LED_LOW_VOLTAGE               3
#define LED_PROTECTED                 4
#define LED_VIN_ERROR                 5
#define LED_OVERCURRENT_ERROR         6


#define SIZEOF_DATA1	512
#define SIZEOF_DATA		256
#define SIZEOF_DATA512	SIZEOF_DATA1
#define SIZEOF_DATA256	SIZEOF_DATA
#define SIZEOF_BUFFTCP	SIZEOF_DATA





/* Module Functions ------------------------------------------------------------*/
unsigned short GetHysteresis (unsigned char);
unsigned char GetNew1to10 (unsigned short);
void UpdateVGrid (void);
void UpdateIGrid (void);
unsigned short GetVGrid (void);
unsigned short GetIGrid (void);
unsigned short PowerCalc (unsigned short, unsigned short);
unsigned short PowerCalcMean8 (unsigned short * p);
void ShowPower (char *, unsigned short, unsigned int, unsigned int);
void ChangeLed (unsigned char);
void UpdateLed (void);
unsigned short UpdateDMAX (unsigned short);
unsigned short UpdateDMAXSF (unsigned short);
unsigned short UpdateDmaxLout (unsigned short);
unsigned short VoutTicksToVoltage (unsigned short);
unsigned short VinTicksToVoltage (unsigned short);
unsigned short Hard_GetDmaxLout (unsigned short, unsigned short);
void WelcomeCodeFeatures (char *);
    
#endif /* _HARD_H_ */
