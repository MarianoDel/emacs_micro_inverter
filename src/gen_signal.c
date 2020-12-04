//------------------------------------------------
// ## Signal Generation Module
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### GEN_SIGNAL.C #############################
//------------------------------------------------

// Includes --------------------------------------------------------------------
#include "gen_signal.h"
#include "pwm_defs.h"
#include "hard.h"
#include "dsp.h"


// Module Private Types Constants and Macros -----------------------------------
#define INDEX_TO_MIDDLE    (60 - 1)
#define INDEX_TO_FALLING   (180 - 1)
#define INDEX_TO_REVERT    204
    

#if (SIZEOF_SIGNAL == 120)
unsigned short sin_half_cycle [SIZEOF_SIGNAL] = {107,214,321,428,534,640,746,851,955,1059,
                                                 1163,1265,1366,1467,1567,1665,1762,1859,1953,2047,
                                                 2139,2230,2319,2406,2492,2577,2659,2740,2818,2895,
                                                 2970,3043,3113,3182,3248,3312,3374,3434,3491,3546,
                                                 3598,3648,3696,3740,3783,3823,3860,3894,3926,3955,
                                                 3981,4005,4026,4044,4059,4072,4082,4089,4093,4095,
                                                 4093,4089,4082,4072,4059,4044,4026,4005,3981,3955,
                                                 3926,3894,3860,3823,3783,3740,3696,3648,3598,3546,
                                                 3491,3434,3374,3312,3248,3182,3113,3043,2970,2895,
                                                 2818,2740,2659,2577,2492,2406,2319,2230,2139,2047,
                                                 1953,1859,1762,1665,1567,1467,1366,1265,1163,1059,
                                                 955,851,746,640,534,428,321,214,107,0};


#elif (SIZEOF_SIGNAL == 240)
unsigned short sin_half_cycle [SIZEOF_SIGNAL] = {53,107,160,214,267,321,374,428,481,534,
                                                 587,640,693,746,798,851,903,955,1007,1059,
                                                 1111,1163,1214,1265,1316,1366,1417,1467,1517,1567,
                                                 1616,1665,1714,1762,1811,1859,1906,1953,2000,2047,
                                                 2093,2139,2185,2230,2275,2319,2363,2406,2450,2492,
                                                 2535,2577,2618,2659,2700,2740,2779,2818,2857,2895,
                                                 2933,2970,3007,3043,3078,3113,3148,3182,3215,3248,
                                                 3281,3312,3344,3374,3404,3434,3463,3491,3519,3546,
                                                 3572,3598,3624,3648,3672,3696,3718,3740,3762,3783,
                                                 3803,3823,3841,3860,3877,3894,3910,3926,3941,3955,
                                                 3969,3981,3994,4005,4016,4026,4035,4044,4052,4059,
                                                 4066,4072,4077,4082,4086,4089,4091,4093,4094,4095,
                                                 4094,4093,4091,4089,4086,4082,4077,4072,4066,4059,
                                                 4052,4044,4035,4026,4016,4005,3994,3981,3969,3955,
                                                 3941,3926,3910,3894,3877,3860,3841,3823,3803,3783,
                                                 3762,3740,3718,3696,3672,3648,3624,3598,3572,3546,
                                                 3519,3491,3463,3434,3404,3374,3344,3312,3281,3248,
                                                 3215,3182,3148,3113,3078,3043,3007,2970,2933,2895,
                                                 2857,2818,2779,2740,2700,2659,2618,2577,2535,2492,
                                                 2450,2406,2363,2319,2275,2230,2185,2139,2093,2047,
                                                 2000,1953,1906,1859,1811,1762,1714,1665,1616,1567,
                                                 1517,1467,1417,1366,1316,1265,1214,1163,1111,1059,
                                                 1007,955,903,851,798,746,693,640,587,534,
                                                 481,428,374,321,267,214,160,107,53,0};


#else
#error "Select SIZEOF_SIGNAL in gen_signal.h"
#endif


typedef enum {
    SIGNAL_RISING = 0,
    SIGNAL_MIDDLE,
    SIGNAL_FALLING,
    SIGNAL_REVERT
        
} signal_state_e;


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned short * p_current_ref;
pid_data_obj_t current_pid;
signal_state_e gen_signal_state = SIGNAL_RISING;


// Module Private Functions ----------------------------------------------------
unsigned short CurrentLoop (unsigned short, unsigned short);
void CurrentLoop_Change_to_LowGain (void);
void CurrentLoop_Change_to_HighGain (void);


// Module Functions ------------------------------------------------------------
gen_signal_e GenSignal (unsigned short i_sample, unsigned short peak_current, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;
    
    //Adelanto la seniales de corriente,
    if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
        //loop de corriente
        unsigned int calc = *p_current_ref * peak_current;
        calc = calc >> 12;

        switch (gen_signal_state)
        {
        case SIGNAL_RISING:
            *duty = CurrentLoop ((unsigned short) calc, i_sample);

            if (signal_index > INDEX_TO_MIDDLE)
            {
                CurrentLoop_Change_to_HighGain();
                gen_signal_state = SIGNAL_MIDDLE;
            }
            break;

        case SIGNAL_MIDDLE:
            *duty = CurrentLoop ((unsigned short) calc, i_sample);

            if (signal_index > INDEX_TO_FALLING)
            {
                CurrentLoop_Change_to_LowGain();
                gen_signal_state = SIGNAL_FALLING;
            }
            break;

        case SIGNAL_FALLING:
            *duty = CurrentLoop ((unsigned short) calc, i_sample);

            if (signal_index > INDEX_TO_REVERT)
            {
                // CurrentLoop_Change_to_LowGain();
                gen_signal_state = SIGNAL_REVERT;
                *duty = 0;
            }
            break;

        case SIGNAL_REVERT:
            // *duty = CurrentLoop ((unsigned short) calc, i_sample);

            // if (signal_index > 204)
            // {
            //     CurrentLoop_Change_to_LowGain();
            //     gen_signal_state = SIGNAL_REVERT;
            // }
            break;
                        
        }                    
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        resp = SIGNAL_FINISH;

    return resp;
}


void GenSignalReset (void)
{
    p_current_ref = sin_half_cycle;
    PID_Flush_Errors(&current_pid);
    CurrentLoop_Change_to_LowGain();
    gen_signal_state = SIGNAL_RISING;
}


unsigned short CurrentLoop (unsigned short setpoint, unsigned short new_sample)
{
    short d = 0;
    
    current_pid.setpoint = setpoint;
    current_pid.sample = new_sample;
    // d = PID(&current_pid);
    d = PI(&current_pid);    
                    
    if (d > 0)
    {
        if (d > DUTY_100_PERCENT)
        {
            d = DUTY_100_PERCENT;
            current_pid.last_d = DUTY_100_PERCENT;
        }
    }
    else
    {
        d = DUTY_NONE;
        current_pid.last_d = DUTY_NONE;
    }

    return (unsigned short) d;
}


void CurrentLoop_Change_to_HighGain (void)
{
    current_pid.kp = 40;
    current_pid.ki = 1;
    current_pid.kd = 0;
}


void CurrentLoop_Change_to_LowGain (void)
{
    current_pid.kp = 10;
    current_pid.ki = 3;
    current_pid.kd = 0;    
}

//--- end of file ---//
