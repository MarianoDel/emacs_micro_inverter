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
#define HOW_MANY_CYCLES    400
// Select Current Signal
// #define USE_SIGNAL_CURRENT_01_A
// #define USE_SIGNAL_CURRENT_02_A
// #define USE_SIGNAL_CURRENT_05_A
// #define USE_SIGNAL_CURRENT_075_A
#define USE_SIGNAL_CURRENT_1_A

#ifdef USE_SIGNAL_CURRENT_005_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    90
#endif

#ifdef USE_SIGNAL_CURRENT_01_A         
#define KI_SIGNAL_PEAK_MULTIPLIER    180
#endif

#ifdef USE_SIGNAL_CURRENT_02_A
#define KI_SIGNAL_PEAK_MULTIPLIER    360
#endif

#ifdef USE_SIGNAL_CURRENT_03_A
#define KI_SIGNAL_PEAK_MULTIPLIER    540
#endif

#ifdef USE_SIGNAL_CURRENT_04_A
#define KI_SIGNAL_PEAK_MULTIPLIER    720
#endif

#ifdef USE_SIGNAL_CURRENT_05_A
#define KI_SIGNAL_PEAK_MULTIPLIER    900
#endif

#ifdef USE_SIGNAL_CURRENT_06_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1080
#endif

#ifdef USE_SIGNAL_CURRENT_075_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1350
#endif

#ifdef USE_SIGNAL_CURRENT_1_A
#define KI_SIGNAL_PEAK_MULTIPLIER    1800
#endif


// Externals -------------------------------------------------------------------
extern unsigned short sin_half_cycle [];
extern short sin_full_cycle [];

unsigned short d_dump [SIZEOF_SIGNAL] = { 0 };
unsigned short i_dump [SIZEOF_SIGNAL] = { 0 };
unsigned short r_dump [SIZEOF_SIGNAL] = { 0 };


// Globals ---------------------------------------------------------------------
#define VECTOR_LENGTH    (2*SIZEOF_SIGNAL * HOW_MANY_CYCLES)
unsigned short duty_high_left [VECTOR_LENGTH] = { 0 };
unsigned short duty_high_right [VECTOR_LENGTH] = { 0 };
short duty_bipolar [VECTOR_LENGTH] = { 0 };
float vinput[VECTOR_LENGTH] = { 0 };
float vline[VECTOR_LENGTH] = { 0 };
float vinput_applied[VECTOR_LENGTH] = { 0 };
float voutput[VECTOR_LENGTH] = { 0 };
unsigned short voutput_adc[VECTOR_LENGTH] = { 0 };
unsigned short current_mode[VECTOR_LENGTH] = { 0 };
unsigned short last_output = 0;
short last_output_bipolar = 0;

// Module Functions to Test ----------------------------------------------------
void Plant_Step_Response (void);
void Plant_Step_Response_Duty (void);

void TestGenSignal (void);
void TestGenSignalVoltage (void);
void TestGenSignalBipolar (void);
void TestSignalCloseLoop (void);
void TestSignalSinus (void);
void TestSignalPreDistorted (void);
void TestStepDCM (void);
void TestStepCCM (void);
void TestInputToOutput (void);
void TestRecursiveDump (void);
void TestPRController (void);

void Filter_Step_Response (void);
float Plant_Out_Recursive (float in);
void Plant_Out_Recursive_Reset (void);


unsigned short Adc12BitsConvertion (float );
void HIGH_LEFT (unsigned short duty);
void LOW_RIGHT (unsigned short duty);

void HIGH_RIGHT (unsigned short duty);
void LOW_LEFT (unsigned short duty);

int GetPwmCounter (void);
void HIGH_BIPOLAR (short duty);

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
    TestGenSignalVoltage();    
    // TestInputToOutput ();    
    // TestGenSignalBipolar();    
    // TestSignalSinus();    
    // TestStepDCM();
    // TestStepCCM();

    // TestRecursiveDump ();
    // TestPRController ();
    return 0;
}


void TestSignalSinus (void)
{
    float calc = 0.0;
    // int filter_cntr = 0;

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

        // if (filter_cntr > 50)
        // {
        //     filter_cntr = 0;
        //     GenSignalSinusApplyFilter ();
        // }
        // else
        //     filter_cntr++;
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
    // ShowVectorUShort("\nVector reference:\n", reference, SIZEOF_SIGNAL);

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
        for (int i = 0; i < (2*SIZEOF_SIGNAL); i++)
        {
            calc = sin (3.1415 * i / SIZEOF_SIGNAL);
            calc = calc * 311;
            vline[i + (j * (2*SIZEOF_SIGNAL))] = calc;
            vinput[i + (j * (2*SIZEOF_SIGNAL))] = 350;
        }
    }

    //distorsiono entrada
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 95; i < 145; i++)
        {
            vline[i + (j * 2*SIZEOF_SIGNAL)] = 295;
            vline[i + SIZEOF_SIGNAL + (j * 2*SIZEOF_SIGNAL)] = -295;
        }
    }

    // ShowVectorFloat("\nVector line:\n", vline, VECTOR_LENGTH);
    // ShowVectorFloat("\nVector input:\n", vinput, VECTOR_LENGTH);    

    gen_signal_e sig_state = SIGNAL_RUNNING;
    short duty = 0;
    short ki_multiplier = KI_SIGNAL_PEAK_MULTIPLIER;

    GenSignalControlInit();
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        //primera parte de la senial
        GenSignalReset();
        sig_state = SIGNAL_RUNNING;
        duty = 0;
        
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            sig_state = GenSignal(last_output_bipolar, ki_multiplier, &duty);
            // sig_state = GenSignal(last_output, ki_multiplier, &duty);            
            if (sig_state == SIGNAL_RUNNING)
            {
                //reviso que duty sea siempre positivo en esta parte
                if (duty < 0)
                    duty = 0;

                // HIGH_LEFT(duty);
                HIGH_BIPOLAR(duty);

                // printf("index: %d duty: %d\n", i, duty);
            }
            else
            {
                // HIGH_LEFT(0);
                HIGH_BIPOLAR(0);
            }
        
        }

        // segunda parte de la senial
        GenSignalReset();
        sig_state = SIGNAL_RUNNING;
        duty = 0;
        
        for (int i = SIZEOF_SIGNAL; i < 2*SIZEOF_SIGNAL; i++)
        {
            // sig_state = GenSignal(last_output, ki_multiplier, &duty);
            // printf("index: %d i: %d ",GetPwmCounter(), last_output_bipolar);
            sig_state = GenSignal(last_output_bipolar, ki_multiplier, &duty);
            // printf("duty: %d\n",duty);
            if (sig_state == SIGNAL_RUNNING)
            {
                //reviso que duty sea siempre positivo
                if (duty < 0)
                    duty = 0;
                
                // HIGH_RIGHT(duty);
                HIGH_BIPOLAR(-duty);                
            }
            else
            {
                // HIGH_RIGHT(0);
                HIGH_BIPOLAR(0);
            }
        
        }
        
    }

    short reference [VECTOR_LENGTH] = { 0 };
    unsigned int ref_calc = 0;
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_half_cycle[i] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * 2*SIZEOF_SIGNAL)] = (unsigned short) ref_calc;
        }

        for (int i = SIZEOF_SIGNAL; i < 2*SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_half_cycle[i - SIZEOF_SIGNAL] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * 2*SIZEOF_SIGNAL)] = -(short) ref_calc;
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


    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    
    Vector_Float_To_File(file, "vline", vline, VECTOR_LENGTH);
    Vector_Float_To_File(file, "vinput", vinput, VECTOR_LENGTH);

    Vector_Short_To_File(file, "reference", reference, VECTOR_LENGTH);
    Vector_Short_To_File(file, "duty_bipolar", duty_bipolar, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "reference", reference, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "vinput", vinput, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "duty_high_left", duty_high_left, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "duty_high_right", duty_high_right, VECTOR_LENGTH);    

    Vector_Float_To_File(file, "vinput applied", vinput_applied, VECTOR_LENGTH);
    Vector_Float_To_File(file, "voutput getted", voutput, VECTOR_LENGTH);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, VECTOR_LENGTH);
    Vector_UShort_To_File(file, "current_mode", current_mode, VECTOR_LENGTH);    

    printf("\nRun by hand python3 simul_sinus_filter.py\n");    
    
}


void TestGenSignalVoltage (void)
{
    float calc = 0.0;

    Plant_Out_Recursive_Reset();
    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < (2*SIZEOF_SIGNAL); i++)
        {
            calc = sin (3.1415 * i / SIZEOF_SIGNAL);
            calc = calc * 311;
            vline[i + (j * (2*SIZEOF_SIGNAL))] = calc;
            vinput[i + (j * (2*SIZEOF_SIGNAL))] = 350;
        }
    }

    //distorsiono entrada
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 95; i < 145; i++)
        {
            vline[i + (j * 2*SIZEOF_SIGNAL)] = 295;
            vline[i + SIZEOF_SIGNAL + (j * 2*SIZEOF_SIGNAL)] = -295;
        }
    }

    // ShowVectorFloat("\nVector line:\n", vline, VECTOR_LENGTH);
    // ShowVectorFloat("\nVector input:\n", vinput, VECTOR_LENGTH);    

    gen_signal_e sig_state = SIGNAL_RUNNING;
    short duty = 0;
    short ki_multiplier = KI_SIGNAL_PEAK_MULTIPLIER;

    GenSignalVoltageInit();    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        //primera parte de la senial
        GenSignalVoltageReset(KI_SIGNAL_PEAK_MULTIPLIER);
        sig_state = SIGNAL_RUNNING;
        duty = 0;
        
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            unsigned short vline_adc = vline[i] * 10;
            sig_state = GenSignalVoltage(vline_adc, last_output_bipolar, &duty);
            if (sig_state == SIGNAL_RUNNING)
            {
                //reviso que duty sea siempre positivo en esta parte
                if (duty < 0)
                    duty = 0;

                // HIGH_LEFT(duty);
                HIGH_BIPOLAR(duty);

                // printf("index: %d duty: %d\n", i, duty);
            }
            else
            {
                // HIGH_LEFT(0);
                HIGH_BIPOLAR(0);
            }
        
        }

        // segunda parte de la senial
        GenSignalVoltageNReset(KI_SIGNAL_PEAK_MULTIPLIER);
        sig_state = SIGNAL_RUNNING;
        duty = 0;
        
        for (int i = SIZEOF_SIGNAL; i < 2*SIZEOF_SIGNAL; i++)
        {
            unsigned short vline_adc = vline[i] * -10;
            sig_state = GenSignalVoltageN(vline_adc, last_output_bipolar, &duty);

            if (sig_state == SIGNAL_RUNNING)
            {
                //reviso que duty sea siempre positivo
                if (duty < 0)
                    duty = 0;
                
                // HIGH_RIGHT(duty);
                HIGH_BIPOLAR(-duty);                
            }
            else
            {
                // HIGH_RIGHT(0);
                HIGH_BIPOLAR(0);
            }
        
        }
        
    }

    short reference [VECTOR_LENGTH] = { 0 };
    unsigned int ref_calc = 0;
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_half_cycle[i] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * 2*SIZEOF_SIGNAL)] = (unsigned short) ref_calc;
        }

        for (int i = SIZEOF_SIGNAL; i < 2*SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_half_cycle[i - SIZEOF_SIGNAL] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * 2*SIZEOF_SIGNAL)] = -(short) ref_calc;
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


    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    
    Vector_Float_To_File(file, "vline", vline, VECTOR_LENGTH);
    Vector_Float_To_File(file, "vinput", vinput, VECTOR_LENGTH);

    Vector_Short_To_File(file, "reference", reference, VECTOR_LENGTH);
    Vector_Short_To_File(file, "duty_bipolar", duty_bipolar, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "reference", reference, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "vinput", vinput, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "duty_high_left", duty_high_left, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "duty_high_right", duty_high_right, VECTOR_LENGTH);    

    Vector_Float_To_File(file, "vinput applied", vinput_applied, VECTOR_LENGTH);
    Vector_Float_To_File(file, "voutput getted", voutput, VECTOR_LENGTH);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, VECTOR_LENGTH);
    Vector_UShort_To_File(file, "current_mode", current_mode, VECTOR_LENGTH);    

    printf("\nRun by hand python3 simul_sinus_filter.py\n");    
    
}


void TestGenSignalBipolar (void)
{
    float calc = 0.0;

    Plant_Out_Recursive_Reset();
    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < (2*SIZEOF_SIGNAL); i++)
        {
            calc = sin (3.1415 * i / SIZEOF_SIGNAL);
            calc = calc * 311;
            vline[i + (j * (2*SIZEOF_SIGNAL))] = calc;
            vinput[i + (j * (2*SIZEOF_SIGNAL))] = 350;
        }
    }

    // ShowVectorFloat("\nVector line:\n", vline, VECTOR_LENGTH);
    // ShowVectorFloat("\nVector input:\n", vinput, VECTOR_LENGTH);    

    gen_signal_e sig_state = SIGNAL_RUNNING;
    short duty = 0;
    short ki_multiplier = KI_SIGNAL_PEAK_MULTIPLIER;

    GenSignalControlInit();
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        GenSignalResetBipolar();
        duty = 0;
        
        for (int i = 0; i < 2*SIZEOF_SIGNAL; i++)
        {
            GenSignalBipolar(last_output_bipolar, ki_multiplier, &duty);
            HIGH_BIPOLAR(duty);
        }
    }

    short reference [VECTOR_LENGTH] = { 0 };
    unsigned int ref_calc = 0;
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < 2*SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_full_cycle[i] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * 2*SIZEOF_SIGNAL)] = (short) ref_calc;
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


    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_Float_To_File(file, "vline", vline, VECTOR_LENGTH);
    Vector_Float_To_File(file, "vinput", vinput, VECTOR_LENGTH);

    Vector_Short_To_File(file, "reference", reference, VECTOR_LENGTH);
    Vector_Short_To_File(file, "duty_bipolar", duty_bipolar, VECTOR_LENGTH);

    Vector_Float_To_File(file, "vinput applied", vinput_applied, VECTOR_LENGTH);
    Vector_Float_To_File(file, "voutput getted", voutput, VECTOR_LENGTH);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, VECTOR_LENGTH);

    printf("\nRun by hand python3 simul_bipolar_filter.py\n");    
    
}


void TestInputToOutput (void)
{
    printf("Input from dump data\n");
    float calc = 0.0;

    Plant_Out_Recursive_Reset();
    
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
    {
        calc = sin (3.1415 * i / SIZEOF_SIGNAL);
        calc = calc * 311;
        vline[i] = calc;
        vinput[i] = 350;
    }

    short duty_dump [SIZEOF_SIGNAL] = { 0, 6, 15, 28, 42, 58, 76, 94, 114, 139,
                                        160, 182, 205, 227, 248, 269, 289, 308, 327, 346,
                                        361, 380, 400, 419, 441, 463, 487, 511, 536, 561,
                                        588, 615, 643, 671, 700, 729, 759, 791, 823, 856,
                                        889, 923, 958, 994, 1032, 1073, 1116, 1160, 1207, 1253,
                                        1297, 1331, 1349, 1341, 1303, 1230, 1120, 982, 835, 709,
                                        629, 602, 615, 670, 757, 867, 992, 1129, 1274, 1424,
                                        1581, 1733, 1841, 1933, 2000, 2000, 1969, 1870, 1693, 1322,
                                        850, 295, 0, 0, 0, 0, 0, 0, 54, 157,
                                        297, 463, 648, 846, 1054, 1268, 1485, 1704, 1923, 2000,
                                        2000, 2000, 2000, 2000, 2000, 1971, 1796, 1681, 1454, 1189,
                                        906, 646, 462, 366, 349, 399, 500, 641, 811, 1003,
                                        1208, 1423, 1642, 1864, 2000, 2000, 2000, 2000, 2000, 2000,
                                        2000, 1981, 1919, 1813, 1664, 1393, 1079, 744, 443, 229,
                                        109, 26, 61, 147, 273, 428, 603, 792, 989, 1191,
                                        1394, 1596, 1793, 1934, 2000, 2000, 2000, 1997, 1919, 1753,
                                        1489, 966, 311, 0, 0, 0, 0, 0, 0, 0, 14,
                                        77, 176, 298, 437, 590, 742, 894, 1043, 1183, 1313,
                                        1430, 1529, 1589, 1584, 1477, 1234, 826, 216, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 13, 56, 120, 197,
                                        281, 367, 451, 530, 598, 653, 688, 695, 667, 588,
                                        445, 233, 0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 11, 29, 51, 72, 92, 108, 117, 116,
                                        93, 40, 0, 0, 0, 0, 0, 0};

    GenSignalResetBipolar();
        
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
    {
        HIGH_BIPOLAR(duty_dump[i]);
    }

    short reference [VECTOR_LENGTH] = { 0 };
    unsigned int ref_calc = 0;
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
    {
        ref_calc = sin_half_cycle[i] * KI_SIGNAL_PEAK_MULTIPLIER;
        ref_calc = ref_calc >> 12;
        reference[i] = (short) ref_calc;
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


    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    Vector_Float_To_File(file, "vline", vline, VECTOR_LENGTH);
    Vector_Float_To_File(file, "vinput", vinput, VECTOR_LENGTH);

    Vector_Short_To_File(file, "reference", reference, VECTOR_LENGTH);
    Vector_Short_To_File(file, "duty_bipolar", duty_bipolar, VECTOR_LENGTH);

    Vector_Float_To_File(file, "vinput applied", vinput_applied, VECTOR_LENGTH);
    Vector_Float_To_File(file, "voutput getted", voutput, VECTOR_LENGTH);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, VECTOR_LENGTH);

    printf("\nRun by hand python3 simul_bipolar_filter.py\n");    
    
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



int pwm_cntr = 0;
void HIGH_LEFT (unsigned short duty)
{
    float out = 0.0;
    float input = 0.0;
    
    duty_high_left[pwm_cntr] = duty;

    //aplico el nuevo duty a la planta
    input = vinput[pwm_cntr] * duty;
    input = input / DUTY_100_PERCENT;

    //reviso si con el nuevo duty estoy en DCM o CCM
    if (input > vline[pwm_cntr])
    {
        //CCM
        vinput_applied[pwm_cntr] = input - vline[pwm_cntr];
    }
    else
    {
        //DCM
        if (duty > 0)
        {
            float duty_sqr = duty;
            duty_sqr = duty_sqr / DUTY_100_PERCENT;
            duty_sqr = duty_sqr * duty_sqr;
            float re = 2 * LF * FPWM / duty_sqr;
            float rth = (RLINE + RSENSE) / (RLINE + RSENSE + re);

            // printf("re: %f rth: %f duty2: %f\n", re, rth, duty_sqr);
            vinput_applied[pwm_cntr] = (vinput[pwm_cntr] - vline[pwm_cntr]) * rth;
        }
        else
            vinput_applied[pwm_cntr] = 0.0;
    }

    voutput[pwm_cntr] = Plant_Out_Recursive(vinput_applied[pwm_cntr]);

    voutput_adc[pwm_cntr] = Adc12BitsConvertion(voutput[pwm_cntr]);
    last_output = voutput_adc[pwm_cntr];
    
    pwm_cntr++;
}


int GetPwmCounter (void)
{
    return pwm_cntr;
}


void HIGH_RIGHT (unsigned short duty)
{
    float out = 0.0;
    float input = 0.0;
    
    duty_high_right[pwm_cntr] = duty;

    //aplico el nuevo duty a la planta
    input = vinput[pwm_cntr] * duty;
    input = input / DUTY_100_PERCENT;
    input = -input;

    // printf("hl duty: %d, index: %d\n", duty, pwm_cntr);
    //reviso si con el nuevo duty estoy en DCM o CCM
    if (input < vline[pwm_cntr])
    {
        //CCM
        vinput_applied[pwm_cntr] = input - vline[pwm_cntr];
    }
    else
    {
        //DCM
        if (duty > 0)
        {
            float duty_sqr = duty;
            duty_sqr = duty_sqr / DUTY_100_PERCENT;
            duty_sqr = duty_sqr * duty_sqr;
            float re = 2 * LF * FPWM / duty_sqr;
            float rth = (RLINE + RSENSE) / (RLINE + RSENSE + re);

            // printf("re: %f rth: %f duty2: %f\n", re, rth, duty_sqr);
            vinput_applied[pwm_cntr] = (-vinput[pwm_cntr] - vline[pwm_cntr]) * rth;
        }
        else
            vinput_applied[pwm_cntr] = 0.0;
    }
    
    voutput[pwm_cntr] = Plant_Out_Recursive(vinput_applied[pwm_cntr]);

    //lo que sale de la planta se invierte antes del ADC
    float current_sample = 0.0;
    if (voutput[pwm_cntr] < 0.00001)
        current_sample = -voutput[pwm_cntr];

    voutput_adc[pwm_cntr] = Adc12BitsConvertion(current_sample);
    last_output = voutput_adc[pwm_cntr];
    
    pwm_cntr++;
}


// #define BIPOLAR_VERBOSE
unsigned char last_mode = 0;
void HIGH_BIPOLAR (short duty)
{
    float out = 0.0;
    float input = 0.0;
    
    duty_bipolar[pwm_cntr] = duty;

    //aplico el nuevo duty a la planta
    input = vinput[pwm_cntr] * duty;
    input = input / DUTY_100_PERCENT;

    //reviso cuadrantes
    if ((input > 0) && (vline[pwm_cntr] > 0))    //primer cuadrante
    {
#ifdef BIPOLAR_VERBOSE
        printf("cuadrante 1 ");
#endif
        if (input > vline[pwm_cntr])    //CCM
        {
            vinput_applied[pwm_cntr] = input - vline[pwm_cntr];
#ifdef BIPOLAR_VERBOSE
            printf("ccm ");
#endif
            last_mode = 1;
            current_mode[pwm_cntr] = 1000;
        }
        else    //DCM
        {
            float duty_sqr = duty;
            duty_sqr = duty_sqr / DUTY_100_PERCENT;
            duty_sqr = duty_sqr * duty_sqr;
            float re = 2 * LF * FPWM / duty_sqr;
            float rth = (RLINE + RSENSE) / (RLINE + RSENSE + re);

            // printf("re: %f rth: %f duty2: %f\n", re, rth, duty_sqr);
            vinput_applied[pwm_cntr] = (vinput[pwm_cntr] - vline[pwm_cntr]) * rth;
#ifdef BIPOLAR_VERBOSE
            printf("dcm ");
#endif
            last_mode = 0;
            current_mode[pwm_cntr] = 0;
        }
#ifdef BIPOLAR_VERBOSE  
        printf("applied: %f cntr: %d\n", vinput_applied[pwm_cntr], pwm_cntr);
#endif
    }
    else if ((input < 0) && (vline[pwm_cntr] < 0))    //tercer cuadrante
    {
#ifdef BIPOLAR_VERBOSE
        printf("cuadrante 3 ");
#endif
        if (input < vline[pwm_cntr])    //CCM
        {
            vinput_applied[pwm_cntr] = input - vline[pwm_cntr];
#ifdef BIPOLAR_VERBOSE
            printf("ccm ");
#endif
            last_mode = 1;
            current_mode[pwm_cntr] = 1000;
        }
        else    //DCM
        {
            float duty_sqr = duty;
            duty_sqr = duty_sqr / DUTY_100_PERCENT;
            duty_sqr = duty_sqr * duty_sqr;
            float re = 2 * LF * FPWM / duty_sqr;
            float rth = (RLINE + RSENSE) / (RLINE + RSENSE + re);

            // printf("re: %f rth: %f duty2: %f\n", re, rth, duty_sqr);
            vinput_applied[pwm_cntr] = (-vinput[pwm_cntr] - vline[pwm_cntr]) * rth;
#ifdef BIPOLAR_VERBOSE
            printf("dcm ");
#endif
            last_mode = 0;
            current_mode[pwm_cntr] = 0;
        }

#ifdef BIPOLAR_VERBOSE
        printf("applied: %f cntr: %d\n", vinput_applied[pwm_cntr], pwm_cntr);
#endif
    }
    else if ((input < 0) && (vline[pwm_cntr] > 0))    //cuadrante 2
    {
#ifdef BIPOLAR_VERBOSE
        printf("cuadrante 2 ");
#endif
        vinput_applied[pwm_cntr] = input - vline[pwm_cntr];
#ifdef BIPOLAR_VERBOSE
        printf("ccm ");
        printf("applied: %f cntr: %d\n", vinput_applied[pwm_cntr], pwm_cntr);
#endif
        last_mode = 1;
        current_mode[pwm_cntr] = 1000;
    }
    else if ((input < 0) && (vline[pwm_cntr] < 0))    //cuadrante 4
    {
#ifdef BIPOLAR_VERBOSE
        printf("cuadrante 4 ");
#endif
        vinput_applied[pwm_cntr] = input - vline[pwm_cntr];
#ifdef BIPOLAR_VERBOSE
        printf("ccm ");
        printf("applied: %f cntr: %d\n", vinput_applied[pwm_cntr], pwm_cntr);
#endif
        last_mode = 1;
        current_mode[pwm_cntr] = 1000;
    }
    else    //input == 0
    {
#ifdef BIPOLAR_VERBOSE
        printf("no input ");
#endif
        vinput_applied[pwm_cntr] = 0.0;
#ifdef BIPOLAR_VERBOSE
        printf("applied: %f cntr: %d\n", vinput_applied[pwm_cntr], pwm_cntr);
#endif
        last_mode = 0;
        current_mode[pwm_cntr] = 0;
    }

    voutput[pwm_cntr] = Plant_Out_Recursive(vinput_applied[pwm_cntr]);

    if (voutput[pwm_cntr] < 0)
    {
        voutput_adc[pwm_cntr] = Adc12BitsConvertion(-voutput[pwm_cntr]);
        // last_output_bipolar = -voutput_adc[pwm_cntr];    //cuando uso bipolar con gen signal bipolar
        last_output_bipolar = voutput_adc[pwm_cntr];    //cuando uso bipolar con gen signal
    }
    else
    {
        voutput_adc[pwm_cntr] = Adc12BitsConvertion(voutput[pwm_cntr]);
        last_output_bipolar = voutput_adc[pwm_cntr];
    }
        
    
    pwm_cntr++;
}


unsigned char GetCurrentMode (void)
{
    return last_mode;
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


void TestRecursiveDump (void)
{
    float b_vector [5] = { 0.48, 0.429, 0.373, 0.335, 0.283 };
    float a_vector [6] = { 1., 0.93, 0.858, 0.785, 0.709, 0.653 };
    float ins_vector [5] = { 0.0 };
    float outs_vector [6] = { 0.0 };
    recursive_filter_t filter_t;

    filter_t.b_params = b_vector;
    filter_t.a_params = a_vector;
    filter_t.b_size = 5;
    filter_t.a_size = 6;
    filter_t.last_inputs = ins_vector;
    filter_t.last_outputs = outs_vector;    
    Recursive_Filter_Float_Reset(&filter_t);

    unsigned short inputs [20] = { 000,   6,  15,  28,  42,  58,  76,  94, 114, 139,
                                   160, 182, 205, 227, 248, 269, 289, 308, 327, 346};

    float outputs[20] = { 0 };

    for (int i = 0; i < 20; i++)
    {
        outputs[i] = Recursive_Filter_Float(&filter_t, inputs[i]);
        printf("i: %02d in: %d out: %f\n", i, inputs[i], outputs[i]);
    }
}


void TestPRController (void)
{
    float calc = 0.0;

    Plant_Out_Recursive_Reset();
    
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < (2*SIZEOF_SIGNAL); i++)
        {
            calc = sin (3.1415 * i / SIZEOF_SIGNAL);
            calc = calc * 311;
            vline[i + (j * (2*SIZEOF_SIGNAL))] = calc;
            vinput[i + (j * (2*SIZEOF_SIGNAL))] = 350;
        }
    }

    //distorsiono entrada
    // for (int j = 0; j < HOW_MANY_CYCLES; j++)
    // {
    //     for (int i = 95; i < 145; i++)
    //     {
    //         vline[i + (j * 2*SIZEOF_SIGNAL)] = 295;
    //         vline[i + SIZEOF_SIGNAL + (j * 2*SIZEOF_SIGNAL)] = -295;
    //     }
    // }

    // ShowVectorFloat("\nVector line:\n", vline, VECTOR_LENGTH);
    // ShowVectorFloat("\nVector input:\n", vinput, VECTOR_LENGTH);    

    gen_signal_e sig_state = SIGNAL_RUNNING;
    short duty = 0;
    short ki_multiplier = KI_SIGNAL_PEAK_MULTIPLIER;

    GenSignalControlInit();
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        //primera parte de la senial
        GenSignalReset();
        sig_state = SIGNAL_RUNNING;
        duty = 0;

        unsigned int inner_calc = 0;
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            inner_calc = sin_half_cycle[i] * ki_multiplier * 5;
            inner_calc /= 10;
            inner_calc >>= 12;
            sig_state = GenSignal((unsigned short) inner_calc, ki_multiplier, &duty);
            // sig_state = GenSignal(last_output, ki_multiplier, &duty);            
            if (sig_state == SIGNAL_RUNNING)
            {
                //reviso que duty sea siempre positivo en esta parte
                if (duty < 0)
                    duty = 0;

                // HIGH_LEFT(duty);
                HIGH_BIPOLAR(duty);

                // printf("index: %d duty: %d\n", i, duty);
            }
            else
            {
                // HIGH_LEFT(0);
                HIGH_BIPOLAR(0);
            }
        
        }

        // segunda parte de la senial
        GenSignalReset();
        sig_state = SIGNAL_RUNNING;
        duty = 0;
        
        for (int i = SIZEOF_SIGNAL; i < 2*SIZEOF_SIGNAL; i++)
        {
            // sig_state = GenSignal(last_output, ki_multiplier, &duty);
            // printf("index: %d i: %d ",GetPwmCounter(), last_output_bipolar);
            sig_state = GenSignal(last_output_bipolar, ki_multiplier, &duty);
            // printf("duty: %d\n",duty);
            if (sig_state == SIGNAL_RUNNING)
            {
                //reviso que duty sea siempre positivo
                if (duty < 0)
                    duty = 0;
                
                // HIGH_RIGHT(duty);
                HIGH_BIPOLAR(-duty);                
            }
            else
            {
                // HIGH_RIGHT(0);
                HIGH_BIPOLAR(0);
            }
        
        }
        
    }

    short reference [VECTOR_LENGTH] = { 0 };
    unsigned int ref_calc = 0;
    for (int j = 0; j < HOW_MANY_CYCLES; j++)
    {
        for (int i = 0; i < SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_half_cycle[i] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * 2*SIZEOF_SIGNAL)] = (unsigned short) ref_calc;
        }

        for (int i = SIZEOF_SIGNAL; i < 2*SIZEOF_SIGNAL; i++)
        {
            ref_calc = sin_half_cycle[i - SIZEOF_SIGNAL] * ki_multiplier;
            ref_calc = ref_calc >> 12;
            reference[i + (j * 2*SIZEOF_SIGNAL)] = -(short) ref_calc;
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


    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    
    Vector_Float_To_File(file, "vline", vline, VECTOR_LENGTH);
    Vector_Float_To_File(file, "vinput", vinput, VECTOR_LENGTH);

    Vector_Short_To_File(file, "reference", reference, VECTOR_LENGTH);
    Vector_Short_To_File(file, "duty_bipolar", duty_bipolar, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "reference", reference, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "vinput", vinput, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "duty_high_left", duty_high_left, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "duty_high_right", duty_high_right, VECTOR_LENGTH);    

    Vector_Float_To_File(file, "vinput applied", vinput_applied, VECTOR_LENGTH);
    Vector_Float_To_File(file, "voutput getted", voutput, VECTOR_LENGTH);    

    Vector_UShort_To_File(file, "voutput_adc", voutput_adc, VECTOR_LENGTH);
    Vector_UShort_To_File(file, "current_mode", current_mode, VECTOR_LENGTH);    

    printf("\nRun by hand python3 simul_sinus_filter.py\n");    
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


