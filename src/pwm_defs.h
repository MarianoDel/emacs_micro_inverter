//----------------------------------------------
// ## PWM definitions for the following modules:
// ## tim
// ## gen_signal
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PWM_DEFS.H #############################
//----------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _PWM_DEFS_H_
#define _PWM_DEFS_H_


// Exported Types Constants and Macros -----------------------------------------


#define DUTY_ALWAYS    (DUTY_100_PERCENT + 1)
#define DUTY_NONE		0

#ifdef USE_FREQ_48KHZ
#define DUTY_5_PERCENT		50
#define DUTY_10_PERCENT		100
#define DUTY_45_PERCENT         450
#define DUTY_50_PERCENT		500
#define DUTY_75_PERCENT		750
#define DUTY_100_PERCENT        1000
#endif

#ifdef USE_FREQ_24KHZ
#define DUTY_5_PERCENT		100
#define DUTY_10_PERCENT		200
#define DUTY_45_PERCENT         900
#define DUTY_50_PERCENT		1000
#define DUTY_75_PERCENT		1500
#define DUTY_100_PERCENT        2000
#endif

#ifdef USE_FREQ_12KHZ
#define DUTY_5_PERCENT		200
#define DUTY_10_PERCENT		400
#define DUTY_45_PERCENT         1800
#define DUTY_50_PERCENT		2000
#define DUTY_75_PERCENT		3000
#define DUTY_100_PERCENT        4000
#endif



#endif    /* _PWM_DEFS_H_ */

//--- end of file ---//

