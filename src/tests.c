//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS.C ###############################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "dsp.h"

#include <stdio.h>
#include <math.h>

// Types Constants and Macros --------------------------------------------------
typedef enum {
    SIGNAL_RISING = 0,
    SIGNAL_MIDDLE,
    SIGNAL_FALLING,
    SIGNAL_REVERT,
    SIGNAL_DO_NOTHING    
        
} signal_state_st;

#define SIZEOF_SIGNAL    240

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

#define KI_SIGNAL_PEAK_MULTIPLIER    2792

// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned short * p_current_ref;
signal_state_st signal_state = SIGNAL_RISING;
pid_data_obj_t current_pid;

unsigned short reference [SIZEOF_SIGNAL] = { 0 };
unsigned short duty_high_left [SIZEOF_SIGNAL] = { 0 };
unsigned short duty_high_right [SIZEOF_SIGNAL] = { 0 };
unsigned short vinput[SIZEOF_SIGNAL] = { 0 };
float vinput_applied[SIZEOF_SIGNAL] = { 0 };
float voutput[SIZEOF_SIGNAL] = { 0 };
unsigned short voutput_adc[SIZEOF_SIGNAL] = { 0 };

// Module Functions to Test ----------------------------------------------------
unsigned short CurrentLoop (unsigned short setpoint, unsigned short new_sample);
void CurrentLoop_Change_to_HighGain (void);
void CurrentLoop_Change_to_LowGain (void);

void Test_ACPOS (void);
void Test_ACNEG (void);

float Plant_Out (float);
void Plant_Step_Response (void);
void Plant_Step_Response_Duty (void);

unsigned short Adc12BitsConvertion (float );
void HIGH_LEFT (unsigned short duty);
void LOW_RIGHT (unsigned short duty);

void HIGH_RIGHT (unsigned short duty);
void LOW_LEFT (unsigned short duty);

//Auxiliares
void ShowVectorFloat (char *, float *, unsigned char);
void ShowVectorUShort (char *, unsigned short *, unsigned char);
void ShowVectorInt (char *, int *, unsigned char);

// Module Functions ------------------------------------------------------------
#define TEST_ON_ACPOS
// #define TEST_ON_ACNEG
int main (int argc, char *argv[])
{
    //pruebo un step de la planta
    // Plant_Step_Response();

    //pruebo un step de la planta pero con duty y vinput
    //la tension de entrada es tan alta que incluso con duty_max = 10000
    //tengo errores del 2%
    // Plant_Step_Response_Duty();
    
    

    //programa loop realimentado
    // PID_Small_Ki_Flush_Errors(&current_pid);
    PID_Flush_Errors(&current_pid);    
    CurrentLoop_Change_to_LowGain();

    p_current_ref = sin_half_cycle;

    float calc = 0.0;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        calc = sin (3.1415 * i / SIZEOF_SIGNAL);
        calc = 350 - calc * 311;
        vinput[i] = (unsigned short) calc;
    }

    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
#ifdef TEST_ON_ACPOS
        Test_ACPOS();
#endif
#ifdef TEST_ON_ACNEG
        Test_ACNEG();
#endif
    }

    // ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);
    // ShowVectorUShort("\nVector voltage input:\n", vinput, SIZEOF_SIGNAL);
#ifdef TEST_ON_ACPOS    
    ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, SIZEOF_SIGNAL);
#endif
#ifdef TEST_ON_ACNEG
    ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, SIZEOF_SIGNAL);
#endif

    ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);

    ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, SIZEOF_SIGNAL);

    int error [SIZEOF_SIGNAL] = { 0 };
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
        error[i] = reference[i] - voutput_adc[i];

    ShowVectorInt("\nPlant output error:\n", error, SIZEOF_SIGNAL);
    ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);

    return 0;
}


///////////////////////////////////////////////
// Cosas que tienen que ver con las seniales //
///////////////////////////////////////////////
#define DUTY_NONE    0
#define DUTY_100_PERCENT    2000
#define DUTY_ALWAYS    (DUTY_100_PERCENT + 1)
/////////////////////////////////////////////
// Cosas que tienen que ver con mediciones //
/////////////////////////////////////////////
#define INDEX_TO_MIDDLE    47
#define INDEX_TO_FALLING    156
#define INDEX_TO_REVERT    204
    

unsigned short I_Sense_Pos = 0;
unsigned short I_Sense_Neg = 0;
unsigned short last_output = 0;

unsigned short d = 0;
void Test_ACPOS (void)
{
    //Adelanto la seniales de corriente,
    // if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    // {
    //     unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
    //     //loop de corriente
    //     unsigned int calc = *p_current_ref * KI_SIGNAL_PEAK_MULTIPLIER;
    //     calc = calc >> 10;

    //     //TODO: modif
    //     reference[signal_index] = (unsigned short) calc;
    //     I_Sense_Pos = last_output;

    //     switch (signal_state)
    //     {
    //     case SIGNAL_RISING:
    //         d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
    //         HIGH_LEFT(d);

    //         if (signal_index > INDEX_TO_MIDDLE)
    //         {
    //             CurrentLoop_Change_to_HighGain();
    //             signal_state = SIGNAL_MIDDLE;
    //             // signal_state = SIGNAL_DO_NOTHING;
    //         }
    //         break;

    //     case SIGNAL_MIDDLE:
    //         d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
    //         HIGH_LEFT(d);

    //         if (signal_index > INDEX_TO_FALLING)
    //         {
    //             CurrentLoop_Change_to_LowGain();
    //             signal_state = SIGNAL_FALLING;
    //         }
    //         break;

    //     case SIGNAL_FALLING:
    //         d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
    //         HIGH_LEFT(d);

    //         if (signal_index > INDEX_TO_REVERT)
    //         {
    //             CurrentLoop_Change_to_LowGain();
    //             signal_state = SIGNAL_REVERT;
    //         }
    //         break;

    //     case SIGNAL_REVERT:
    //         d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
    //         HIGH_LEFT(d);

    //         // if (signal_index > 204)
    //         // {
    //         //     CurrentLoop_Change_to_LowGain();
    //         //     signal_state = SIGNAL_REVERT;
    //         // }
    //         break;

    //     case SIGNAL_DO_NOTHING:
    //         HIGH_LEFT(0);
    //         break;
            
    //     }
                    
    //     p_current_ref++;
    // }
    // else
    //     //termino de generar la senoidal, corto el mosfet
    //     LOW_RIGHT(DUTY_NONE);

    if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
        //loop de corriente
        unsigned int calc = *p_current_ref * KI_SIGNAL_PEAK_MULTIPLIER;
        calc = calc >> 12;

        //TODO: modif
        reference[signal_index] = (unsigned short) calc;
        I_Sense_Pos = last_output;
        
        switch (signal_state)
        {
        case SIGNAL_RISING:
            d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
            HIGH_LEFT(d);

            if (signal_index > INDEX_TO_MIDDLE)
            {
                CurrentLoop_Change_to_HighGain();
                signal_state = SIGNAL_MIDDLE;
            }
            break;

        case SIGNAL_MIDDLE:
            //TODO: modif
            HIGH_LEFT(DUTY_NONE);

            // d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
            // HIGH_LEFT(d);

            // if (signal_index > INDEX_TO_FALLING)
            // {
            //     CurrentLoop_Change_to_LowGain();
            //     signal_state = SIGNAL_FALLING;
            // }
            break;

        case SIGNAL_FALLING:
            //TODO: modif
            HIGH_LEFT(DUTY_NONE);

            // d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
            // HIGH_LEFT(d);

            // if (signal_index > INDEX_TO_REVERT)
            // {
            //     // CurrentLoop_Change_to_LowGain();
            //     signal_state = SIGNAL_REVERT;
            //     HIGH_LEFT(DUTY_NONE);
            // }
            break;

        case SIGNAL_REVERT:
            //TODO: modif
            HIGH_LEFT(DUTY_NONE);
            // d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
            // HIGH_LEFT(d);

            // if (signal_index > 204)
            // {
            //     CurrentLoop_Change_to_LowGain();
            //     signal_state = SIGNAL_REVERT;
            // }
            break;
                        
        }
                    
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        LOW_RIGHT(DUTY_NONE);
    
}


void Test_ACNEG (void)
{
    //Adelanto la senial de corriente,
    if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
        //loop de corriente
        unsigned int calc = *p_current_ref * KI_SIGNAL_PEAK_MULTIPLIER;
        calc = calc >> 12;

        reference[signal_index] = (unsigned short) calc;
        I_Sense_Neg = last_output;
        
        switch (signal_state)
        {
        case SIGNAL_RISING:
            d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
            HIGH_RIGHT(d);

            if (signal_index > INDEX_TO_MIDDLE)
            {
                CurrentLoop_Change_to_HighGain();
                signal_state = SIGNAL_MIDDLE;
            }
            break;

        case SIGNAL_MIDDLE:
            d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
            HIGH_RIGHT(d);

            if (signal_index > INDEX_TO_FALLING)
            {
                CurrentLoop_Change_to_LowGain();
                signal_state = SIGNAL_FALLING;
            }
            break;

        case SIGNAL_FALLING:
            d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
            HIGH_RIGHT(d);

            if (signal_index > INDEX_TO_REVERT)
            {
                // CurrentLoop_Change_to_LowGain();
                signal_state = SIGNAL_REVERT;
                HIGH_RIGHT(DUTY_NONE);
            }
            break;

        case SIGNAL_REVERT:
            // d = CurrentLoop ((unsigned short) calc, I_Sense_Neg);
            // HIGH_RIGHT(d);

            // if (signal_index > 204)
            // {
            //     CurrentLoop_Change_to_LowGain();
            //     signal_state = SIGNAL_REVERT;
            // }
            break;
                        
        }
                    
        p_current_ref++;
    }
    else
        //termino de generar la senoidal, corto el mosfet
        LOW_LEFT(DUTY_NONE);
    
}


unsigned short CurrentLoop (unsigned short setpoint, unsigned short new_sample)
{
    short d = 0;
    
    current_pid.setpoint = setpoint;
    current_pid.sample = new_sample;
    // d = PID_Small_Ki(&current_pid);
    d = PID(&current_pid);
                    
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
    current_pid.kp = 10;
    current_pid.ki = 3;
    current_pid.kd = 0;

    // current_pid.kp = 30;
    // current_pid.ki = 16;
    // current_pid.kd = 0;
    
    // current_pid.kp = 5;    
    // current_pid.ki = 320;    
    // current_pid.kd = 0;    
}


void CurrentLoop_Change_to_LowGain (void)
{
    current_pid.kp = 10;
    current_pid.ki = 3;
    current_pid.kd = 0;    
    // current_pid.kp = 5;
    // current_pid.ki = 32;
    // current_pid.kd = 16;    
}


unsigned char cntr_high_left = 0;
void HIGH_LEFT (unsigned short duty)
{
    float out = 0.0;
    float input = 0.0;
    
    duty_high_left[cntr_high_left] = duty;

    //aplico el nuevo duty a la planta
    input = vinput[cntr_high_left] * duty;
    input = input / DUTY_100_PERCENT;
    vinput_applied[cntr_high_left] = input;

    voutput[cntr_high_left] = Plant_Out(input);
    voutput_adc[cntr_high_left] = Adc12BitsConvertion(voutput[cntr_high_left]);
    last_output = voutput_adc[cntr_high_left];
    
    cntr_high_left++;
}

unsigned char cntr_high_right = 0;
void HIGH_RIGHT (unsigned short duty)
{
    float out = 0.0;
    float input = 0.0;
    
    duty_high_right[cntr_high_right] = duty;

    //aplico el nuevo duty a la planta
    input = vinput[cntr_high_right] * duty;
    input = input / DUTY_100_PERCENT;
    vinput_applied[cntr_high_right] = input;

    voutput[cntr_high_right] = Plant_Out(input);
    voutput_adc[cntr_high_right] = Adc12BitsConvertion(voutput[cntr_high_right]);
    last_output = voutput_adc[cntr_high_right];
    
    cntr_high_right++;
}


unsigned short duty_low_right [SIZEOF_SIGNAL] = { 0 };
unsigned char cntr_low_right = 0;
void LOW_RIGHT (unsigned short duty)
{
    duty_low_right[cntr_low_right] = duty;
    cntr_low_right++;
}


unsigned short duty_low_left [SIZEOF_SIGNAL] = { 0 };
unsigned char cntr_low_left = 0;
void LOW_LEFT (unsigned short duty)
{
    duty_low_left[cntr_low_left] = duty;
    cntr_low_left++;
}



//b[3]: [0.] b[2]: [0.02032562] b[1]: [0.0624813] b[0]: [0.01978482]
//a[3]: 1.0 a[2]: 0.011881459485754697 a[1]: 0.014346828516393684 a[0]: -0.9474935161284341
float output = 0.0;
float output_z1 = 0.0;
float output_z2 = 0.0;
float output_z3 = 0.0;
float input_z1 = 0.0;
float input_z2 = 0.0;
float input_z3 = 0.0;

unsigned char cntr_plant = 0;
float Plant_Out (float in)
{
    float output_b = 0.0;
    float output_a = 0.0;
    
    // output = 0. * input + 0.02032 * input_z1 + 0.06248 * input_z2 + 0.01978 * input_z3
    //     - 0.01188 * output_z1 - 0.01434 * output_z2 + 0.94749 * output_z3;

    if (cntr_plant > 2)
    {
        output_b = 0. * in + 0.02032 * input_z1 + 0.06248 * input_z2 + 0.01978 * input_z3;
        output_a = 0.01188 * output_z1 + 0.01434 * output_z2 - 0.94749 * output_z3;

        output = output_b - output_a;

        input_z3 = input_z2;
        input_z2 = input_z1;        
        input_z1 = in;

        output_z3 = output_z2;
        output_z2 = output_z1;        
        output_z1 = output;
        
    }
    else if (cntr_plant > 1)
    {
        output_b = 0. * in + 0.02032 * input_z1 + 0.06248 * input_z2;
        output_a = 0.01188 * output_z1 + 0.01434 * output_z2;
        output = output_b - output_a;

        input_z2 = input_z1;
        input_z1 = in;

        output_z2 = output_z1;
        output_z1 = output;
    }
    else if (cntr_plant > 0)
    {
        output_b = 0. * in + 0.02032 * input_z1;
        output_a = 0.01188 * output_z1;
        output = output_b - output_a;

        input_z2 = input_z1;
        input_z1 = in;

        output_z2 = output_z1;
        output_z1 = output;
    }
    else
    {
        output = 0. * in;
        
        input_z1 = in;

        output_z1 = output;
    }

    cntr_plant++;

    return output;
}


void Plant_Step_Response (void)
{
    printf("\nPlant Step Response\n");
    
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        vinput[i] = 1;
    }

    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        voutput[i] = Plant_Out(vinput[i]);
    }
    

    ShowVectorUShort("\nVector voltage input:\n", vinput, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);

}


void Plant_Step_Response_Duty (void)
{
    printf("\nPlant Step Response with duty and vinput\n");

    unsigned short d = 28;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        vinput[i] = 350;
    }

    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        vinput_applied[i] = vinput[i] * d;
        vinput_applied[i] = vinput_applied[i] / DUTY_100_PERCENT;
        voutput[i] = Plant_Out(vinput_applied[i]);
    }
    

    ShowVectorFloat("\nVector voltage input applied:\n", vinput_applied, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);
    
    unsigned short adc_out [SIZEOF_SIGNAL] = { 0 };
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
        adc_out[i] = Adc12BitsConvertion(voutput[i]);

    ShowVectorUShort("\nVector plant output ADC:\n", adc_out, SIZEOF_SIGNAL);
    
}


unsigned short Adc12BitsConvertion (float sample)
{
    if (sample > 0.0001)
    {
        sample = sample / 3.3;
        sample = sample * 4095;
        
        if (sample > 4095)
            sample = 4095;
    }
    else
        sample = 0.0;

    return (unsigned short) sample;
    
}


void ShowVectorFloat (char * s_comment, float * f_vect, unsigned char size)
{
    printf(s_comment);
    for (unsigned char i = 0; i < size; i+=8)
        printf("index: %03d - %f %f %f %f %f %f %f %f\n",
               i,
               *(f_vect+i+0),
               *(f_vect+i+1),
               *(f_vect+i+2),
               *(f_vect+i+3),
               *(f_vect+i+4),
               *(f_vect+i+5),
               *(f_vect+i+6),
               *(f_vect+i+7));
    
}


void ShowVectorUShort (char * s_comment, unsigned short * int_vect, unsigned char size)
{
    printf(s_comment);
    for (unsigned char i = 0; i < size; i+=8)
        printf("index: %03d - %d %d %d %d %d %d %d %d\n",
               i,
               *(int_vect+i+0),
               *(int_vect+i+1),
               *(int_vect+i+2),
               *(int_vect+i+3),
               *(int_vect+i+4),
               *(int_vect+i+5),
               *(int_vect+i+6),
               *(int_vect+i+7));
    
}


void ShowVectorInt (char * s_comment, int * int_vect, unsigned char size)
{
    printf(s_comment);
    for (unsigned char i = 0; i < size; i+=8)
        printf("index: %03d - %d %d %d %d %d %d %d %d\n",
               i,
               *(int_vect+i+0),
               *(int_vect+i+1),
               *(int_vect+i+2),
               *(int_vect+i+3),
               *(int_vect+i+4),
               *(int_vect+i+5),
               *(int_vect+i+6),
               *(int_vect+i+7));
    
}

//--- end of file ---//


