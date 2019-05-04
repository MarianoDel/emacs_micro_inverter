//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DSP.H #################################
//---------------------------------------------

#ifndef _DSP_H_
#define _DSP_H_

//--- Exported types ---//


//--- Exported constants ---//
#define USE_PID_CONTROLLERS
#define USE_MA8_CIRCULAR

//--- Module Functions ---//
unsigned short RandomGen (unsigned int);
unsigned char MAFilter (unsigned char, unsigned char *);
unsigned short MAFilterFast (unsigned short ,unsigned short *);
unsigned short MAFilter8 (unsigned short *);
unsigned short MAFilter32 (unsigned short, unsigned short *);

unsigned short MAFilter32Fast (unsigned short *);
unsigned short MAFilter32Circular (unsigned short, unsigned short *, unsigned char *, unsigned int *);

short PID (short, short);
short PID_roof (short, short, short, short *, short *);

void MA8Circular_Start (void);
void MA8Circular_Reset (void);
unsigned short MA8Circular (unsigned short);

#endif /* _DSP_H_ */
