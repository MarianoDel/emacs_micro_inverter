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
#define VER_1_0    //version original


//---- Configuration for Hardware Versions -------
#ifdef VER_2_0
#define HARDWARE_VERSION_2_0
#define SOFTWARE_VERSION_2_0
#endif

#ifdef VER_1_0
#define HARDWARE_VERSION_1_0
#define SOFTWARE_VERSION_1_0
#endif


//---- Features Configuration ----------------
//features are activated here and annouced in hard.c
#define FEATURES

//-- Types of Voltage Sense ----------
#define ONLY_ONE_KB817
// #define ONLY_ONE_KB814
// #define TWO_KB817

//-- Types of programs ----------
#define GRID_TIED_FULL_CONECTED

//-- Types of led indications ----------
// #define USE_LED_FOR_AC_PULSES
// #define USE_LED_FOR_SYNC_PULSES
// #define USE_LED_FOR_ZERO_CROSSING
// #define USE_LED_FOR_VLINE_MAX
#define USE_LED_FOR_MAIN_POLARITY
// #define USE_LED_FOR_MAIN_POLARITY_BEFORE_GEN    //este y el de arriba los puedo combinar
// #define USE_LED_FOR_PROTECTIONS
// #define USE_LED_FOR_PID_CALCS
// #define USE_LED_FOR_MAIN_STATES

//-- Frequency selection ----------
// #define USE_FREQ_48KHZ
#define USE_FREQ_24KHZ
// #define USE_FREQ_16KHZ
// #define USE_FREQ_12KHZ
// #define USE_FREQ_9_6KHZ

//-- How many cycles between the relay conection and the generation phase
//even number for a complete mains cycle
#define CYCLES_BEFORE_START    50

//-- Types of Interrupts ----------
#define WITH_AC_SYNC_INT
#define WITH_OVERCURRENT_SHUTDOWN
#define WITH_SOFT_OVERCURRENT_SHUTDOWN
#define WITH_FEW_CYCLES_OF_50HZ

//-- Some timeouts ----------
#define TT_FOR_CURRENT_ERROR    10000
#define TT_FOR_JUMPER_PROT    1000

//---- End of Features Configuration ----------

//---- Some Checks for avoid Configuration Errors ----------
#ifdef WITH_FEW_CYCLES_OF_50HZ
#define WITH_FEW_CYCLES_OF_50HZ_POS
// #define WITH_FEW_CYCLES_OF_50HZ_NEG
#define TT_FEW_CYCLES_DUMP    10000
#define CYCLES_OF_50HZ    1
#endif
#ifdef WITH_SOFT_OVERCURRENT_SHUTDOWN
#define SOFT_OVERCURRENT_THRESHOLD    3500
#endif

#ifdef HARD_TEST_MODE_STEP_RESPONSE_NEGATIVE
#ifndef ONLY_ONE_KB814
#error "This soft needs a KB814 on board for Voltage Sense"
#endif
#endif

#ifdef INVERTER_MODE_VOLTAGE_FDBK
#if !defined ONLY_ONE_KB814 && \
    !defined ONLY_ONE_KB817
#error "This soft needs an opto to check the Voltage Sense"
#endif
#endif


#if (defined INVERTER_MODE_CURRENT_FDBK) || \
    (defined GRID_TIED_ONLY_SYNC_AND_POLARITY) || \
    (defined GRID_TIED_FULL_CONECTED)
#ifndef ONLY_ONE_KB817
#error "This soft needs only one KB817 on board for Voltage Sense"
#endif
#endif

#if (defined GRID_TIED_ONLY_SYNC_AND_POLARITY) || \
    (defined GRID_TIED_FULL_CONECTED)
#ifndef WITH_AC_SYNC_INT
#error "This soft needs AC_SYNC_INT active to get grid sync"
#endif
#endif


//--- Stringtify Utils -----------------------
#define STRING_CONCAT(str1,str2) #str1 " " #str2
#define STRING_CONCAT_NEW_LINE(str1,str2) xstr(str1) #str2 "\n"
#define xstr_macro(s) str_macro(s)
#define str_macro(s) #s

//--- Hardware Welcome Code ------------------//
#ifdef HARDWARE_VERSION_1_0
#define HARD "Hardware V: 1.0\n"
#endif

//--- Software Welcome Code ------------------//
#ifdef SOFTWARE_VERSION_1_0
#define SOFT "Software V: 1.0\n"
#endif


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

//------- PIN CONFIG ----------------------
#ifdef VER_1_0
//GPIOA pin0	V_Sense / Vline_Sense
//GPIOA pin1	I_Sense_Pos
//GPIOA pin2	I_Sense_Neg

//GPIOA pin3	NC

//GPIOA pin4	
#define PROT_POS    ((GPIOA->IDR & 0x0010) != 0)

//GPIOA pin5
#define PROT_NEG    ((GPIOA->IDR & 0x0020) != 0)

//GPIOA pin6    TIM3_CH1 (L_LEFT)
//GPIOA pin7	TIM3_CH2 (H_LEFT)

//GPIOB pin0    TIM3_CH3 (L_RIGHT)
//GPIOB pin1	TIM3_CH4 (H_RIGHT)

//GPIOA pin8	
#define AC_SYNC ((GPIOA->IDR & 0x0100) != 0)

//GPIOA pin9
//GPIOA pin10	usart1 tx rx

//GPIOA pin11	NC

//GPIOA pin12	
#define LED ((GPIOA->ODR & 0x1000) != 0)
#define LED_ON	GPIOA->BSRR = 0x00001000
#define LED_OFF GPIOA->BSRR = 0x10000000

//GPIOA pin13	NC
//GPIOA pin14	NC

//GPIOA pin15
#define RELAY ((GPIOA->ODR & 0x8000) != 0)
#define RELAY_ON  GPIOA->BSRR = 0x00008000
#define RELAY_OFF GPIOA->BSRR = 0x80000000

//GPIOB pin3	NC
//GPIOB pin4	NC
//GPIOB pin5	NC

//GPIOB pin6
#define STOP_JUMPER ((GPIOB->IDR & 0x0040) == 0)

//GPIOB pin7	NC
#endif

//------- END OF PIN CONFIG -------------------


//AC_SYNC States
typedef enum
{
    START_SYNCING = 0,
    SWITCH_RELAY_TO_ON,
    WAIT_RELAY_TO_ON,
    WAIT_SYNC_FEW_CYCLES_BEFORE_START,
    WAIT_FOR_FIRST_SYNC,
    GEN_POS,
    WAIT_CROSS_POS_TO_NEG,
    GEN_NEG,
    WAIT_CROSS_NEG_TO_POS,
    JUMPER_PROTECTED,
    JUMPER_PROTECT_OFF,
    OVERCURRENT_ERROR,
    FEW_CYCLES_DUMP_DATA,
    FEW_CYCLES_DUMP_DATA_1,
    FEW_CYCLES_DUMP_DATA_2
    
} ac_sync_state_t;

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
#define LED_JUMPER_PROTECTED          4
#define LED_VIN_ERROR                 5
#define LED_OVERCURRENT_POS           6
#define LED_OVERCURRENT_NEG           7


#define SIZEOF_DATA1	512
#define SIZEOF_DATA		256
#define SIZEOF_DATA512	SIZEOF_DATA1
#define SIZEOF_DATA256	SIZEOF_DATA
#define SIZEOF_BUFFTCP	SIZEOF_DATA


#define LED_TOGGLE do { if (LED) \
                            LED_OFF; \
                        else         \
                            LED_ON;  \
                      } while (0)

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
unsigned short VoutTicksToVoltage (unsigned short);
unsigned short VinTicksToVoltage (unsigned short);
void WelcomeCodeFeatures (char *);
    
#endif /* _HARD_H_ */
