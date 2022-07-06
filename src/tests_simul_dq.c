//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### DQ_SIMULATIONS ########################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
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
unsigned short vinput[SIZEOF_SIGNAL] = { 0 };
float vinput_applied[SIZEOF_SIGNAL] = { 0 };
float voutput[SIZEOF_SIGNAL] = { 0 };
unsigned short voutput_adc[SIZEOF_SIGNAL] = { 0 };

short voltage_input [SIZEOF_SIGNAL] = { 0 };
short voltage_filtered [SIZEOF_SIGNAL] = { 0 };
short voltage_output [SIZEOF_SIGNAL] = { 0 };


// Module Functions to Test ----------------------------------------------------
void Create_Sine_UShort (unsigned short * vect,
                         unsigned short vect_len,
                         unsigned short freq,
                         unsigned short fsampling,
                         unsigned short peak,
                         unsigned short offset);

void Create_Sine_Short (short * vect,
                        unsigned short vect_len,
                        unsigned short freq,
                        unsigned short fsampling,
                        short peak,
                        short offset);                        

void IIR_Params_Float (unsigned short fc, unsigned short fsampling,
                       float * b_coeff, float * a_coeff);

void IIR_Filter_Coeff_Reset (iir_filter_st * iir_f);
void IIR_Filter_Reset (iir_filter_st * iir_f);
float IIR_Filter (iir_filter_st * iir_f, float in);

void Step_Response_UShort (iir_filter_st * iir_f, unsigned short * vect, unsigned short vect_len);

void Delay_90Degree_Short (short * vinput, short * voutput);
void Synchronous_Frame (short * vds, short * vqs, short * volts_e, short * theta, short * omega);

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
           
    // Create_Sine_UShort(voltage_input, total_samples, NOMINAL_FREQ, SAMPLING_FREQ,
    //                    2047, 2048);
    
    // ShowVectorUShort("\nVoltage signal:\n", voltage_input, total_samples);

    iir_filter_st filter_step;
    IIR_Filter_Coeff_Reset (&filter_step);
    IIR_Params_Float (NOMINAL_FREQ, SAMPLING_FREQ, &filter_step.b_coeff[0], &filter_step.a_coeff[0]);
    // Step_Response_UShort(&filter_step, vinput, SIZEOF_SIGNAL);

    Delay_90Degree_Short (voltage_input, voltage_output);

    short voltage_est [SIZEOF_SIGNAL] = { 0 };
    short theta_input [SIZEOF_SIGNAL] = { 0 };
    short omega_input [SIZEOF_SIGNAL] = { 0 };
    Synchronous_Frame (voltage_output, voltage_input, voltage_est, theta_input, omega_input);

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


void Create_Sine_Short (short * vect,
                        unsigned short vect_len,
                        unsigned short freq,
                        unsigned short fsampling,
                        short peak,
                        short offset)
{
    float calc = 0.0;
    int sym_cycle = fsampling / freq;
    
    for (int i = 0; i < vect_len; i++)
    {
        calc = sin(i*(6.28/sym_cycle));
        calc = calc * peak + offset;
        vect[i] = (short) calc;
    }
}


void Synchronous_Frame (short * vds, short * vqs, short * volts_e, short * theta, short * omega)
{
    short vde [SIZEOF_SIGNAL] = { 0 };
    short vqe [SIZEOF_SIGNAL] = { 0 };
    short pi_input [SIZEOF_SIGNAL] = { 0 };
    short pi_output [SIZEOF_SIGNAL] = { 0 };    
    const short wset = 314;

    int total_samples = CYCLES_NUM * SAMPLING_FREQ / NOMINAL_FREQ;
    float calc = 0.0;
    
    pid_data_obj_t pi_filter;
    pi_filter.kp = 10;
    pi_filter.ki = 10;
    PID_Flush_Errors (&pi_filter);

    // theorical theta
    int delta_theta = 4096 * NOMINAL_FREQ / SAMPLING_FREQ;
    // for (int i = 0; i < total_samples; i++)
    // {
    //     if (i)
    //         *(theta + i) += *(theta + i - 1) + delta_theta;
    //     else
    //         *(theta + 0) = delta_theta;
        
    //     if (*(theta + i) > 2047)
    //         *(theta + i) -= 4096;
    // }
    
    for (int i = 0; i < total_samples; i++)
    {
        //
        // DQ PLL
        //
        float radians = 0.0;
        radians = 3.1415 * ((float) *(theta + i)) / 2048;
        
        // estimate vde & vqe        
        calc = (float) *(vds + i) * cos(radians) + (float) *(vqs + i) * sin(radians);
        // printf("vds: %d cos: %f vqs: %d sin: %f vde: %f theta: %f\n",
        //        *(vds + i),
        //        cos(radians),
        //        *(vqs + i),
        //        sin(radians),
        //        calc,
        //        radians);
        
        *(vde + i) = (short) calc;
        calc = (float) -*(vds + i) * sin(radians) + (float) *(vqs + i) * cos(radians);
        *(vqe + i) = (short) calc;

        // new PI output
        *(pi_input + i) = *(vqe + i);        
        pi_filter.setpoint = *(pi_input + i);    //to set the error on correct value
        pi_filter.sample = 0;
        *(pi_output + i) = PI (&pi_filter);

        // estimate new omega
        *(omega + i) = *(pi_output + i) + wset;
        // *(omega + i) = *(pi_output + i);        

        // estimate next theta
        if (i < (total_samples - 1))
        {
            *(theta + i + 1) = *(theta + i) + *(omega + i) * 400 / wset;
            
            if ((*(theta + i) + 1) > 2047)
                *(theta + i + 1) -= 4096;
        }

        // save volts estimate
        *(volts_e + i) = *(vde + i);

        //
        // end of DQ PLL
        //

        
        
    }    // end of for loop

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return;
    }
    Vector_Short_To_File(file, "vqe", vqe, SIZEOF_SIGNAL);
    Vector_Short_To_File(file, "vde", vde, SIZEOF_SIGNAL);
    // Vector_Short_To_File(file, "pi_input", pi_input, SIZEOF_SIGNAL);
    Vector_Short_To_File(file, "pi_output", pi_output, SIZEOF_SIGNAL);
    Vector_Short_To_File(file, "omega", omega, SIZEOF_SIGNAL);
    Vector_Short_To_File(file, "theta", theta, SIZEOF_SIGNAL);
    // Vector_Short_To_File(file, "volts_e", volts_e, SIZEOF_SIGNAL);    

    printf("\nRun by hand python3 simul_outputs.py\n");
    
}


void Delay_90Degree_Short (short * vinput, short * voutput)
{
    float b [1] = {0.0157};
    float a [2] = {1.0, -0.9843};
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

    int total_samples = CYCLES_NUM * SAMPLING_FREQ / NOMINAL_FREQ;
    Create_Sine_Short(vinput, total_samples, NOMINAL_FREQ, SAMPLING_FREQ, 2047, 0);    
    
    for (int i = 0; i < total_samples; i++)
        *(voltage_filtered + i) = Recursive_Filter_Float(&filter_step, vinput[i]);

    /////////////////////////////////////////
    // Multiply -2, add to original vinput //
    /////////////////////////////////////////
    for (int i = 0; i < total_samples; i++)
        *(voutput + i) = (*(voltage_filtered + i)* -2) + *(vinput + i);
    
    // ShowVectorShort("\nVector output:\n", voutput, total_samples);

    ///////////////////////////
    // Backup Data to a file //
    ///////////////////////////
    // FILE * file = fopen("data.txt", "w");

    // if (file == NULL)
    // {
    //     printf("data file not created!\n");
    //     return;
    // }
    // Vector_Short_To_File(file, "filter_in", vinput, SIZEOF_SIGNAL);
    // Vector_Short_To_File(file, "filter_filtered", voltage_filtered, SIZEOF_SIGNAL);
    // Vector_Short_To_File(file, "delayed_out", voutput, SIZEOF_SIGNAL);    

    // printf("\nRun by hand python3 simul_outputs.py\n");
    
}


void Step_Response_UShort (iir_filter_st * iir_f, unsigned short * vect, unsigned short vect_len)
{
    float b [1] = {0.0157};
    float a [2] = {1.0, -0.9843};
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
    for (int i = 0; i < vect_len; i++)
        *(vect + i) = Recursive_Filter_Float(&filter_step, 4095);
    
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

    Vector_UShort_To_File(file, "filter_out", vect, SIZEOF_SIGNAL);

    printf("\nRun by hand python3 simul_outputs.py\n");
    
}


void IIR_Params_Float (unsigned short fc, unsigned short fsampling,
                       float * b_coeff, float * a_coeff)
{
    float calc = 0.0;

    calc = (float)fc*6.28 / (float)fsampling;
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


