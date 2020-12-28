//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS_SIGNALS.C #######################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "gen_signal.h"
#include "pwm_defs.h"
#include "dsp.h"
#include "tests_vector_utils.h"

#include <stdio.h>
#include <math.h>

// Types Constants and Macros --------------------------------------------------
// Select how many cycles
#define HOW_MANY_CYCLES    20
// Select Current Signal
// #define USE_SIGNAL_CURRENT_01_A
#define USE_SIGNAL_CURRENT_05_A
// #define USE_SIGNAL_CURRENT_075_A
// #define USE_SIGNAL_CURRENT_1_A

// Set of Peak Current in ADC points, 12bits
// Voltage in I_Sense is 3 . Ipeak
// ADC points = 3 . Ipeak . 4095 / 3.3
#ifdef USE_SIGNAL_CURRENT_01_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    373
#endif

#ifdef USE_SIGNAL_CURRENT_05_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1861
#endif

#ifdef USE_SIGNAL_CURRENT_075_A
#define KI_SIGNAL_PEAK_MULTIPLIER    2792
#endif

#ifdef USE_SIGNAL_CURRENT_1_A
#define KI_SIGNAL_PEAK_MULTIPLIER    3722
#endif


// Externals -------------------------------------------------------------------
extern unsigned short sin_half_cycle [];

// Globals ---------------------------------------------------------------------
#define VECTOR_LENGTH    (SIZEOF_SIGNAL * HOW_MANY_CYCLES)
unsigned short duty_high_left [VECTOR_LENGTH] = { 0 };
unsigned short duty_high_right [VECTOR_LENGTH] = { 0 };
unsigned short vinput[VECTOR_LENGTH] = { 0 };
float vinput_applied[VECTOR_LENGTH] = { 0 };
float voutput[VECTOR_LENGTH] = { 0 };
unsigned short voutput_adc[VECTOR_LENGTH] = { 0 };
unsigned short last_output = 0;

// Module Functions to Test ----------------------------------------------------
float Plant_Out (float);
void Plant_Step_Response (void);
void Plant_Step_Response_Duty (void);
void TestSignalCloseLoop (void);
void TestSignalSinus (void);
void TestSignalPreDistorted (void);

unsigned short Adc12BitsConvertion (float );
void HIGH_LEFT (unsigned short duty);
void LOW_RIGHT (unsigned short duty);

void HIGH_RIGHT (unsigned short duty);
void LOW_LEFT (unsigned short duty);


void PrintOK (void);
void PrintERR (void);


// Module Functions ------------------------------------------------------------
int main (int argc, char *argv[])
{
    printf("Start of Analog simulations...\n");
    // Plant_Step_Response();
    // Plant_Step_Response_Duty();
    // TestSignalCloseLoop ();
    // TestSignalPreDistorted();
    TestSignalSinus();
    return 0;
}


void TestSignalSinus (void)
{
    float calc = 0.0;

    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            calc = sin (3.1415 * i / SIZEOF_SIGNAL);
            calc = 350 - calc * 311;
            vinput[i + (j * SIZEOF_SIGNAL)] = (unsigned short) calc;
            // vinput[i] = 350;
        }
    }

    gen_signal_e sig_state = SIGNAL_RUNNING;
    unsigned short duty = 0;
    unsigned short isense = 0;
    unsigned short ki_multiplier = 180;
    // unsigned short ki_multiplier = KI_SIGNAL_PEAK_MULTIPLIER;    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        GenSignalSinusReset();
        sig_state = SIGNAL_RUNNING;
        duty = 0;
        isense = 0;
        
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            isense = last_output;
            sig_state = GenSignalSinus(isense, ki_multiplier, &duty);
            if (sig_state == SIGNAL_RUNNING)
                HIGH_LEFT(duty);
        
        }
    }

    unsigned short reference [VECTOR_LENGTH] = { 0 };
    unsigned int ref_calc = 0;
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_half_cycle[i] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * SIZEOF_SIGNAL)] = (unsigned short) ref_calc;
        }
    }

    ShowVectorUShort("\nVector reference:\n", reference, VECTOR_LENGTH);
    // ShowVectorUShort("\nVector voltage input:\n", vinput, VECTOR_LENGTH);
    ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, VECTOR_LENGTH);
//     ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, VECTOR_LENGTH);

    ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, VECTOR_LENGTH);
    ShowVectorFloat("\nVector plant output:\n", voutput, VECTOR_LENGTH);

    ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, VECTOR_LENGTH);

    int error [VECTOR_LENGTH] = { 0 };
    for (int i = 0; i < VECTOR_LENGTH; i++)
        error[i] = reference[i] - voutput_adc[i];

    ShowVectorInt("\nPlant output error:\n", error, VECTOR_LENGTH);
//     ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_UShort_To_File(file, "reference", reference, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "vinput", vinput, VECTOR_LENGTH);    
    Vector_UShort_To_File(file, "duty_high_left", duty_high_left, VECTOR_LENGTH);

    Vector_Float_To_File(file, "vinput applied", vinput_applied, VECTOR_LENGTH);
    Vector_Float_To_File(file, "voutput getted", voutput, VECTOR_LENGTH);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, VECTOR_LENGTH);
    
}


void TestSignalPreDistorted (void)
{
    float calc = 0.0;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        calc = sin (3.1415 * i / SIZEOF_SIGNAL);
        calc = 350 - calc * 311;
        vinput[i] = (unsigned short) calc;
        // vinput[i] = 350;
    }

    GenSignalPreDistortedReset();
    gen_signal_e sig_state = SIGNAL_RUNNING;
    unsigned short duty = 0;
    unsigned short isense = 0;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        isense = last_output;
        sig_state = GenSignalPreDistorted(isense, KI_SIGNAL_PEAK_MULTIPLIER, &duty);
        if (sig_state == SIGNAL_RUNNING)
            HIGH_LEFT(duty);
        
    }

    unsigned short reference [SIZEOF_SIGNAL] = { 0 };
    unsigned int ref_calc = 0;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        ref_calc = sin_half_cycle[i] * KI_SIGNAL_PEAK_MULTIPLIER;
        ref_calc = ref_calc >> 12;
        reference[i] = (unsigned short) ref_calc;
    }

    ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);
    // ShowVectorUShort("\nVector voltage input:\n", vinput, SIZEOF_SIGNAL);
    ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, SIZEOF_SIGNAL);
//     ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, SIZEOF_SIGNAL);

    ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);

    ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, SIZEOF_SIGNAL);

    int error [SIZEOF_SIGNAL] = { 0 };
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
        error[i] = reference[i] - voutput_adc[i];

    ShowVectorInt("\nPlant output error:\n", error, SIZEOF_SIGNAL);
//     ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_UShort_To_File(file, "reference", reference, SIZEOF_SIGNAL);
    Vector_UShort_To_File(file, "duty_high_left", duty_high_left, SIZEOF_SIGNAL);

    Vector_Float_To_File(file, "vinput_applied", vinput_applied, SIZEOF_SIGNAL);
    Vector_Float_To_File(file, "voutput", voutput, SIZEOF_SIGNAL);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, SIZEOF_SIGNAL);
    
}




void TestSignalCloseLoop (void)
{
    float calc = 0.0;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        calc = sin (3.1415 * i / SIZEOF_SIGNAL);
        calc = 350 - calc * 311;
        vinput[i] = (unsigned short) calc;
        // vinput[i] = 350;
    }

    GenSignalReset();
    gen_signal_e sig_state = SIGNAL_RUNNING;
    unsigned short duty = 0;
    unsigned short isense = 0;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        isense = last_output;
        sig_state = GenSignal(isense, KI_SIGNAL_PEAK_MULTIPLIER, &duty);
        if (sig_state == SIGNAL_RUNNING)
            HIGH_LEFT(duty);
        
    }

    unsigned short reference [SIZEOF_SIGNAL] = { 0 };
    unsigned int ref_calc = 0;
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        ref_calc = sin_half_cycle[i] * KI_SIGNAL_PEAK_MULTIPLIER;
        ref_calc = ref_calc >> 12;
        reference[i] = (unsigned short) ref_calc;
    }

    ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);
    // ShowVectorUShort("\nVector voltage input:\n", vinput, SIZEOF_SIGNAL);
    ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, SIZEOF_SIGNAL);
//     ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, SIZEOF_SIGNAL);

    ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);

    ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, SIZEOF_SIGNAL);

    int error [SIZEOF_SIGNAL] = { 0 };
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
        error[i] = reference[i] - voutput_adc[i];

    ShowVectorInt("\nPlant output error:\n", error, SIZEOF_SIGNAL);
//     ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_UShort_To_File(file, "reference", reference, SIZEOF_SIGNAL);
    Vector_UShort_To_File(file, "duty_high_left", duty_high_left, SIZEOF_SIGNAL);

    Vector_Float_To_File(file, "vinput_applied", vinput_applied, SIZEOF_SIGNAL);
    Vector_Float_To_File(file, "voutput", voutput, SIZEOF_SIGNAL);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, SIZEOF_SIGNAL);
    
}




int cntr_high_left = 0;
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


//pruebo un step de la planta pero con duty y vinput
//la tension de entrada es tan alta que incluso con duty_max = 10000
//tengo errores del 2%
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


void PrintOK (void)
{
    printf("\033[0;32m");    //green
    printf("OK\n");
    printf("\033[0m");    //reset
}


void PrintERR (void)
{
    printf("\033[0;31m");    //red
    printf("ERR\n");
    printf("\033[0m");    //reset
}
//--- end of file ---//


