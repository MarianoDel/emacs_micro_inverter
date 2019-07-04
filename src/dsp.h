//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DSP.H #################################
//---------------------------------------------

#ifndef _DSP_H_
#define _DSP_H_

//--- Defines for configuration -----------------
#define USE_PID_CONTROLLERS
#define USE_PID_FIXED_CONSTANTS
// #define USE_PID_UPDATED_CONSTANTS

// #define USE_MA8_CIRCULAR
#define USE_MA32_CIRCULAR


//--- Exported constants ------------------------

//--- Exported types ----------------------------

//--- Module Functions --------------------------
unsigned short RandomGen (unsigned int);
unsigned char MAFilter (unsigned char, unsigned char *);
unsigned short MAFilterFast (unsigned short ,unsigned short *);
unsigned short MAFilter8 (unsigned short *);
unsigned short MAFilter32 (unsigned short, unsigned short *);

unsigned short MAFilter32Fast (unsigned short *);
unsigned short MAFilter32Circular (unsigned short, unsigned short *, unsigned char *, unsigned int *);

#ifdef USE_PID_CONTROLLERS
short PID (short, short);
short PID_roof (short, short, short, short *, short *);
#endif

#ifdef USE_MA8_CIRCULAR
void MA8Circular_Start (void);
void MA8Circular_Reset (void);
unsigned short MA8Circular (unsigned short);
#endif
#ifdef USE_MA32_CIRCULAR
void MA32Circular_Start (void);
void MA32Circular_Reset (void);
unsigned short MA32Circular_Calc (void);
void MA32Circular_Load (unsigned short);
#endif

#endif /* _DSP_H_ */
