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
#include "tests_recursive_utils.h"


#include <stdio.h>
#include <math.h>

// Types Constants and Macros --------------------------------------------------
#define LF    3.4e-3
#define FPWM    24.0e3
#define RLINE    11.
#define RSENSE    0.33

// Select how many cycles
#define HOW_MANY_CYCLES    2000
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
float vinput[VECTOR_LENGTH] = { 0 };
float vline[VECTOR_LENGTH] = { 0 };
float vinput_applied[VECTOR_LENGTH] = { 0 };
float voutput[VECTOR_LENGTH] = { 0 };
unsigned short voutput_adc[VECTOR_LENGTH] = { 0 };
unsigned short last_output = 0;

// Module Functions to Test ----------------------------------------------------
void Plant_Step_Response (void);
void Plant_Step_Response_Duty (void);

void TestGenSignal (void);
void TestSignalCloseLoop (void);
void TestSignalSinus (void);
void TestSignalPreDistorted (void);
void TestStepDCM (void);
void TestStepCCM (void);

void Filter_Step_Response (void);
float Plant_Out_Recursive (float in);
void Plant_Out_Recursive_Reset (void);


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
    // Filter_Step_Response();
    // Plant_Step_Response();
    // Plant_Step_Response_Duty();
    // TestSignalCloseLoop ();
    // TestSignalPreDistorted();
    // TestGenSignal();
    TestSignalSinus();    
    // TestStepDCM();
    // TestStepCCM();    
    return 0;
}


void TestSignalSinus (void)
{
    float calc = 0.0;
    int filter_cntr = 0;

    Plant_Out_Recursive_Reset();
    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            calc = sin (3.1415 * i / SIZEOF_SIGNAL);
            calc = calc * 311;
            vline[i + (j * SIZEOF_SIGNAL)] = calc;
            vinput[i + (j * SIZEOF_SIGNAL)] = 350;
        }
    }

    GenSignalSinusDutySet(100);

    gen_signal_e sig_state = SIGNAL_RUNNING;
    unsigned short duty = 0;
    unsigned short isense = 0;
    unsigned short ki_multiplier = KI_SIGNAL_PEAK_MULTIPLIER;    
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
            else
                HIGH_LEFT(0);
        }

        // GenSignalSinusApplyFilter();
        if (filter_cntr > 20)
        {
            GenSignalSinusApplyFilter();
            filter_cntr = 0;
        }
        else
            filter_cntr++;
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

    // ShowVectorUShort("\nVector reference:\n", reference, VECTOR_LENGTH);
    // ShowVectorUShort("\nVector voltage input:\n", vinput, VECTOR_LENGTH);
    // ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, VECTOR_LENGTH);
    // ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, VECTOR_LENGTH);

    // ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, VECTOR_LENGTH);
    // ShowVectorFloat("\nVector plant output:\n", voutput, VECTOR_LENGTH);

    // ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, VECTOR_LENGTH);

    int error [VECTOR_LENGTH] = { 0 };
    for (int i = 0; i < VECTOR_LENGTH; i++)
        error[i] = reference[i] - voutput_adc[i];

    // ShowVectorInt("\nPlant output error:\n", error, VECTOR_LENGTH);
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

    printf("\nRun by hand python3 simul_sinus_filter.py\n");    
    
}


void TestGenSignal (void)
{
    float calc = 0.0;

    Plant_Out_Recursive_Reset();
    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            calc = sin (3.1415 * i / SIZEOF_SIGNAL);
            calc = calc * 311;
            vline[i + (j * SIZEOF_SIGNAL)] = calc;
            vinput[i + (j * SIZEOF_SIGNAL)] = 350;
        }
    }

    gen_signal_e sig_state = SIGNAL_RUNNING;
    unsigned short duty = 0;
    unsigned short isense = 0;
    unsigned short ki_multiplier = KI_SIGNAL_PEAK_MULTIPLIER;    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        GenSignalReset();        
        sig_state = SIGNAL_RUNNING;
        duty = 0;
        isense = 0;
        
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            isense = last_output;
            sig_state = GenSignal(isense, ki_multiplier, &duty);
            if (sig_state == SIGNAL_RUNNING)
                HIGH_LEFT(duty);
            else
                HIGH_LEFT(0);
        
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

    // ShowVectorUShort("\nVector reference:\n", reference, VECTOR_LENGTH);
    // ShowVectorUShort("\nVector voltage input:\n", vinput, VECTOR_LENGTH);
    // ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, VECTOR_LENGTH);
    // ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, VECTOR_LENGTH);

    // ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, VECTOR_LENGTH);
    // ShowVectorFloat("\nVector plant output:\n", voutput, VECTOR_LENGTH);

    // ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, VECTOR_LENGTH);

    int error [VECTOR_LENGTH] = { 0 };
    for (int i = 0; i < VECTOR_LENGTH; i++)
        error[i] = reference[i] - voutput_adc[i];

    ShowVectorInt("\nPlant output error:\n", error, VECTOR_LENGTH);


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

    printf("\nRun by hand python3 simul_sinus_filter.py\n");    
    
}


void TestStepDCM (void)
{
    unsigned short duty_step [SIZEOF_SIGNAL] = { 0 };
    
    printf("\nFilter T in DCM Step Response\n");
    Plant_Out_Recursive_Reset();
    
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
    {
        vinput[i] = 350.0;
        vline[i] = 311.0;
    }

    // step 0.7 -> 0.87
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
    {
        if (i < (SIZEOF_SIGNAL / 2))
            duty_step[i] = 1400;
        else
            duty_step[i] = 1740;            
    }

    for (int i = 0; i < SIZEOF_SIGNAL; i++)
        HIGH_LEFT(duty_step[i]);
    

    ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);
    ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, SIZEOF_SIGNAL);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_Float_To_File(file, "vinput_applied", vinput_applied, SIZEOF_SIGNAL);
    Vector_Float_To_File(file, "voutput", voutput, SIZEOF_SIGNAL);    

    Vector_UShort_To_File(file, "duty_step", duty_step, SIZEOF_SIGNAL);
    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, SIZEOF_SIGNAL);    

    printf("\nRun by hand python3 simul_ccm_dcm_filter.py\n");
}


void TestStepCCM (void)
{
    unsigned short duty_step [SIZEOF_SIGNAL] = { 0 };
    
    printf("\nFilter T in CCM Step Response\n");
    Plant_Out_Recursive_Reset();
    
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
    {
        vinput[i] = 350.0;
        vline[i] = 311.0;
    }

    // step 0.9 -> 0.94
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
    {
        if (i < (SIZEOF_SIGNAL / 2))
            duty_step[i] = 1800;
        else
            duty_step[i] = 1880;            
    }

    
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
        HIGH_LEFT(duty_step[i]);
    

    ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);
    ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, SIZEOF_SIGNAL);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_Float_To_File(file, "vinput_applied", vinput_applied, SIZEOF_SIGNAL);
    Vector_Float_To_File(file, "voutput", voutput, SIZEOF_SIGNAL);    

    Vector_UShort_To_File(file, "duty_step", duty_step, SIZEOF_SIGNAL);
    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, SIZEOF_SIGNAL);    
    
    printf("\nRun by hand python3 simul_ccm_dcm_filter.py\n");
}

// void TestSignalPreDistorted (void)
// {
//     float calc = 0.0;
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         calc = sin (3.1415 * i / SIZEOF_SIGNAL);
//         calc = 350 - calc * 311;
//         vinput[i] = (unsigned short) calc;
//         // vinput[i] = 350;
//     }

//     GenSignalPreDistortedReset();
//     gen_signal_e sig_state = SIGNAL_RUNNING;
//     unsigned short duty = 0;
//     unsigned short isense = 0;
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         isense = last_output;
//         sig_state = GenSignalPreDistorted(isense, KI_SIGNAL_PEAK_MULTIPLIER, &duty);
//         if (sig_state == SIGNAL_RUNNING)
//             HIGH_LEFT(duty);
        
//     }

//     unsigned short reference [SIZEOF_SIGNAL] = { 0 };
//     unsigned int ref_calc = 0;
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         ref_calc = sin_half_cycle[i] * KI_SIGNAL_PEAK_MULTIPLIER;
//         ref_calc = ref_calc >> 12;
//         reference[i] = (unsigned short) ref_calc;
//     }

//     ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);
//     // ShowVectorUShort("\nVector voltage input:\n", vinput, SIZEOF_SIGNAL);
//     ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, SIZEOF_SIGNAL);
// //     ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, SIZEOF_SIGNAL);

//     ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, SIZEOF_SIGNAL);
//     ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);

//     ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, SIZEOF_SIGNAL);

//     int error [SIZEOF_SIGNAL] = { 0 };
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//         error[i] = reference[i] - voutput_adc[i];

//     ShowVectorInt("\nPlant output error:\n", error, SIZEOF_SIGNAL);
// //     ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);

//     ///////////////////////////
//     // Backup Data to a file //
//     ///////////////////////////
//     FILE * file = fopen("data.txt", "w");

//     if (file == NULL)
//     {
//         printf("data file not created!\n");
//         return;
//     }

//     Vector_UShort_To_File(file, "reference", reference, SIZEOF_SIGNAL);
//     Vector_UShort_To_File(file, "duty_high_left", duty_high_left, SIZEOF_SIGNAL);

//     Vector_Float_To_File(file, "vinput_applied", vinput_applied, SIZEOF_SIGNAL);
//     Vector_Float_To_File(file, "voutput", voutput, SIZEOF_SIGNAL);    

//     Vector_UShort_To_File(file, "voutput_adc", voutput_adc, SIZEOF_SIGNAL);
    
// }




// void TestSignalCloseLoop (void)
// {
//     float calc = 0.0;
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         calc = sin (3.1415 * i / SIZEOF_SIGNAL);
//         calc = 350 - calc * 311;
//         vinput[i] = (unsigned short) calc;
//         // vinput[i] = 350;
//     }

//     GenSignalReset();
//     gen_signal_e sig_state = SIGNAL_RUNNING;
//     unsigned short duty = 0;
//     unsigned short isense = 0;
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         isense = last_output;
//         sig_state = GenSignal(isense, KI_SIGNAL_PEAK_MULTIPLIER, &duty);
//         if (sig_state == SIGNAL_RUNNING)
//             HIGH_LEFT(duty);
        
//     }

//     unsigned short reference [SIZEOF_SIGNAL] = { 0 };
//     unsigned int ref_calc = 0;
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         ref_calc = sin_half_cycle[i] * KI_SIGNAL_PEAK_MULTIPLIER;
//         ref_calc = ref_calc >> 12;
//         reference[i] = (unsigned short) ref_calc;
//     }

//     ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);
//     // ShowVectorUShort("\nVector voltage input:\n", vinput, SIZEOF_SIGNAL);
//     ShowVectorUShort("\nVector duty_high_left:\n", duty_high_left, SIZEOF_SIGNAL);
// //     ShowVectorUShort("\nVector duty_high_right:\n", duty_high_right, SIZEOF_SIGNAL);

//     ShowVectorFloat("\nVector vinput_applied:\n", vinput_applied, SIZEOF_SIGNAL);
//     ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);

//     ShowVectorUShort("\nVector plant output ADC:\n", voutput_adc, SIZEOF_SIGNAL);

//     int error [SIZEOF_SIGNAL] = { 0 };
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//         error[i] = reference[i] - voutput_adc[i];

//     ShowVectorInt("\nPlant output error:\n", error, SIZEOF_SIGNAL);
// //     ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);

//     ///////////////////////////
//     // Backup Data to a file //
//     ///////////////////////////
//     FILE * file = fopen("data.txt", "w");

//     if (file == NULL)
//     {
//         printf("data file not created!\n");
//         return;
//     }

//     Vector_UShort_To_File(file, "reference", reference, SIZEOF_SIGNAL);
//     Vector_UShort_To_File(file, "duty_high_left", duty_high_left, SIZEOF_SIGNAL);

//     Vector_Float_To_File(file, "vinput_applied", vinput_applied, SIZEOF_SIGNAL);
//     Vector_Float_To_File(file, "voutput", voutput, SIZEOF_SIGNAL);    

//     Vector_UShort_To_File(file, "voutput_adc", voutput_adc, SIZEOF_SIGNAL);
    
// }



int cntr_high_left = 0;
void HIGH_LEFT (unsigned short duty)
{
    float out = 0.0;
    float input = 0.0;
    
    duty_high_left[cntr_high_left] = duty;

    //aplico el nuevo duty a la planta
    input = vinput[cntr_high_left] * duty;
    input = input / DUTY_100_PERCENT;

    //reviso si con el nuevo duty estoy en DCM o CCM
    if (input > vline[cntr_high_left])
    {
        //CCM
        vinput_applied[cntr_high_left] = input - vline[cntr_high_left];
    }
    else
    {
        //DCM
        if (duty)
        {
            float duty_sqr = duty;
            duty_sqr = duty_sqr / DUTY_100_PERCENT;
            duty_sqr = duty_sqr * duty_sqr;
            float re = 2 * LF * FPWM / duty_sqr;
            float rth = (RLINE + RSENSE) / (RLINE + RSENSE + re);

            // printf("re: %f rth: %f duty2: %f\n", re, rth, duty_sqr);
            vinput_applied[cntr_high_left] = (vinput[cntr_high_left] - vline[cntr_high_left]) * rth;
        }
        else
            vinput_applied[cntr_high_left] = 0.0;
    }

    voutput[cntr_high_left] = Plant_Out_Recursive(vinput_applied[cntr_high_left]);
    voutput_adc[cntr_high_left] = Adc12BitsConvertion(voutput[cntr_high_left]);
    last_output = voutput_adc[cntr_high_left];
    
    cntr_high_left++;
}

unsigned char cntr_high_right = 0;
// void HIGH_RIGHT (unsigned short duty)
// {
//     float out = 0.0;
//     float input = 0.0;
    
//     duty_high_right[cntr_high_right] = duty;

//     //aplico el nuevo duty a la planta
//     input = vinput[cntr_high_right] * duty;
//     input = input / DUTY_100_PERCENT;
//     vinput_applied[cntr_high_right] = input;

//     voutput[cntr_high_right] = Plant_Out(input);
//     voutput_adc[cntr_high_right] = Adc12BitsConvertion(voutput[cntr_high_right]);
//     last_output = voutput_adc[cntr_high_right];
    
//     cntr_high_right++;
// }


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


void Filter_Step_Response (void)
{
    printf("\nFilter T Step Response\n");
    Plant_Out_Recursive_Reset();
    
    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        vinput_applied[i] = 1.;
    }

    for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
    {
        voutput[i] = Plant_Out_Recursive(vinput_applied[i]);
    }
    

    ShowVectorFloat("\nVector voltage input:\n", vinput_applied, SIZEOF_SIGNAL);
    ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_Float_To_File(file, "vinput_applied", vinput_applied, SIZEOF_SIGNAL);
    Vector_Float_To_File(file, "voutput", voutput, SIZEOF_SIGNAL);

    printf("\nRun by hand python3 simul_filter.py\n");

}


// Parametros Filtro T digitales desde filtro_t_analog_digi.py Vinput = 1
// Filtro y Opamp Digital Zoh:
// TransferFunctionDiscrete(
// array([0.00296021, 0.0101069 , 0.00276169]),
// array([ 1.        , -1.02797943,  1.02185231, -0.87036028]),
// dt: 4.1666666666666665e-05
// )
float b_vector [3] = { 0.00296 , 0.0101, 0.00276 };
float a_vector [4] = { 1., -1.02798, 1.02185, -0.87036 };
float ins_vector [3] = { 0.0 };
float outs_vector [4] = { 0.0 };
recursive_filter_t filter_t;
void Plant_Out_Recursive_Reset (void)
{
    filter_t.b_params = b_vector;
    filter_t.a_params = a_vector;
    filter_t.b_size = 3;
    filter_t.a_size = 4;
    filter_t.last_inputs = ins_vector;
    filter_t.last_outputs = outs_vector;    
    Recursive_Filter_Float_Reset(&filter_t);
}


float Plant_Out_Recursive (float in)
{
    return Recursive_Filter_Float(&filter_t, in);
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


