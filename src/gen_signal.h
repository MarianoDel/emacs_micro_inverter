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

gen_signal_e GenSignalPreDistorted (unsigned short, unsigned short, short *);
void GenSignalPreDistortedReset (void);

gen_signal_e GenSignalSinus (unsigned short, unsigned short, short *);
void GenSignalSinusReset (void);
void GenSignalSinusDutySet (unsigned short d);
void GenSignalSinusResetCntrs (void);
void GenSignalSinusApplyFilter (void);

gen_signal_e GenSignalSinus2 (unsigned short, unsigned short, short *);
void GenSignalSinus2Reset (void);
void GenSignalSinus2ResetCntrs (void);

gen_signal_e GenSignalTriang (unsigned short, unsigned short, short *);
void GenSignalTriangReset (void);
void GenSignalTriangResetCntrs (void);

#endif    /* _GEN_SIGNAL_H_ */

//--- end of file ---//

