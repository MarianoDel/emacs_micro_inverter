//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### DQ_SIMULATIONS ########################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
// #include "gen_signal.h"
// #include "pwm_defs.h"
#include "dsp.h"
#include "tests_vector_utils.h"
#include "tests_recursive_utils.h"

#include <stdio.h>
#include <math.h>

// Types Constants and Macros --------------------------------------------------
#define CYCLES_NUM    10
#define SAMPLING_FREQ    20000
#define NOMINAL_FREQ    50

#define SIZEOF_SIGNAL    (CYCLES_NUM * SAMPLING_FREQ / NOMINAL_FREQ)

#define SIZEOF_FILTER_DEPTH    10
typedef struct {
    float b_coeff [SIZEOF_FILTER_DEPTH];
    float a_coeff [SIZEOF_FILTER_DEPTH];
    float input_z [SIZEOF_FILTER_DEPTH];
    float output_z [SIZEOF_FILTER_DEPTH];
    
} iir_filter_st;

// Externals -------------------------------------------------------------------
extern unsigned short sin_half_cycle [];

// Globals ---------------------------------------------------------------------
unsigned short duty_high_left [SIZEOF_SIGNAL] = { 0 };
unsigned short duty_high_right [SIZEOF_SIGNAL] = { 0 };
unsigned short vinput[SIZEOF_SIGNAL] = { 0 };
float vinput_applied[SIZEOF_SIGNAL] = { 0 };
float voutput[SIZEOF_SIGNAL] = { 0 };
unsigned short voutput_adc[SIZEOF_SIGNAL] = { 0 };
unsigned short last_output = 0;
unsigned short d_dump [SIZEOF_SIGNAL] = { 0 };
unsigned short voltage_input [SIZEOF_SIGNAL] = { 0 };


// Module Functions to Test ----------------------------------------------------
void Create_Sine_UShort (unsigned short * vect,
                         unsigned short vect_len,
                         unsigned short freq,
                         unsigned short fsampling,
                         unsigned short peak,
                         unsigned short offset);

void IIR_Params_Float (unsigned short fc, unsigned short fsampling,
                       float * b_coeff, float * a_coeff);

void IIR_Filter_Coeff_Reset (iir_filter_st * iir_f);
void IIR_Filter_Reset (iir_filter_st * iir_f);
float IIR_Filter (iir_filter_st * iir_f, float in);

void Step_Response_UShort (iir_filter_st * iir_f, unsigned short * vect, unsigned short vect_len);

void TEST_Gen_Signal_Module (void);

float Plant_Out (float);
void Plant_Step_Response (void);
void Plant_Step_Response_Duty (void);

unsigned short Adc12BitsConvertion (float );


// Module Functions ------------------------------------------------------------
int main (int argc, char *argv[])
{
    printf("\nsignal simulation fnominal: %d cycles: %d fsampling: %d\n",
           NOMINAL_FREQ,
           CYCLES_NUM,
           SAMPLING_FREQ);

    int total_samples = CYCLES_NUM * SAMPLING_FREQ / NOMINAL_FREQ;
    int samples_per_cycle = SAMPLING_FREQ / NOMINAL_FREQ;
    printf("total samples: %d samples per cycle: %d SIZEOF_SIGNAL: %d\n",
           total_samples,
           samples_per_cycle,
           SIZEOF_SIGNAL);
           
    Create_Sine_UShort(voltage_input, total_samples, NOMINAL_FREQ, SAMPLING_FREQ,
                       2047, 2048);
    
    // ShowVectorUShort("\nVoltage signal:\n", voltage_input, total_samples);

    iir_filter_st filter_step;
    IIR_Filter_Coeff_Reset (&filter_step);
    IIR_Params_Float (NOMINAL_FREQ, SAMPLING_FREQ, &filter_step.b_coeff[0], &filter_step.a_coeff[0]);

    Step_Response_UShort(&filter_step, vinput, SIZEOF_SIGNAL);
    
//     printf("Simple module tests\n");
//     TEST_Dsp_Module();
    
//     TEST_Gen_Signal_Module();

//     printf("Start of Analog simulations...\n");
//     //pruebo un step de la planta
//     // Plant_Step_Response();

//     //pruebo un step de la planta pero con duty y vinput
//     //la tension de entrada es tan alta que incluso con duty_max = 10000
//     //tengo errores del 2%
//     // Plant_Step_Response_Duty();

    
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

    return 0;
}


void Create_Sine_UShort (unsigned short * vect,
                         unsigned short vect_len,
                         unsigned short freq,
                         unsigned short fsampling,
                         unsigned short peak,
                         unsigned short offset)
{
    float calc = 0.0;
    int sym_cycle = fsampling / freq;
    
    for (int i = 0; i < vect_len; i++)
    {
        calc = sin(i*(6.28/sym_cycle));
        calc = calc * peak + offset;
        vect[i] = (unsigned short) calc;
    }
}




// //b[3]: [0.] b[2]: [0.02032562] b[1]: [0.0624813] b[0]: [0.01978482]
// //a[3]: 1.0 a[2]: 0.011881459485754697 a[1]: 0.014346828516393684 a[0]: -0.9474935161284341
// float output = 0.0;
// float output_z1 = 0.0;
// float output_z2 = 0.0;
// float output_z3 = 0.0;
// float input_z1 = 0.0;
// float input_z2 = 0.0;
// float input_z3 = 0.0;

// unsigned char cntr_plant = 0;
// float Plant_Out (float in)
// {
//     float output_b = 0.0;
//     float output_a = 0.0;
    
//     // output = 0. * input + 0.02032 * input_z1 + 0.06248 * input_z2 + 0.01978 * input_z3
//     //     - 0.01188 * output_z1 - 0.01434 * output_z2 + 0.94749 * output_z3;

//     if (cntr_plant > 2)
//     {
//         output_b = 0. * in + 0.02032 * input_z1 + 0.06248 * input_z2 + 0.01978 * input_z3;
//         output_a = 0.01188 * output_z1 + 0.01434 * output_z2 - 0.94749 * output_z3;

//         output = output_b - output_a;

//         input_z3 = input_z2;
//         input_z2 = input_z1;        
//         input_z1 = in;

//         output_z3 = output_z2;
//         output_z2 = output_z1;        
//         output_z1 = output;
        
//     }
//     else if (cntr_plant > 1)
//     {
//         output_b = 0. * in + 0.02032 * input_z1 + 0.06248 * input_z2;
//         output_a = 0.01188 * output_z1 + 0.01434 * output_z2;
//         output = output_b - output_a;

//         input_z2 = input_z1;
//         input_z1 = in;

//         output_z2 = output_z1;
//         output_z1 = output;
//     }
//     else if (cntr_plant > 0)
//     {
//         output_b = 0. * in + 0.02032 * input_z1;
//         output_a = 0.01188 * output_z1;
//         output = output_b - output_a;

//         input_z2 = input_z1;
//         input_z1 = in;

//         output_z2 = output_z1;
//         output_z1 = output;
//     }
//     else
//     {
//         output = 0. * in;
        
//         input_z1 = in;

//         output_z1 = output;
//     }

//     cntr_plant++;

//     return output;
// }


void Step_Response_UShort (iir_filter_st * iir_f, unsigned short * vect, unsigned short vect_len)
{
    float b [1] = {0.0025};
    float a [2] = {1.0, -0.9975};
    float ins_vector [1] = { 0.0 };
    float outs_vector [2] = { 0.0 };
    
    recursive_filter_t filter_step;
    filter_step.b_size= 1;
    filter_step.b_params = b;
    filter_step.a_size = 2;
    filter_step.a_params = a;
    filter_step.last_inputs = ins_vector;
    filter_step.last_outputs = outs_vector;    
    

    Recursive_Filter_Float_Reset(&filter_step);
    Recursive_Filter_Float_Show_Params(&filter_step);
    // printf("\nIIR Step Response length: %d\n", vect_len);

    // IIR_Filter_Reset(iir_f);

    // float out = 0.0;
    for (int i = 0; i < vect_len; i++)
    {
        // out = IIR_Filter(iir_f, 1.0);
        *(vect + i) = Recursive_Filter_Float(&filter_step, 4095);
        // *(vect + i) = IIR_Filter(iir_f, 1.0);        
    }
    
    
    ShowVectorUShort("\nVector output:\n", vect, vect_len);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }

    
    // Vector_Float_To_File(file, "vline", vline, VECTOR_LENGTH);
    // Vector_Float_To_File(file, "vinput", vinput, VECTOR_LENGTH);

    // Vector_Short_To_File(file, "reference", reference, VECTOR_LENGTH);
    // Vector_Short_To_File(file, "duty_bipolar", duty_bipolar, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "reference", reference, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "vinput", vinput, VECTOR_LENGTH);    
    // Vector_UShort_To_File(file, "duty_high_left", duty_high_left, VECTOR_LENGTH);
    // Vector_UShort_To_File(file, "duty_high_right", duty_high_right, VECTOR_LENGTH);    

    // Vector_Float_To_File(file, "vinput applied", vinput_applied, VECTOR_LENGTH);
    // Vector_Float_To_File(file, "voutput getted", voutput, VECTOR_LENGTH);    

    Vector_UShort_To_File(file, "filter out", vect, SIZEOF_SIGNAL);

    printf("\nRun by hand python3 simul_outputs.py\n");
    
}


void IIR_Params_Float (unsigned short fc, unsigned short fsampling,
                       float * b_coeff, float * a_coeff)
{
    float calc = 0.0;

    calc = (float)fc / (float)fsampling;
    printf("b coeff: %f a0: 1 a1: %f\n", calc, 1 - calc);
    b_coeff[0] = calc;
    a_coeff[0] = 1.0;
    a_coeff[1] = 1 - calc;
}


float IIR_Filter (iir_filter_st * iir_f, float in)
{
    int i = 0;
    float b_calc = 0.0;
    float a_calc = 0.0;
    
    // fix inputs for calcs
    for (i = 0; i < (SIZEOF_FILTER_DEPTH - 2); i++)
        iir_f->input_z[i + 1] = iir_f->input_z[i];
    
    iir_f->input_z[0] = in;
    ShowVectorFloat("\nin\n", iir_f->input_z, SIZEOF_FILTER_DEPTH);

    // for (i = 0; i < (SIZEOF_FILTER_DEPTH - 1); i++)
    //     b_calc += iir_f->b_coeff[i] * iir_f->input_z[i];

    // // fix outputs for calcs
    // for (i = 0; i < (SIZEOF_FILTER_DEPTH - 2); i++)
    //     iir_f->output_z[i + 1] = iir_f->output_z[i];
    
    // ShowVectorFloat("\nout\n", iir_f->output_z, SIZEOF_FILTER_DEPTH);    
    
    // for (i = 0; i < (SIZEOF_FILTER_DEPTH - 2); i++)    //a0 not used
    //     a_calc += iir_f->a_coeff[i+1] * iir_f->output_z[i];

    printf("b: %d a: %d\n", b_calc, a_calc);
    return (b_calc - a_calc);

}


void IIR_Filter_Reset (iir_filter_st * iir_f)
{
    for (int i = 0; i < SIZEOF_FILTER_DEPTH; i++)
    {
        iir_f->input_z[i] = 0.0;
        iir_f->output_z[i] = 0.0;
    }
}


void IIR_Filter_Coeff_Reset (iir_filter_st * iir_f)
{
    for (int i = 0; i < SIZEOF_FILTER_DEPTH; i++)
    {
        iir_f->b_coeff[i] = 0.0;
        iir_f->a_coeff[i] = 0.0;
    }
}

// void Plant_Step_Response_Duty (void)
// {
//     printf("\nPlant Step Response with duty and vinput\n");

//     unsigned short d = 28;
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         vinput[i] = 350;
//     }

//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//     {
//         vinput_applied[i] = vinput[i] * d;
//         vinput_applied[i] = vinput_applied[i] / DUTY_100_PERCENT;
//         voutput[i] = Plant_Out(vinput_applied[i]);
//     }
    

//     ShowVectorFloat("\nVector voltage input applied:\n", vinput_applied, SIZEOF_SIGNAL);
//     ShowVectorFloat("\nVector plant output:\n", voutput, SIZEOF_SIGNAL);
    
//     unsigned short adc_out [SIZEOF_SIGNAL] = { 0 };
//     for (unsigned char i = 0; i < SIZEOF_SIGNAL; i++)
//         adc_out[i] = Adc12BitsConvertion(voutput[i]);

//     ShowVectorUShort("\nVector plant output ADC:\n", adc_out, SIZEOF_SIGNAL);
    
// }


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


//--- end of file ---//


