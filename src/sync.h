//-----------------------------------------------
// #### MICROINVERTER PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### SYNC.C ##################################
//-----------------------------------------------
#ifndef __SYNC_H_
#define __SYNC_H_


//--- Defines for configuration -----------------



//--- Exported constants ------------------------
#define DELTA_T2_FOR_49HZ    10204
#define DELTA_T2_FOR_51HZ     9804

// ancho minimo y maximo que permito del pulso de sync
// #define DELTA_T1_BAR_FOR_49HZ    1152
// #define DELTA_T1_BAR_FOR_51HZ    1048
#define DELTA_T1_HALF_BAR_MAX    1200
#define DELTA_T1_HALF_BAR_MIN    400    //16-12-19 cambio 600 por 400, cuando genera (en lampara) achica el pulso

#define DELTA_T1_MAX    2400
#define DELTA_T1_MIN    800

#define TT_FOR_NO_SYNC    60

#define VLINE_SENSE_MIN_THRESOLD    200
// #define VLINE_SENSE_MAX    520    //supongo 220Vac en 465 +/- 10% 13-04 muevo a 520 por las lamparas en serie
#define VLINE_SENSE_MAX    540
#define VLINE_SENSE_MIN    420


//--- Exported types ----------------------------
typedef enum {
    POLARITY_UNKNOWN = 0,
    POLARITY_POS,
    POLARITY_NEG

} polarity_t;

typedef enum {
    SYNC_PULSE_NONE = 0,
    SYNC_PULSE_TO_LOW,
    SYNC_PULSE_TO_HIGH,
    SYNC_PULSE_TO_LATE,
    SYNC_PULSE_IS_GOOD

} sync_pulse_t;


//--- Module Functions --------------------------
void SYNC_InitSetup (void);
void SYNC_Update_Sync (void);
void SYNC_Update_Polarity (void);
void SYNC_Rising_Edge_Handler (void);
void SYNC_Falling_Edge_Handler (void);
void SYNC_Zero_Crossing_Handler (void);
polarity_t SYNC_Last_Polarity_Check (void);
unsigned char SYNC_Sync_Now (void);
void SYNC_Sync_Now_Reset (void);
unsigned char SYNC_Cycles_Cnt (void);
void SYNC_Cycles_Cnt_Reset (void);
unsigned short SYNC_Vline_Max (void);
unsigned char SYNC_All_Good (void);
void SYNC_Restart (void);
unsigned short SYNC_delta_t1_half_bar (void);
unsigned short SYNC_delta_t2_bar (void);
unsigned short SYNC_delta_t2 (void);
unsigned short SYNC_delta_t1 (void);
unsigned short SYNC_vline_max (void);
unsigned char SYNC_Frequency_Check (void);
unsigned char SYNC_Pulses_Check (void);
unsigned char SYNC_Vline_Check (void);
    
#endif    /* __SYNC_H_ */
