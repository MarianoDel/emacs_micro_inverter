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

#define DELTA_T1_BAR_FOR_49HZ    1152
#define DELTA_T1_BAR_FOR_51HZ    1048

#define TT_FOR_NO_SYNC    60

#define VLINE_SENSE_MIN_THRESOLD    200


//--- Exported types ----------------------------
typedef enum {
    POLARITY_UNKNOWN = 0,
    POLARITY_POS,
    POLARITY_NEG

} polarity_t;


//--- Module Functions --------------------------
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


#endif    /* __SYNC_H_ */
