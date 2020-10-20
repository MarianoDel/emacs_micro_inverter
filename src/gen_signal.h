//----------------------------------------------
// ## Signal Generation Module
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### GEN_SIGNAL.H #############################
//----------------------------------------------

// Prevent recursive inclusion -------------------------------------------------
#ifndef _GEN_SIGNAL_H_
#define _GEN_SIGNAL_H_


// Exported Types Constants and Macros -----------------------------------------
typedef enum {
    SIGNAL_RUNNING,
    SIGNAL_FINISH

} gen_signal_e;


// Exported Module Functions ---------------------------------------------------
gen_signal_e GenSignal (unsigned short, unsigned short, short *);
void GenSignalReset (void);


#endif    /* _GEN_SIGNAL_H_ */

//--- end of file ---//

