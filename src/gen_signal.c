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

#define SINUS_INDEX_TO_MIDDLE    (116 - 1)
#define SINUS_INDEX_TO_FALLING   (124 - 1)
#define SINUS_INDEX_TO_REVERT    150    //modif 26-12-2020, hago el corte mas rapido
// #define SINUS_INDEX_TO_REVERT    204

#define TRIANG_INDEX_TO_CHECK_MAX_CURRENT    (204 - 1)


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


unsigned short triang_half_cycle [SIZEOF_SIGNAL] = {0,68,136,204,273,341,409,477,546,614,
                                                    682,750,819,887,955,1023,1092,1160,1228,1296,
                                                    1365,1433,1501,1569,1638,1706,1774,1842,1911,1979,
                                                    2047,2115,2184,2252,2320,2388,2457,2525,2593,2661,
                                                    2730,2798,2866,2934,3003,3071,3139,3207,3276,3344,
                                                    3412,3480,3549,3617,3685,3753,3822,3890,3958,4026,
                                                    4095,4026,3958,3890,3822,3753,3685,3617,3549,3480,
                                                    3412,3344,3276,3207,3139,3071,3003,2934,2866,2798,
                                                    2730,2661,2593,2525,2457,2388,2320,2252,2184,2115,
                                                    2047,1979,1911,1842,1774,1706,1638,1569,1501,1433,
                                                    1365,1296,1228,1160,1092,1023,955,887,819,750,
                                                    682,614,546,477,409,341,273,204,136,68};

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

// unsigned short triang_half_cycle [SIZEOF_SIGNAL] = {0,34,68,102,136,170,204,238,273,307,
//                                                     341,375,409,443,477,511,546,580,614,648,
//                                                     682,716,750,784,819,853,887,921,955,989,
//                                                     1023,1057,1092,1126,1160,1194,1228,1262,1296,1330,
//                                                     1365,1399,1433,1467,1501,1535,1569,1603,1638,1672,
//                                                     1706,1740,1774,1808,1842,1876,1911,1945,1979,2013,
//                                                     2047,2081,2115,2149,2184,2218,2252,2286,2320,2354,
//                                                     2388,2422,2457,2491,2525,2559,2593,2627,2661,2695,
//                                                     2730,2764,2798,2832,2866,2900,2934,2968,3003,3037,
//                                                     3071,3105,3139,3173,3207,3241,3276,3310,3344,3378,
//                                                     3412,3446,3480,3514,3549,3583,3617,3651,3685,3719,
//                                                     3753,3787,3822,3856,3890,3924,3958,3992,4026,4060,
//                                                     4095,4060,4026,3992,3958,3924,3890,3856,3822,3787,
//                                                     3753,3719,3685,3651,3617,3583,3549,3514,3480,3446,
//                                                     3412,3378,3344,3310,3276,3241,3207,3173,3139,3105,
//                                                     3071,3037,3003,2968,2934,2900,2866,2832,2798,2764,
//                                                     2730,2695,2661,2627,2593,2559,2525,2491,2457,2422,
//                                                     2388,2354,2320,2286,2252,2218,2184,2149,2115,2081,
//                                                     2047,2013,1979,1945,1911,1876,1842,1808,1774,1740,
//                                                     1706,1672,1638,1603,1569,1535,1501,1467,1433,1399,
//                                                     1365,1330,1296,1262,1228,1194,1160,1126,1092,1057,
//                                                     1023,989,955,921,887,853,819,784,750,716,
//                                                     682,648,614,580,546,511,477,443,409,375,
//                                                     341,307,273,238,204,170,136,102,68,34};

unsigned short triang_half_cycle [SIZEOF_SIGNAL] = {0,34,68,102,136,170,204,238,273,307,
                                                    341,375,409,443,477,511,546,580,614,648,
                                                    682,716,750,784,819,853,887,921,955,989,
                                                    1023,1057,1092,1126,1160,1194,1228,1262,1296,1330,
                                                    1365,1399,1433,1467,1501,1535,1569,1603,1638,1672,
                                                    1706,1740,1774,1808,1842,1876,1911,1945,1979,2013,
                                                    2047,2081,2115,2149,2184,2218,2252,2286,2320,2354,
                                                    2388,2422,2457,2491,2525,2559,2593,2627,2661,2695,
                                                    2730,2764,2798,2832,2866,2900,2934,2968,3003,3037,
                                                    3071,3105,3139,3173,3207,3241,3276,3310,3344,3378,
                                                    3412,3446,3480,3514,3549,3583,3617,3651,3685,3719,
                                                    3753,3787,3822,3856,3890,3924,3958,3992,4026,4060,
                                                    3071,3037,3003,2968,2934,2900,2866,2832,2798,2764,
                                                    2388,2354,2320,2286,2252,2218,2184,2149,2115,2081,
                                                    1706,1672,1638,1603,1569,1535,1501,1467,1433,1399,
                                                    1023,989,955,921,887,853,819,784,750,716,
                                                    341,307,273,238,204,170,136,102,68,34,
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,0,0,0,0,0,0,0};

unsigned short duty_pre_dist [SIZEOF_SIGNAL] =
{
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,

    60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
    85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
    120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
    180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180,

    230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
    280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
    280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
    220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220,

    155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,

    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};


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
void CurrentLoop_Change_to_RevertGain (void);


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
                CurrentLoop_Change_to_RevertGain();
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
    //vsense directo de Rsense
    // current_pid.kp = 128;
    // current_pid.ki = 33;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    current_pid.kp = 29;
    current_pid.ki = 7;
    current_pid.kd = 0;
    
    PID_Flush_Only_Errors(&current_pid);    
}


void CurrentLoop_Change_to_LowGain (void)
{
    //vsense directo de Rsense    
    // current_pid.kp = 128;
    // current_pid.ki = 33;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    current_pid.kp = 29;
    current_pid.ki = 7;
    current_pid.kd = 0;
    
    PID_Flush_Only_Errors(&current_pid);    
}


void CurrentLoop_Change_to_RevertGain (void)
{
    //vsense directo de Rsense    
    // current_pid.kp = 128;
    // current_pid.ki = 33;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    current_pid.kp = 29;
    current_pid.ki = 7;
    current_pid.kd = 0;
    
    PID_Flush_Only_Errors(&current_pid);    
}


gen_signal_e GenSignalPreDistorted (unsigned short i_sample, unsigned short peak_current, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;
    
    //Adelanto la seniales de corriente,
    if (p_current_ref < &duty_pre_dist[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - duty_pre_dist);
                    
        //loop de corriente
        unsigned int calc = *p_current_ref * peak_current;
        calc = calc >> 8;

        switch (gen_signal_state)
        {
        case SIGNAL_RISING:
            *duty = (unsigned short) calc;
            // *duty = duty_pre_dist[signal_index];

            if (signal_index > INDEX_TO_MIDDLE)
                gen_signal_state = SIGNAL_MIDDLE;

            break;

        case SIGNAL_MIDDLE:
            // now check for max current
            *duty = (unsigned short) calc;
            // *duty = duty_pre_dist[signal_index];

            if (signal_index > INDEX_TO_FALLING)
                gen_signal_state = SIGNAL_FALLING;

            break;

        case SIGNAL_FALLING:
            *duty = (unsigned short) calc;
            // *duty = duty_pre_dist[signal_index];

            if (signal_index > INDEX_TO_REVERT)
            {
                gen_signal_state = SIGNAL_REVERT;
                *duty = 0;
            }
            break;

        case SIGNAL_REVERT:

            break;
                        
        }                    
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        resp = SIGNAL_FINISH;

    return resp;
}


void GenSignalPreDistortedReset (void)
{
    p_current_ref = duty_pre_dist;
    gen_signal_state = SIGNAL_RISING;
}


unsigned short last_peak_current = 0;
unsigned short last_current_filtered = 0;
unsigned short current_filtered = 0;
ma8_u16_data_obj_t ma8_filter;
gen_signal_e GenSignalSinus2 (unsigned short i_sample, unsigned short peak_current, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;
    unsigned int calc = 0;

    //Adelanto la senial de tension-corriente
    if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
        //loop de corriente, es el inicial? o ya lo vengo trabanjando
        if (!last_peak_current)
        {
            MA8_U16Circular_Reset (&ma8_filter);

            //loop inicial, empiezo con el 12.5% de la corriente pico seteada
            calc = peak_current;
            calc = calc >> 3;
            last_peak_current = (unsigned short) calc;

            calc = *p_current_ref * last_peak_current;
            calc = calc >> 12;
        }
        else
        {
            //loop de ciclos consecutivos
            calc = *p_current_ref * last_peak_current;
            calc = calc >> 12;
        }

        switch (gen_signal_state)
        {
        case SIGNAL_RISING:
            // filter to check max current            
            current_filtered = MA8_U16Circular(&ma8_filter, i_sample);
            if (last_current_filtered < current_filtered)
                last_current_filtered = current_filtered;
            
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_MIDDLE)
                gen_signal_state = SIGNAL_MIDDLE;

            break;

        case SIGNAL_MIDDLE:
            // filter to check max current
            current_filtered = MA8_U16Circular(&ma8_filter, i_sample);
            if (last_current_filtered < current_filtered)
                last_current_filtered = current_filtered;
            
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_FALLING)
                gen_signal_state = SIGNAL_FALLING;

            break;

        case SIGNAL_FALLING:
            // filter to check max current
            current_filtered = MA8_U16Circular(&ma8_filter, i_sample);
            if (last_current_filtered < current_filtered)
                last_current_filtered = current_filtered;
            
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_REVERT)
            {
                if (last_current_filtered < peak_current)
                    last_peak_current++;
                else if (last_current_filtered > peak_current)
                    last_peak_current--;

                // printf("ref: %d filtered: %d peak: %d\n",
                //        current_ref,
                //        current_filtered,
                //        last_peak_current);
                
                gen_signal_state = SIGNAL_REVERT;
                *duty = 0;
            }
            break;

        case SIGNAL_REVERT:

            break;
                        
        }                    
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        resp = SIGNAL_FINISH;

    return resp;
}


gen_signal_e GenSignalSinus (unsigned short i_sample, unsigned short peak_current, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;
    unsigned int calc = 0;

    //Adelanto la senial de tension-corriente
    if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
        //loop de corriente, es el inicial? o ya lo vengo trabanjando
        if (!last_peak_current)
        {
            MA8_U16Circular_Reset (&ma8_filter);

            //loop inicial, empiezo con el 12.5% de la corriente pico seteada
            calc = peak_current;
            calc = calc >> 3;
            last_peak_current = (unsigned short) calc;

            calc = *p_current_ref * last_peak_current;
            calc = calc >> 12;
        }
        else
        {
            //loop de ciclos consecutivos
            calc = *p_current_ref * last_peak_current;
            calc = calc >> 12;
        }

        switch (gen_signal_state)
        {
        case SIGNAL_RISING:
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_MIDDLE)
                gen_signal_state = SIGNAL_MIDDLE;

            break;

        case SIGNAL_MIDDLE:
            // now check for max current
            current_filtered = MA8_U16Circular(&ma8_filter, i_sample);
            *duty = (unsigned short) calc;
            // printf("i_sample: %d current filtered: %d\n", i_sample, current_filtered);

            if (signal_index > SINUS_INDEX_TO_FALLING)
            {
                if (current_filtered < peak_current)
                    last_peak_current++;
                else if (current_filtered > peak_current)
                    last_peak_current--;
                         

                // printf("ref: %d filtered: %d peak: %d\n",
                //        current_ref,
                //        current_filtered,
                //        last_peak_current);
                
                gen_signal_state = SIGNAL_FALLING;
            }
            break;

        case SIGNAL_FALLING:
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_REVERT)
            {
                gen_signal_state = SIGNAL_REVERT;
                *duty = 0;
            }
            break;

        case SIGNAL_REVERT:

            break;
                        
        }                    
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        resp = SIGNAL_FINISH;

    return resp;
}


void GenSignalSinusReset (void)
{
    last_current_filtered = 0;
    p_current_ref = sin_half_cycle;
    gen_signal_state = SIGNAL_RISING;
}


void GenSignalSinusResetCntrs (void)
{
    last_peak_current = 0;
    GenSignalSinusReset();
}


gen_signal_e GenSignalTriang (unsigned short i_sample, unsigned short peak_current, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;
    unsigned int calc = 0;

    //Adelanto la senial de tension-corriente
    if (p_current_ref < &triang_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - triang_half_cycle);
                    
        //loop de corriente, es el inicial? o ya lo vengo trabanjando
        if (!last_peak_current)
        {
            MA8_U16Circular_Reset (&ma8_filter);

            //loop inicial, empiezo con el 12.5% de la corriente pico seteada
            calc = peak_current;
            calc = calc >> 3;
            last_peak_current = (unsigned short) calc;

            calc = *p_current_ref * last_peak_current;
            calc = calc >> 12;
        }
        else
        {
            //loop de ciclos consecutivos
            calc = *p_current_ref * last_peak_current;
            calc = calc >> 12;
        }

        switch (gen_signal_state)
        {
        case SIGNAL_RISING:
            // filter to check max current            
            current_filtered = MA8_U16Circular(&ma8_filter, i_sample);
            if (last_current_filtered < current_filtered)
                last_current_filtered = current_filtered;
            
            *duty = (unsigned short) calc;

            if (signal_index > TRIANG_INDEX_TO_CHECK_MAX_CURRENT)
            {
                if (last_current_filtered < peak_current)
                    last_peak_current++;
                else if (last_current_filtered > peak_current)
                    last_peak_current--;

                // printf("ref: %d filtered: %d peak: %d\n",
                //        current_ref,
                //        current_filtered,
                //        last_peak_current);
                
                gen_signal_state = SIGNAL_REVERT;
                *duty = 0;
            }
            break;

        case SIGNAL_REVERT:

            break;
                        
        }                    
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        resp = SIGNAL_FINISH;

    return resp;
}


void GenSignalTriangReset (void)
{
    last_current_filtered = 0;
    p_current_ref = triang_half_cycle;
    gen_signal_state = SIGNAL_RISING;
}


void GenSignalTriangResetCntrs (void)
{
    last_peak_current = 0;
    GenSignalTriangReset();
}

//--- end of file ---//
