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
    SIGNAL_REVERT
        
} signal_state_st;

#define SIZEOF_SIGNAL    240

unsigned short sin_half_cycle [SIZEOF_SIGNAL] = {13,26,40,53,66,80,93,106,120,133,
                                                 146,160,173,186,199,212,225,238,251,264,
                                                 277,290,303,316,328,341,354,366,379,391,
                                                 403,416,428,440,452,464,476,488,499,511,
                                                 523,534,545,557,568,579,590,601,612,622,
                                                 633,643,654,664,674,684,694,704,713,723,
                                                 732,742,751,760,769,777,786,795,803,811,
                                                 819,827,835,843,850,857,865,872,879,885,
                                                 892,899,905,911,917,923,929,934,939,945,
                                                 950,955,959,964,968,972,976,980,984,988,
                                                 991,994,997,1000,1003,1005,1008,1010,1012,1014,
                                                 1015,1017,1018,1019,1020,1021,1022,1022,1022,1023,
                                                 1022,1022,1022,1021,1020,1019,1018,1017,1015,1014,
                                                 1012,1010,1008,1005,1003,1000,997,994,991,988,
                                                 984,980,976,972,968,964,959,955,950,945,
                                                 939,934,929,923,917,911,905,899,892,885,
                                                 879,872,865,857,850,843,835,827,819,811,
                                                 803,795,786,777,769,760,751,742,732,723,
                                                 713,704,694,684,674,664,654,643,633,622,
                                                 612,601,590,579,568,557,545,534,523,511,
                                                 499,488,476,464,452,440,428,416,403,391,
                                                 379,366,354,341,328,316,303,290,277,264,
                                                 251,238,225,212,199,186,173,160,146,133,
                                                 120,106,93,80,66,53,40,26,13,0};

// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned short * p_current_ref;
signal_state_st signal_state = SIGNAL_RISING;
pid_data_obj_t current_pid;

unsigned short reference [SIZEOF_SIGNAL] = { 0 };
unsigned short duty_high_left [SIZEOF_SIGNAL] = { 0 };
unsigned short vinput[SIZEOF_SIGNAL] = { 0 };
unsigned short voutput[SIZEOF_SIGNAL] = { 0 };

// Module Functions to Test ----------------------------------------------------
unsigned short CurrentLoop (unsigned short setpoint, unsigned short new_sample);
void CurrentLoop_Change_to_HighGain (void);
void CurrentLoop_Change_to_LowGain (void);

void Test_ACPOS (void);

unsigned short Plant_Out (short duty);
void HIGH_LEFT (unsigned short duty);
void LOW_RIGHT (unsigned short duty);

// Module Functions ------------------------------------------------------------
int main (int argc, char *argv[])
{
    PID_Small_Ki_Flush_Errors(&current_pid);
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
        Test_ACPOS();
    }
    
    printf("\nVector reference:\n");
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i+=8)
        printf("index: %03d d: %d %d %d %d %d %d %d %d\n",
               i,
               reference[i+0],
               reference[i+1],
               reference[i+2],
               reference[i+3],
               reference[i+4],
               reference[i+5],
               reference[i+6],
               reference[i+7]);

    printf("\nVector voltage input:\n");
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i+=8)
        printf("index: %03d d: %d %d %d %d %d %d %d %d\n",
               i,
               vinput[i+0],
               vinput[i+1],
               vinput[i+2],
               vinput[i+3],
               vinput[i+4],
               vinput[i+5],
               vinput[i+6],
               vinput[i+7]);
                              
    printf("\nVector duty_high_left:\n");
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i+=8)
        printf("index: %03d d: %d %d %d %d %d %d %d %d\n",
               i,
               duty_high_left[i+0],
               duty_high_left[i+1],
               duty_high_left[i+2],
               duty_high_left[i+3],
               duty_high_left[i+4],
               duty_high_left[i+5],
               duty_high_left[i+6],
               duty_high_left[i+7]);

    printf("\nVector plant output:\n");
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i+=8)
        printf("index: %03d d: %d %d %d %d %d %d %d %d\n",
               i,
               voutput[i+0],
               voutput[i+1],
               voutput[i+2],
               voutput[i+3],
               voutput[i+4],
               voutput[i+5],
               voutput[i+6],
               voutput[i+7]);

    printf("\nPlant output error:\n");
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i+=8)
        printf("index: %03d d: %d %d %d %d %d %d %d %d\n",
               i,
               reference[i+0] - voutput[i+0],
               reference[i+1] - voutput[i+1],
               reference[i+2] - voutput[i+2],
               reference[i+3] - voutput[i+3],
               reference[i+4] - voutput[i+4],
               reference[i+5] - voutput[i+5],
               reference[i+6] - voutput[i+6],
               reference[i+7] - voutput[i+7]);
    

    return 0;
}


///////////////////////////////////////////////
// Cosas que tienen que ver con las seniales //
///////////////////////////////////////////////
#define KI_SIGNAL_PEAK_MULTIPLIER    465   // depende de cual es la medicion del opamp de corriente
#define KI_SIGNAL_50_PERCENT         232   // 0.25 Apk


#define DUTY_NONE    0
#define DUTY_100_PERCENT    1000
#define DUTY_ALWAYS    (DUTY_100_PERCENT + 1)
/////////////////////////////////////////////
// Cosas que tienen que ver con mediciones //
/////////////////////////////////////////////
#define INDEX_TO_MIDDLE    47
#define INDEX_TO_FALLING    156
#define INDEX_TO_REVERT    204
    
#define I_Sense_Neg    10

unsigned short I_Sense_Pos = 0;
unsigned short last_output = 0;

unsigned short d = 0;
void Test_ACPOS (void)
{
    //Adelanto la seniales de corriente,
    if (p_current_ref < &sin_half_cycle[(SIZEOF_SIGNAL - 1)])
    {
        unsigned char signal_index = (unsigned char) (p_current_ref - sin_half_cycle);
                    
        //loop de corriente
        unsigned int calc = *p_current_ref * KI_SIGNAL_PEAK_MULTIPLIER;
        calc = calc >> 10;

        //TODO: modif
        reference[signal_index] = (unsigned short) calc;

        I_Sense_Pos = last_output;
        voutput[signal_index] = last_output;

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
            d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
            HIGH_LEFT(d);

            if (signal_index > INDEX_TO_FALLING)
            {
                CurrentLoop_Change_to_LowGain();
                signal_state = SIGNAL_FALLING;
            }
            break;

        case SIGNAL_FALLING:
            d = CurrentLoop ((unsigned short) calc, I_Sense_Pos);
            HIGH_LEFT(d);

            if (signal_index > INDEX_TO_REVERT)
            {
                // CurrentLoop_Change_to_LowGain();
                signal_state = SIGNAL_REVERT;
                HIGH_LEFT(DUTY_NONE);
            }
            break;

        case SIGNAL_REVERT:
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


unsigned short CurrentLoop (unsigned short setpoint, unsigned short new_sample)
{
    short d = 0;
    
    current_pid.setpoint = setpoint;
    current_pid.sample = new_sample;
    d = PID_Small_Ki(&current_pid);
                    
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
    current_pid.kp = 100;
    current_pid.ki = 320;
    current_pid.kd = 0;    
    // current_pid.kp = 5;
    // current_pid.ki = 32;
    // current_pid.kd = 16;    
}


unsigned char cntr_high_left = 0;
void HIGH_LEFT (unsigned short duty)
{
    duty_high_left[cntr_high_left] = duty;

    //aplico el nuevo duty a la planta
    last_output = Plant_Out(duty);
    cntr_high_left++;
}


unsigned short duty_low_right [SIZEOF_SIGNAL] = { 0 };
unsigned char cntr_low_right = 0;
void LOW_RIGHT (unsigned short duty)
{
    duty_low_right[cntr_low_right] = duty;
    cntr_low_right++;
}



//b[3]: [0.] b[2]: [0.02032562] b[1]: [0.0624813] b[0]: [0.01978482]
//a[3]: 1.0 a[2]: 0.011881459485754697 a[1]: 0.014346828516393684 a[0]: -0.9474935161284341
float output = 0;
float output_z1 = 0;
float output_z2 = 0;
float output_z3 = 0;
float input = 0;
float input_z1 = 0;
float input_z2 = 0;
float input_z3 = 0;

unsigned char cntr_plant = 0;
unsigned short Plant_Out (short duty)
{
    float output_b = 0.0;
    float output_a = 0.0;
    
    // output = 0. * input + 0.02032 * input_z1 + 0.06248 * input_z2 + 0.01978 * input_z3
    //     - 0.01188 * output_z1 - 0.01434 * output_z2 + 0.94749 * output_z3;

    input = vinput[cntr_plant] * duty / 1000;
    if (cntr_plant > 2)
    {
        output_b = 0. * input + 0.02032 * input_z1 + 0.06248 * input_z2 + 0.01978 * input_z3;
        output_a = 0.01188 * output_z1 + 0.01434 * output_z2 - 0.94749 * output_z3;

        output = output_b - output_a;

        input_z3 = input_z2;
        input_z2 = input_z1;        
        input_z1 = input;

        output_z3 = output_z2;
        output_z2 = output_z1;        
        output_z1 = output;
        
    }
    else if (cntr_plant > 1)
    {
        output_b = 0. * input + 0.02032 * input_z1 + 0.06248 * input_z2;
        output_a = 0.01188 * output_z1 + 0.01434 * output_z2;
        output = output_b - output_a;

        input_z2 = input_z1;
        input_z1 = input;

        output_z2 = output_z1;
        output_z1 = output;
    }
    else if (cntr_plant > 0)
    {
        output_b = 0. * input + 0.02032 * input_z1;
        output_a = 0.01188 * output_z1;
        output = output_b - output_a;

        input_z2 = input_z1;
        input_z1 = input;

        output_z2 = output_z1;
        output_z1 = output;
    }
    else
    {
        output = 0. * input;
        
        input_z1 = input;

        output_z1 = output;
    }

    cntr_plant++;

    return (unsigned short) output;
}


//--- end of file ---//


