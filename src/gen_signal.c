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
#define INDEX_TO_REVERT    (240 - 10)    //230

// #define INDEX_TO_MIDDLE    (60 - 1)
// #define INDEX_TO_FALLING   (180 - 1)
// #define INDEX_TO_REVERT    (240 - 10)    //230

#define SINUS_INDEX_TO_MIDDLE    (116 - 1)
#define SINUS_INDEX_TO_FALLING   (124 - 1)
// #define SINUS_INDEX_TO_REVERT    150    //modif 26-12-2020, hago el corte mas rapido
#define SINUS_INDEX_TO_REVERT    (240 - 30)    //modif 16-02-2021
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


short sin_full_cycle [2*SIZEOF_SIGNAL] = {53,107,160,214,267,321,374,428,481,534,
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
                                          481,428,374,321,267,214,160,107,53,0,

                                          -53,-107,-160,-214,-267,-321,-374,-428,-481,-534,
                                          -587,-640,-693,-746,-798,-851,-903,-955,-1007,-1059,
                                          -1111,-1163,-1214,-1265,-1316,-1366,-1417,-1467,-1517,-1567,
                                          -1616,-1665,-1714,-1762,-1811,-1859,-1906,-1953,-2000,-2047,
                                          -2093,-2139,-2185,-2230,-2275,-2319,-2363,-2406,-2450,-2492,
                                          -2535,-2577,-2618,-2659,-2700,-2740,-2779,-2818,-2857,-2895,
                                          -2933,-2970,-3007,-3043,-3078,-3113,-3148,-3182,-3215,-3248,
                                          -3281,-3312,-3344,-3374,-3404,-3434,-3463,-3491,-3519,-3546,
                                          -3572,-3598,-3624,-3648,-3672,-3696,-3718,-3740,-3762,-3783,
                                          -3803,-3823,-3841,-3860,-3877,-3894,-3910,-3926,-3941,-3955,
                                          -3969,-3981,-3994,-4005,-4016,-4026,-4035,-4044,-4052,-4059,
                                          -4066,-4072,-4077,-4082,-4086,-4089,-4091,-4093,-4094,-4095,
                                          -4094,-4093,-4091,-4089,-4086,-4082,-4077,-4072,-4066,-4059,
                                          -4052,-4044,-4035,-4026,-4016,-4005,-3994,-3981,-3969,-3955,
                                          -3941,-3926,-3910,-3894,-3877,-3860,-3841,-3823,-3803,-3783,
                                          -3762,-3740,-3718,-3696,-3672,-3648,-3624,-3598,-3572,-3546,
                                          -3519,-3491,-3463,-3434,-3404,-3374,-3344,-3312,-3281,-3248,
                                          -3215,-3182,-3148,-3113,-3078,-3043,-3007,-2970,-2933,-2895,
                                          -2857,-2818,-2779,-2740,-2700,-2659,-2618,-2577,-2535,-2492,
                                          -2450,-2406,-2363,-2319,-2275,-2230,-2185,-2139,-2093,-2047,
                                          -2000,-1953,-1906,-1859,-1811,-1762,-1714,-1665,-1616,-1567,
                                          -1517,-1467,-1417,-1366,-1316,-1265,-1214,-1163,-1111,-1059,
                                          -1007,-955,-903,-851,-798,-746,-693,-640,-587,-534,
                                          -481,-428,-374,-321,-267,-214,-160,-107,-53,0};


unsigned short sin_sqrd_half_cycle [SIZEOF_SIGNAL] = {0,2,6,11,17,25,34,44,56,69,
                                                      84,100,117,135,155,177,199,223,248,274,
                                                      301,330,360,391,423,456,490,525,562,599,
                                                      638,677,717,758,801,844,887,932,977,1023,
                                                      1070,1117,1166,1214,1263,1313,1364,1414,1465,1517,
                                                      1569,1621,1674,1727,1780,1833,1886,1940,1993,2047,
                                                      2101,2154,2208,2261,2314,2367,2420,2473,2525,2577,
                                                      2629,2680,2730,2781,2831,2880,2928,2977,3024,3071,
                                                      3117,3162,3207,3250,3293,3336,3377,3417,3456,3495,
                                                      3532,3569,3604,3638,3671,3703,3734,3764,3793,3820,
                                                      3846,3871,3895,3917,3939,3959,3977,3994,4010,4025,
                                                      4038,4050,4060,4069,4077,4083,4088,4092,4094,4095,
                                                      4094,4092,4088,4083,4077,4069,4060,4050,4038,4025,
                                                      4010,3994,3977,3959,3939,3917,3895,3871,3846,3820,
                                                      3793,3764,3734,3703,3671,3638,3604,3569,3532,3495,
                                                      3456,3417,3377,3336,3293,3250,3207,3162,3117,3071,
                                                      3024,2977,2928,2880,2831,2781,2730,2680,2629,2577,
                                                      2525,2473,2420,2367,2314,2261,2208,2154,2101,2047,
                                                      1993,1940,1886,1833,1780,1727,1674,1621,1569,1517,
                                                      1465,1414,1364,1313,1263,1214,1166,1117,1070,1023,
                                                      977,932,887,844,801,758,717,677,638,599,
                                                      562,525,490,456,423,391,360,330,301,274,
                                                      248,223,199,177,155,135,117,100,84,69,
                                                      56,44,34,25,17,11,6,2,0,0};

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
    SIGNAL_REVERT,
    SIGNAL_DCM,
    SIGNAL_CCM,
    SIGNAL_CCM_REVERT
        
} signal_state_e;


// Externals -------------------------------------------------------------------
#ifdef WITH_FEW_CYCLES_OF_50HZ
extern unsigned short d_dump [];
extern unsigned short i_dump [];
extern unsigned short r_dump [];
#endif


// Globals ---------------------------------------------------------------------
// #define CURRENT_LOOP_PR_CONTROLLER
#define CURRENT_LOOP_PI_CONTROLLER
// #define CURRENT_LOOP_ZP_CONTROLLER
unsigned short * p_current_ref;
short * p_current_ref_bipolar;
#ifdef CURRENT_LOOP_PI_CONTROLLER
pid_data_obj_t current_pid;
#endif
#ifdef CURRENT_LOOP_PR_CONTROLLER
pr_data_obj_t current_pr;
#endif
#ifdef CURRENT_LOOP_ZP_CONTROLLER
short last_e = 0;
short last_d = 0;
#endif
signal_state_e gen_signal_state = SIGNAL_RISING;


// Module Private Functions ----------------------------------------------------
short CurrentLoop (short, short);
// void CurrentLoop_SetLastD (unsigned short d);
void CurrentLoop_Change_to_LowGain (void);
void CurrentLoop_Change_to_HighGain (void);
void CurrentLoop_Change_to_RevertGain (void);
unsigned short Distance (unsigned short a, unsigned short b);


// Module Functions ------------------------------------------------------------
// #define FUZZY_BY_SAMPLES
#define FUZZY_BY_MODE

#ifdef FUZZY_BY_SAMPLES
#define SAMPLE_BY_SAMPLE
// #define UNDERSAMPLING
#endif

#ifdef UNDERSAMPLING
unsigned char undersampling = 0;
short last_duty = 0;
#endif
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
#ifdef UNDERSAMPLING
            if (!undersampling)
            {
                *duty = CurrentLoop ((short) calc, i_sample);
                last_duty = *duty;
                undersampling = 9;
            }
            else
            {
                undersampling--;
                *duty = last_duty;
            }

            if (signal_index > INDEX_TO_FALLING)
            {
                gen_signal_state = SIGNAL_REVERT;
                *duty = 0;
                last_duty = 0;
            }
            
#endif
#ifdef SAMPLE_BY_SAMPLE
            *duty = CurrentLoop ((short) calc, i_sample);

            if (signal_index > INDEX_TO_MIDDLE)
            {
                CurrentLoop_Change_to_RevertGain();
                gen_signal_state = SIGNAL_FALLING;
            }
#endif
            break;

        case SIGNAL_MIDDLE:
            *duty = CurrentLoop ((short) calc, i_sample);

            if (signal_index > INDEX_TO_FALLING)
            {
                CurrentLoop_Change_to_LowGain();
                gen_signal_state = SIGNAL_FALLING;
            }
            break;

        case SIGNAL_FALLING:
            *duty = CurrentLoop ((short) calc, i_sample);

            if (signal_index > INDEX_TO_REVERT)
            {
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

#ifdef FUZZY_BY_MODE
        case SIGNAL_DCM:
            //cuando la corriente supera la seteada supongo CCM despues de algunas muestras
            // y ademas no vuelvo
            // if ((signal_index > 20) && (i_sample > calc))
            // {
            //     //supongo CCM
            //     gen_signal_state = SIGNAL_CCM;
            //     CurrentLoop_Change_to_LowGain();
            // }

            //en la ultima parte de la senial aumento la ganancia independiente de donde este
            // if (signal_index > 210)
            // {
            //     gen_signal_state = SIGNAL_CCM;
            //     CurrentLoop_Change_to_LowGain();
            // }

            *duty = CurrentLoop ((short) calc, i_sample);
            if ((signal_index > 20) &&
                (i_sample > 600))
            {
                gen_signal_state = SIGNAL_CCM;
                CurrentLoop_Change_to_LowGain();
            }
            break;

        case SIGNAL_CCM:
            *duty = CurrentLoop ((short) calc, i_sample);

            if (signal_index > 200)
            {
                gen_signal_state = SIGNAL_CCM_REVERT;
                CurrentLoop_Change_to_HighGain();
                *duty = 0;
            }
            break;

        case SIGNAL_CCM_REVERT:
            *duty = CurrentLoop ((short) calc, i_sample);
            
            if (signal_index > 234)
            {
                *duty = 0;
            }
            break;
#endif
            
        }                    
        p_current_ref++;

#ifdef WITH_FEW_CYCLES_OF_50HZ
        i_dump[signal_index] = i_sample;
        d_dump[signal_index] = *duty;
        r_dump[signal_index] = (unsigned short) calc;
#endif

    }
    else
        //termino de generar la senoidal, corto el mosfet
        resp = SIGNAL_FINISH;

    return resp;
}


unsigned char volt_index = 0;
unsigned short last_peak_current = 0;
unsigned short last_peak_multiplier = 0;
gen_signal_e GenSignalVoltage (unsigned short v_sample, unsigned short i_sample, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;

    if (volt_index < 240)
    {
        //calculo la referencia de tension que quiero
        unsigned int calc = v_sample * last_peak_multiplier;
        calc = calc >> 12;

        if (calc > 2000)
            calc = 2000;
        
        *duty = (short) calc;

#ifdef WITH_FEW_CYCLES_OF_50HZ
        i_dump[volt_index] = i_sample;
        d_dump[volt_index] = *duty;
        r_dump[volt_index] = v_sample;
#endif

        //si en algun momento del ciclo me paso de la corriente maxima trato de bajar
        // if (i_sample > 1200)
        // {
        //     if (last_peak_multiplier > 10)
        //         last_peak_multiplier -= 10;
        //     else
        //         last_peak_multiplier--;
        // }

        if (i_sample > last_peak_current)
            last_peak_current = i_sample;

        volt_index++;
    }
    else
    {
        *duty = 0;
        resp = SIGNAL_FINISH;
    }

    return resp;
}


pid_data_obj_t voltage_pid;
void GenSignalVoltageReset (void)
{
    short new_d = 0;
    //hago un update del multiplier en funcion de la corriente pico
    voltage_pid.setpoint = 900;
    voltage_pid.sample = last_peak_current;
    new_d = PI(&voltage_pid);

    if (new_d < 0)
        new_d = 0;

    if (new_d > 4095)
        new_d = 4095;

    last_peak_multiplier = new_d;
    last_peak_current = 0;
    volt_index = 0;
}


void GenSignalVoltageInit (void)
{
    //armo un pid para controlar la corriente pico a traves del multiplicador
    voltage_pid.kp = 0;
    voltage_pid.ki = 11;    //
    voltage_pid.kd = 0;
    PID_Flush_Only_Errors(&voltage_pid);
    
    GenSignalVoltageReset();
}


void GenSignalBipolar (short i_sample, short peak_current, short * duty)
{
    //Adelanto la seniales de corriente,
    if (p_current_ref_bipolar < &sin_full_cycle[(2*SIZEOF_SIGNAL - 1)])
    {
        unsigned short signal_index = (unsigned short) (p_current_ref_bipolar - sin_full_cycle);

        //loop de corriente
        int calc = *p_current_ref_bipolar * peak_current;
        calc = calc >> 12;

        // printf("i: %d peak: %d ref: %d ", i_sample, peak_current, (short) calc);
        
        *duty = CurrentLoop ((short) calc, i_sample);

        p_current_ref_bipolar++;

        // printf("duty: %d index: %d\n", *duty, signal_index);
    }
    
}


void GenSignalResetBipolar (void)
{
    p_current_ref_bipolar = sin_full_cycle;
}


void GenSignalReset (void)
{
    p_current_ref = sin_half_cycle;
#ifdef CURRENT_LOOP_PI_CONTROLLER
    PID_Flush_Errors(&current_pid);
    CurrentLoop_Change_to_HighGain();
#endif

#ifdef CURRENT_LOOP_PR_CONTROLLER
    PR_Flush_Errors(&current_pr);
    CurrentLoop_Change_to_HighGain();
#endif

#ifdef CURRENT_LOOP_ZP_CONTROLLER
    last_d = 0;
    last_e = 0;
#endif

#ifdef FUZZY_BY_SAMPLES
    gen_signal_state = SIGNAL_RISING;
#ifdef UNDERSAMPLING
    undersampling = 0;    
#endif
#endif
    
#ifdef FUZZY_BY_MODE
    gen_signal_state = SIGNAL_DCM;
#endif
    
}


void GenSignalControlInit(void)
{
#ifdef CURRENT_LOOP_PR_CONTROLLER
    PR_Flush_Errors(&current_pr);
    CurrentLoop_Change_to_LowGain();
#endif
}


short CurrentLoop (short setpoint, short new_sample)
{
#ifdef CURRENT_LOOP_PR_CONTROLLER
    short d = 0;
    
    current_pr.setpoint = setpoint;
    current_pr.sample = new_sample;
    d = PR(&current_pr);
    
    if (d > DUTY_100_PERCENT)
    {
        d = DUTY_100_PERCENT;
        current_pr.d_z1 = DUTY_100_PERCENT;
        current_pr.d_z2 = DUTY_100_PERCENT;            
    }
    else if (d < -DUTY_100_PERCENT)
    {
        d = -DUTY_100_PERCENT;
        current_pr.d_z1 = -DUTY_100_PERCENT;
        current_pr.d_z2 = -DUTY_100_PERCENT;            
    }
    else
    {
        //do nothing here
    }
    printf("set: %d i: %d d: %d\n", setpoint, new_sample, d);
#endif

#ifdef CURRENT_LOOP_PI_CONTROLLER
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
#endif

#ifdef CURRENT_LOOP_ZP_CONTROLLER
    short d = 0;
    short e = setpoint - new_sample;
    int e_z1 = last_e * 383;
    e_z1 >>= 7;

    int d_z1 = last_d * 126;
    d_z1 >>= 7;
    
    d = e * 3 - e_z1 + d_z1;
    
    if (d > 0)
    {
        if (d > DUTY_100_PERCENT)
            d = DUTY_100_PERCENT;
    }
    else
    {
        d = DUTY_NONE;
    }
    
    printf("set: %d i: %d l_d: %d l_e: %d e_z1: %d d_z1: %d d: %d\n",
           setpoint,
           new_sample,
           last_d,
           last_e,
           e_z1,
           d_z1,
           d);    

    last_e = e;
    last_d = d;


#endif
    
    return d;
}


// void CurrentLoop_SetLastD (unsigned short d)
// {
//     current_pid.last_d = d;
// }


void CurrentLoop_Change_to_HighGain (void)
{
#ifdef CURRENT_LOOP_PI_CONTROLLER
    //vsense directo de Rsense
    // current_pid.kp = 128;
    // current_pid.ki = 33;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    // current_pid.kp = 29;
    // current_pid.ki = 7;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    // current_pid.kp = 1;
    // current_pid.ki = 45;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    current_pid.kp = 1;
    current_pid.ki = 55;    //para undersampling 4
    // current_pid.ki = 11;    //para sample_by_sample
    current_pid.kd = 0;

    
    PID_Flush_Only_Errors(&current_pid);
#endif
    
#ifdef CURRENT_LOOP_PR_CONTROLLER
// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       , -0.19768152,  0.09769864]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
// )

    // current_pr.b0 = 13;
    // current_pr.b1 = -25;
    // current_pr.b2 = 12;

    // // current_pr.a0 = 128;
    // current_pr.a1 = -256;
    // current_pr.a2 = 128;

    // current_pr.b0 = 0.2;    //A = 35dB Kp = 0.2
    // current_pr.b1 = -0.27402; //oscila mas grande a menos freq
    // current_pr.b2 = 0.07405;
    // current_pr.a1 = -1.99774;
    // current_pr.a2 = 0.99791;

    // current_pr.b0 = 0.2;    //A = 30dB Kp = 0.2
    // current_pr.b1 = -0.3367; //oscila mas grande a menos freq
    // current_pr.b2 = 0.1368;
    // current_pr.a1 = -1.99774;
    // current_pr.a2 = 0.99791;

    current_pr.b0 = 0.2;    //A = 30dB Kp = 0.2
    current_pr.b1 = -0.32103; //oscila mas grande a menos freq
    current_pr.b2 = 0.12104;
    current_pr.a1 = -1.99734;
    current_pr.a2 = 0.99738;

    PR_Flush_Errors(&current_pr);
#endif
}


void CurrentLoop_Change_to_LowGain (void)
{
#ifdef CURRENT_LOOP_PI_CONTROLLER
    //vsense directo de Rsense    
    // current_pid.kp = 128;
    // current_pid.ki = 33;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    // current_pid.kp = 29;
    // current_pid.ki = 7;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    // current_pid.kp = 1;
    // current_pid.ki = 45;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    current_pid.kp = 0;
    current_pid.ki = 5;
    current_pid.kd = 0;
    
    PID_Flush_Only_Errors(&current_pid);
#endif
    
#ifdef CURRENT_LOOP_PR_CONTROLLER
// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       , -0.19768152,  0.09769864]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
// )


    // current_pr.b0 = 104;    //oscila
    // current_pr.b1 = -200;
    // current_pr.b2 = 96;

    // current_pr.b0 = 78;
    // current_pr.b1 = -150;
    // current_pr.b2 = 72;
    
    // current_pr.b0 = 26;
    // current_pr.b1 = -50;
    // current_pr.b2 = 24;

    // current_pr.b0 = 13;
    // current_pr.b1 = -25;
    // current_pr.b2 = 12;

    // current_pr.b0 = 128;    //oscila
    // current_pr.b1 = -255;
    // current_pr.b2 = 127;

    // current_pr.a0 = 128;
    // current_pr.a1 = -256;
    // current_pr.a2 = 128;

// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       , -0.15793079,  0.05794791]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
    // current_pr.b0 = 0.1;
    // current_pr.b1 = -0.15793;
    // current_pr.b2 = 0.05794;

// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       , -0.11608792,  0.01610503]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
    // current_pr.b0 = 0.1;
    // current_pr.b1 = -0.11608;
    // current_pr.b2 = 0.01610;

// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       , -0.07424504, -0.02573784]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
    // current_pr.b0 = 0.1;    //oscila algo A = 35dB Kp = 0.1
    // current_pr.b1 = -0.07424;
    // current_pr.b2 = -0.02573;
    // current_pr.a1 = -1.99774;
    // current_pr.a2 = 0.99791;

// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.05      ,  0.02564179, -0.07563323]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
    // current_pr.b0 = 0.05;    //A = 35dB Kp = 0.05    
    // current_pr.b1 = 0.02564; //oscila mas grande a menos freq
    // current_pr.b2 = -0.07563;
    // current_pr.a1 = -1.99774;
    // current_pr.a2 = 0.99791;

// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.2       , -0.27401871,  0.07405294]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
    // current_pr.b0 = 0.2;    //A = 35dB Kp = 0.2
    // current_pr.b1 = -0.27402; //oscila mas grande a menos freq
    // current_pr.b2 = 0.07405;
    // current_pr.a1 = -1.99774;
    // current_pr.a2 = 0.99791;

    // current_pr.b0 = 0.2;    //A = 30dB Kp = 0.2
    // current_pr.b1 = -0.3367; //oscila mas grande a menos freq
    // current_pr.b2 = 0.1368;
    // current_pr.a1 = -1.99774;
    // current_pr.a2 = 0.99791;

    // current_pr.b0 = 0.2;    //A = 30dB Kp = 0.2
    // current_pr.b1 = -0.321; //oscila mas grande a menos freq
    // current_pr.b2 = 0.121;
    // current_pr.a1 = -1.99721;
    // current_pr.a2 = 0.99738;

    current_pr.b0 = 0.2;    //A = 30dB Kp = 0.2
    current_pr.b1 = -0.32103; //oscila mas grande a menos freq
    current_pr.b2 = 0.12104;
    current_pr.a1 = -1.99734;
    current_pr.a2 = 0.99738;

// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       ,  0.0094407 , -0.10942358]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
    // current_pr.b0 = 0.1;    //oscila mucho
    // current_pr.b1 = 0.00944;
    // current_pr.b2 = -0.10942;
    // current_pr.a1 = -1.99774;
    // current_pr.a2 = 0.99791;
    // hasta aca fbw = 4Hz
    
// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       , -0.13708104,  0.03709816]),
// array([ 1.        , -1.9987821 ,  0.99895335]),
// dt: 4.1666666666666665e-05

    // current_pr.b0 = 0.1;    //
    // current_pr.b1 = -0.13708;
    // current_pr.b2 = 0.03709;

    // current_pr.a1 = -1.99878;
    // current_pr.a2 = 0.99895;
    //fbw = 2Hz


    
    

    PR_Flush_Only_Errors(&current_pr);
#endif
    
}


void CurrentLoop_Change_to_RevertGain (void)
{
#ifdef CURRENT_LOOP_PI_CONTROLLER
    //vsense directo de Rsense    
    // current_pid.kp = 128;
    // current_pid.ki = 33;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    // current_pid.kp = 29;
    // current_pid.ki = 7;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    // current_pid.kp = 1;
    // current_pid.ki = 45;
    // current_pid.kd = 0;

    //vsense con opamp 4.4
    current_pid.kp = 0;
    current_pid.ki = 2;
    current_pid.kd = 0;
    
    PID_Flush_Only_Errors(&current_pid);
#endif

#ifdef CURRENT_LOOP_PR_CONTROLLER
// Controlador PR Digital Zoh:
// TransferFunctionDiscrete(
// array([ 0.1       , -0.19768152,  0.09769864]),
// array([ 1.        , -1.99773663,  0.9979078 ]),
// dt: 4.1666666666666665e-05
// )

    current_pr.b0 = 13;
    current_pr.b1 = -25;
    current_pr.b2 = 12;

    // current_pr.a0 = 128;
    current_pr.a1 = -256;
    current_pr.a2 = 128;

    PR_Flush_Errors(&current_pr);
#endif
    
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


unsigned short current_i_peak_value = 0;
// unsigned short last_peak_current = 0;
unsigned char current_i_peak_cntr = 0;
unsigned char last_peak_index = 0;    //por ahora solo adelanto
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
            //loop inicial, empiezo con el 12.5% de la corriente pico seteada
            calc = peak_current;
            calc = calc >> 3;
            last_peak_current = (unsigned short) calc;
        }
        
        //loop de ciclos consecutivos, por ahora solo adelanto
        if ((signal_index + last_peak_index) < (SIZEOF_SIGNAL - 1))
        {
            calc = *(p_current_ref + last_peak_index) * last_peak_current;
            calc = calc >> 12;
        }
        else
        {
            calc = *p_current_ref * last_peak_current;
            calc = calc >> 12;
        }

        switch (gen_signal_state)
        {
        case SIGNAL_RISING:
            // check max current
            if (current_i_peak_value < i_sample)
            {
                current_i_peak_value = i_sample;
                current_i_peak_cntr = signal_index;
            }
            
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_MIDDLE)
                gen_signal_state = SIGNAL_MIDDLE;

            break;

        case SIGNAL_MIDDLE:
            // check max current
            if (current_i_peak_value < i_sample)
            {
                current_i_peak_value = i_sample;
                current_i_peak_cntr = signal_index;
            }
            
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_FALLING)
                gen_signal_state = SIGNAL_FALLING;

            break;

        case SIGNAL_FALLING:
            // check max current
            if (current_i_peak_value < i_sample)
            {
                current_i_peak_value = i_sample;
                current_i_peak_cntr = signal_index;
            }
            
            *duty = (unsigned short) calc;

            if (signal_index > SINUS_INDEX_TO_REVERT)
            {
                //ajusto proximo pico de corriente
                if (current_i_peak_value < peak_current)
                    last_peak_current++;
                else if (current_i_peak_value > peak_current)
                    last_peak_current--;

                //ajusto proximo indice del pico, por ahora solo adelanto
                if (current_i_peak_cntr > 119)
                {
                    if (last_peak_index < 5)
                        last_peak_index++;
                }
                else if (current_i_peak_cntr < 119)
                {
                    if (last_peak_index)
                        last_peak_index--;
                }

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


// #define SINUS_NORMAL
// #define SINUS_SQUARED
#define SINUS_FOLLOWS_REF
#if (defined SINUS_NORMAL) || (defined SINUS_SQUARED) || (defined SINUS_FOLLOWS_REF)
unsigned short last_current_filtered = 0;
unsigned short current_filtered = 0;
ma8_u16_data_obj_t ma8_filter;
#endif

#ifdef SINUS_NORMAL
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

            //loop inicial, empiezo con el 80% de la corriente pico seteada
            calc = peak_current;
            calc = calc * 95;
            calc = calc / 100;            
            last_peak_current = (unsigned short) calc;

            // //loop inicial, empiezo con el 12.5% de la corriente pico seteada
            // calc = peak_current;
            // calc = calc >> 3;
            // last_peak_current = (unsigned short) calc;
            
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

#endif    //SINUS_NORMAL

#ifdef SINUS_SQUARED
gen_signal_e GenSignalSinus (unsigned short i_sample, unsigned short peak_current, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;
    unsigned int calc = 0;

    //Adelanto la senial de tension-corriente
    if (p_current_ref < &sin_sqrd_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_sqrd_half_cycle);
                    
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
    p_current_ref = sin_sqrd_half_cycle;
    gen_signal_state = SIGNAL_RISING;
}

#endif    //SINUS_SQUARED


#ifdef SINUS_FOLLOWS_REF
unsigned short duty_saved [SIZEOF_SIGNAL] = { 0 };
short last_filter = 0;
gen_signal_e GenSignalSinus (unsigned short i_sample, unsigned short peak_current, short * duty)
{
    gen_signal_e resp = SIGNAL_RUNNING;
    unsigned int calc = 0;

    //Adelanto la senial de tension-corriente
    if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);

        // calculo la referencia de corriente para este punto
        calc = *p_current_ref * peak_current;
        calc = calc >> 12;

        // printf("i: %d calc: %d index: %d ", i_sample, calc, signal_index);
        // reviso si estoy por debajo o arriba de la corriente requerida
        if (i_sample < calc)
        {
            if (signal_index)    //reviso que no sea el primer punto
            {
                if (duty_saved[signal_index] < DUTY_100_PERCENT)
                {
                    if (Distance(duty_saved[signal_index - 1], duty_saved[signal_index]) < 40)
                        duty_saved[signal_index] += 1;
                        
                }
                else
                    duty_saved[signal_index] = DUTY_100_PERCENT;
            }
        }
        else if (i_sample > calc)
        {
            if (duty_saved[signal_index] > DUTY_NONE)
            {
                if (signal_index)    //reviso que no sea el primer punto
                {
                    if (Distance(duty_saved[signal_index - 1], duty_saved[signal_index]) < 40)
                        duty_saved[signal_index] -= 1;

                }
                else
                    duty_saved[signal_index] -= 1;
            }
            else
                duty_saved[signal_index] = DUTY_NONE;
            
        }
        else
        {
            // do nothing in here
        }

        // if (signal_index)
        //     printf("duty: %d -> %d\n", duty_saved[signal_index - 1], duty_saved[signal_index]);
        // else
        //     printf("duty: %d\n", duty_saved[signal_index]);

        if (signal_index > SINUS_INDEX_TO_REVERT)
            duty_saved[signal_index] = 0;

        //aplico finalmente un filtro de muestras
        if (signal_index)
        {
            short b = 0;
            short a = 0;
            // b = 0.8
            b = duty_saved[signal_index] * 1;
            b = b / 10;
            // a = 0.2
            a = last_filter * 9;
            a = a / 10;

            *duty = b - a;
            
            last_filter = *duty;
        }
        else
            *duty = duty_saved[signal_index];
        
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        resp = SIGNAL_FINISH;

    return resp;
}

void GenSignalSinusReset (void)
{
    p_current_ref = sin_half_cycle;
    gen_signal_state = SIGNAL_RISING;
}


void GenSignalSinusDutySet (unsigned short d)
{
    for (unsigned short i = 0; i < SIZEOF_SIGNAL; i++)
        duty_saved[i] = d;
}

void GenSignalSinusResetCntrs (void)
{
    last_peak_current = 0;
    GenSignalSinusReset();
}


void GenSignalSinusApplyFilter (void)
{
    int calc = 0;
    for (int i = 0; i < (SIZEOF_SIGNAL - 4); i++)
    {
        calc = duty_saved[i] + duty_saved[i+1] + duty_saved[i+2] + duty_saved[i+3];
        calc >>= 2;
        duty_saved[i] = (unsigned short) calc;
    }
}


#endif    //SINUS_FOLLOWS_REF


void GenSignalSinus2Reset (void)
{
    current_i_peak_value = 0;
    current_i_peak_cntr = 0;
    p_current_ref = sin_half_cycle;
    gen_signal_state = SIGNAL_RISING;
}


void GenSignalSinus2ResetCntrs (void)
{
    last_peak_current = 0;
    last_peak_index = 0;
    GenSignalSinus2Reset();
}


// gen_signal_e GenSignalTriang (unsigned short i_sample, unsigned short peak_current, short * duty)
// {
//     gen_signal_e resp = SIGNAL_RUNNING;
//     unsigned int calc = 0;

//     //Adelanto la senial de tension-corriente
//     if (p_current_ref < &triang_half_cycle[(SIZEOF_SIGNAL - 1)])
//     {
//         unsigned char signal_index = (unsigned char) (p_current_ref - triang_half_cycle);
                    
//         //loop de corriente, es el inicial? o ya lo vengo trabanjando
//         if (!last_peak_current)
//         {
//             MA8_U16Circular_Reset (&ma8_filter);

//             //loop inicial, empiezo con el 12.5% de la corriente pico seteada
//             calc = peak_current;
//             calc = calc >> 3;
//             last_peak_current = (unsigned short) calc;

//             calc = *p_current_ref * last_peak_current;
//             calc = calc >> 12;
//         }
//         else
//         {
//             //loop de ciclos consecutivos
//             calc = *p_current_ref * last_peak_current;
//             calc = calc >> 12;
//         }

//         switch (gen_signal_state)
//         {
//         case SIGNAL_RISING:
//             // filter to check max current            
//             current_filtered = MA8_U16Circular(&ma8_filter, i_sample);
//             if (last_current_filtered < current_filtered)
//                 last_current_filtered = current_filtered;
            
//             *duty = (unsigned short) calc;

//             if (signal_index > TRIANG_INDEX_TO_CHECK_MAX_CURRENT)
//             {
//                 if (last_current_filtered < peak_current)
//                     last_peak_current++;
//                 else if (last_current_filtered > peak_current)
//                     last_peak_current--;

//                 // printf("ref: %d filtered: %d peak: %d\n",
//                 //        current_ref,
//                 //        current_filtered,
//                 //        last_peak_current);
                
//                 gen_signal_state = SIGNAL_REVERT;
//                 *duty = 0;
//             }
//             break;

//         case SIGNAL_REVERT:

//             break;
                        
//         }                    
//         p_current_ref++;
//     }
//     else
//         //termino de generar la senoidal, corto el mosfet
//         resp = SIGNAL_FINISH;

//     return resp;
// }


// void GenSignalTriangReset (void)
// {
//     last_current_filtered = 0;
//     p_current_ref = triang_half_cycle;
//     gen_signal_state = SIGNAL_RISING;
// }


// void GenSignalTriangResetCntrs (void)
// {
//     last_peak_current = 0;
//     GenSignalTriangReset();
// }


unsigned short Distance (unsigned short a, unsigned short b)
{
    if (a > b)
        return a - b;
    else
        return b - a;
    
}
//--- end of file ---//
