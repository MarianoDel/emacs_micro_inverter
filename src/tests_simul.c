//----------------------------------------------------------
// #### TEST THE LIMITS FUNCTIONS - Custom Board ###########
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TEST_SIMUL.C #######################################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "parameters.h"
#include "dsp.h"

#include <stdio.h>
#include <string.h>





// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
#define DMX_TICK_MS    25

#define SIMUL_LENGTH_MS    2000
unsigned short pwm1 [SIMUL_LENGTH_MS] = { 0 };
unsigned short pwm2 [SIMUL_LENGTH_MS] = { 0 };
unsigned short pwm3 [SIMUL_LENGTH_MS] = { 0 };
unsigned short pwm4 [SIMUL_LENGTH_MS] = { 0 };
unsigned short pwm5 [SIMUL_LENGTH_MS] = { 0 };
unsigned short pwm6 [SIMUL_LENGTH_MS] = { 0 };

unsigned short ch1_pwm = 0;
unsigned short ch2_pwm = 0;
unsigned short ch3_pwm = 0;
unsigned short ch4_pwm = 0;
unsigned short ch5_pwm = 0;
unsigned short ch6_pwm = 0;

unsigned char dmx_data [SIMUL_LENGTH_MS] = { 0 };

parameters_typedef mem_conf;

ma16_u16_data_obj_t st_sp1;
ma16_u16_data_obj_t st_sp2;
ma16_u16_data_obj_t st_sp3;
ma16_u16_data_obj_t st_sp4;
ma16_u16_data_obj_t st_sp5;
ma16_u16_data_obj_t st_sp6;


// Module Private Utils Functions ----------------------------------------------
void Print_Array_Uchar (unsigned char * array, int size, int length);
void Print_Array_Ushort (unsigned short * array, int size, int length);
void Update_Channels_Points (void);
void Update_Channels_Points13 (void);
void Update_Channels_Points46 (void);
void PWM_Update_CH1 (unsigned short pwm);
void PWM_Update_CH2 (unsigned short pwm);
void PWM_Update_CH3 (unsigned short pwm);
void PWM_Update_CH4 (unsigned short pwm);
void PWM_Update_CH5 (unsigned short pwm);
void PWM_Update_CH6 (unsigned short pwm);
void Vector_UShort_To_File (FILE * f, char * v_name, unsigned short * v_data, int v_len);
void Vector_UChar_To_File (FILE * f, char * v_name, unsigned char * v_data, int v_len);
void Variable_UShort_To_File (FILE * f, char * v_name, unsigned short data);
void Variable_UChar_To_File (FILE * f, char * v_name, unsigned char data);
    

// Module Private Functions to Test --------------------------------------------
void CheckFiltersAndOffsets_SM(volatile unsigned char *);
void PWM_Set_PwrCtrl (unsigned char * ch_dmx_val, unsigned char chnls_qtty, unsigned short max_power);
unsigned short PWM_Map_From_Dmx (unsigned char dmx_val);


// Module Functions ------------------------------------------------------------
int main(void)
{
    //each tick is 1ms
    //each dmx new data is 25ms

    //reset filters
    MA16_U16Circular_Reset (&st_sp1);
    MA16_U16Circular_Reset (&st_sp2);
    MA16_U16Circular_Reset (&st_sp3);
    MA16_U16Circular_Reset (&st_sp4);
    MA16_U16Circular_Reset (&st_sp5);
    MA16_U16Circular_Reset (&st_sp6);

    //set number of channels
    mem_conf.dmx_channel_quantity = 6;
    
    //set the individual current limits
    mem_conf.max_current_channels[0] = 127;
    mem_conf.max_current_channels[1] = 127;
    mem_conf.max_current_channels[2] = 127;
    mem_conf.max_current_channels[3] = 0;
    mem_conf.max_current_channels[4] = 0;
    mem_conf.max_current_channels[5] = 0;

    //set the total current limit
    mem_conf.max_power = 255;

    //set the dmx data
    int cntr = 0;
    unsigned char data = 0;
    for (int i = 0; i < SIMUL_LENGTH_MS; i++)
    {
        dmx_data[i] = data;
        
        if (cntr < DMX_TICK_MS)
            cntr++;
        else
        {
            cntr = 0;
            if (data < 255)
                data++;
        }
    }

    unsigned char ch_dmx_val[6] = { 0 };
    unsigned char loop = 0;
    for (int i = 0; i < SIMUL_LENGTH_MS; i++)
    {
        for (int j = 0; j < 6; j++)
            ch_dmx_val[j] = dmx_data[i];
        
        CheckFiltersAndOffsets_SM (ch_dmx_val);
    }

    // printf("input dmx data\n");    
    // Print_Array_Uchar(dmx_data, SIMUL_LENGTH_MS, -100);

    // printf("simul pwm1\n");
    // Print_Array_Ushort(pwm1, SIMUL_LENGTH_MS, -100);

    // printf("simul pwm2\n");    
    // Print_Array_Ushort(pwm2, SIMUL_LENGTH_MS, 64);

    FILE * file = fopen("data.txt", "w");

    if (file == NULL)
    {
        printf("data file not created!\n");
        return 0;
    }

    Vector_UChar_To_File(file, "dmx_data", dmx_data, SIMUL_LENGTH_MS);

    Vector_UShort_To_File(file, "pwm1", pwm1, SIMUL_LENGTH_MS);
    // Vector_UShort_To_File(file, "pwm2", pwm2, SIMUL_LENGTH_MS);
    // Vector_UShort_To_File(file, "pwm3", pwm3, SIMUL_LENGTH_MS);
    // Vector_UShort_To_File(file, "pwm4", pwm4, SIMUL_LENGTH_MS);
    // Vector_UShort_To_File(file, "pwm5", pwm5, SIMUL_LENGTH_MS);
    // Vector_UShort_To_File(file, "pwm6", pwm6, SIMUL_LENGTH_MS);

    Variable_UShort_To_File(file, "max_power", mem_conf.max_power);
    
    fclose(file);

    
    
    return 0;
}


#define CH1_VAL_OFFSET    0
#define CH2_VAL_OFFSET    1
#define CH3_VAL_OFFSET    2
#define CH4_VAL_OFFSET    3
#define CH5_VAL_OFFSET    4
#define CH6_VAL_OFFSET    5

typedef enum {
    FILTERS_BKP_CHANNELS,
    FILTERS_LIMIT_EACH_CHANNEL,
    FILTERS_LIMIT_ALL_CHANNELS,
    FILTERS_OUTPUTS_CH1_CH3,
    FILTERS_OUTPUTS_CH4_CH6
    
} filters_and_offsets_e;

filters_and_offsets_e filters_sm = FILTERS_BKP_CHANNELS;
unsigned char limit_output [6] = { 0 };
void CheckFiltersAndOffsets_SM (volatile unsigned char * ch_dmx_val)
{
    unsigned short calc = 0;    
    
    switch (filters_sm)
    {
    case FILTERS_BKP_CHANNELS:
        limit_output[0] = *(ch_dmx_val + 0);
        limit_output[1] = *(ch_dmx_val + 1);
        limit_output[2] = *(ch_dmx_val + 2);
        limit_output[3] = *(ch_dmx_val + 3);
        limit_output[4] = *(ch_dmx_val + 4);
        limit_output[5] = *(ch_dmx_val + 5);

        Update_Channels_Points();
        filters_sm++;
        break;

    case FILTERS_LIMIT_EACH_CHANNEL:
        calc = limit_output[0] * mem_conf.max_current_channels[0];
        calc >>= 8;
        limit_output[0] = (unsigned char) calc;

        calc = limit_output[1] * mem_conf.max_current_channels[1];
        calc >>= 8;
        limit_output[1] = (unsigned char) calc;

        calc = limit_output[2] * mem_conf.max_current_channels[2];
        calc >>= 8;
        limit_output[2] = (unsigned char) calc;

        calc = limit_output[3] * mem_conf.max_current_channels[3];
        calc >>= 8;
        limit_output[3] = (unsigned char) calc;

        calc = limit_output[4] * mem_conf.max_current_channels[4];
        calc >>= 8;
        limit_output[4] = (unsigned char) calc;

        calc = limit_output[5] * mem_conf.max_current_channels[5];
        calc >>= 8;
        limit_output[5] = (unsigned char) calc;

        Update_Channels_Points();        
        filters_sm++;
        break;

    case FILTERS_LIMIT_ALL_CHANNELS:
        PWM_Set_PwrCtrl(limit_output,
                        mem_conf.dmx_channel_quantity,
                        mem_conf.max_power);

        Update_Channels_Points();        
        filters_sm++;
        break;

    case FILTERS_OUTPUTS_CH1_CH3:
        // channel 1
        ch1_pwm = MA16_U16Circular (
            &st_sp1,
            PWM_Map_From_Dmx(*(limit_output + CH1_VAL_OFFSET))
            );
        PWM_Update_CH1(ch1_pwm);

        // channel 2
        ch2_pwm = MA16_U16Circular (
            &st_sp2,
            PWM_Map_From_Dmx(*(limit_output + CH2_VAL_OFFSET))
            );
        PWM_Update_CH2(ch2_pwm);

        // channel 3
        ch3_pwm = MA16_U16Circular (
            &st_sp3,
            PWM_Map_From_Dmx(*(limit_output + CH3_VAL_OFFSET))
            );
        PWM_Update_CH3(ch3_pwm);

        Update_Channels_Points46();
        filters_sm++;
        break;

    case FILTERS_OUTPUTS_CH4_CH6:
        // channel 4
        ch4_pwm = MA16_U16Circular (
            &st_sp4,
            PWM_Map_From_Dmx(*(limit_output + CH4_VAL_OFFSET))
            );
        PWM_Update_CH4(ch4_pwm);

        // channel 5
        ch5_pwm = MA16_U16Circular (
            &st_sp5,
            PWM_Map_From_Dmx(*(limit_output + CH5_VAL_OFFSET))
            );
        PWM_Update_CH5(ch5_pwm);

        // channel 6
        ch6_pwm = MA16_U16Circular (
            &st_sp6,
            PWM_Map_From_Dmx(*(limit_output + CH6_VAL_OFFSET))
            );
        PWM_Update_CH6(ch6_pwm);

        Update_Channels_Points13();        
        filters_sm = FILTERS_BKP_CHANNELS;
        break;
        
    default:
        filters_sm = FILTERS_BKP_CHANNELS;
        break;
    }
}


void PWM_Set_PwrCtrl (unsigned char * ch_dmx_val, unsigned char chnls_qtty, unsigned short max_power)
{
    unsigned short total_dmx = 0;

    //cuantos en total
    for (unsigned char i = 0; i < chnls_qtty; i++)
        total_dmx += *(ch_dmx_val + i);

    if (total_dmx > max_power)
    {
        unsigned int new = 0;
        for (unsigned char i = 0; i < chnls_qtty; i++)
        {
            // si el canal tiene algo
            if (*(ch_dmx_val + i))
            {
                new = *(ch_dmx_val + i) * max_power;
                new = new / total_dmx;

                // no dejo que se apaguen los canales
                if (new)
                    *(ch_dmx_val + i) = (unsigned char) new;
                else
                    *(ch_dmx_val + i) = 1;
                
            }
        }
    }
}


#define DUTY_100_PERCENT    4000
unsigned short PWM_Map_From_Dmx (unsigned char dmx_val)
{
    unsigned int pwm = 0;

#if (DUTY_100_PERCENT == 1000)    
    if (dmx_val)
    {
        pwm = dmx_val * 391;
        pwm = pwm / 100;
        pwm += 4;
    }
#endif
#if (DUTY_100_PERCENT == 4000)
    if (dmx_val)
    {
        pwm = dmx_val * 157;
        pwm = pwm / 10;
        pwm += 13;
    }
#endif

    return (unsigned short) pwm;

}



/////////////////////
// Utils Functions //
/////////////////////
void Print_Array_Uchar (unsigned char * array, int size, int length)
{
    if (length > 0)    //print from the beginning
    {
        if (length < size)
        {
            for (int i = 0; i < length; i+= 8)
            {
                printf("seq: %4d\t %3d %3d %3d %3d %3d %3d %3d %3d\n",
                       i,
                       *(array + i + 0),
                       *(array + i + 1),
                       *(array + i + 2),
                       *(array + i + 3),
                       *(array + i + 4),
                       *(array + i + 5),
                       *(array + i + 6),
                       *(array + i + 7));
            }
        }
    }
    else if (length < 0)    //print the last part of array
    {
        int len = -length;
        
        if (len < size)
        {
            for (int i = size - len; i < size; i+= 8)
            {
                printf("seq: %4d\t %3d %3d %3d %3d %3d %3d %3d %3d\n",
                       i,
                       *(array + i + 0),
                       *(array + i + 1),
                       *(array + i + 2),
                       *(array + i + 3),
                       *(array + i + 4),
                       *(array + i + 5),
                       *(array + i + 6),
                       *(array + i + 7));
            }
        }
        else
            printf("isnt less\n");
    }
    else    //print all array
    {
        printf("not implemented yet!\n");
    }
}


void Print_Array_Ushort (unsigned short * array, int size, int length)
{
    if (length > 0)    //print from the beginning
    {
        if (length < size)
        {
            for (int i = 0; i < length; i+= 8)
            {
                printf("seq: %4d\t %3d %3d %3d %3d %3d %3d %3d %3d\n",
                       i,
                       *(array + i + 0),
                       *(array + i + 1),
                       *(array + i + 2),
                       *(array + i + 3),
                       *(array + i + 4),
                       *(array + i + 5),
                       *(array + i + 6),
                       *(array + i + 7));
            }
        }
    }
    else if (length < 0)    //print the last part of array
    {
        int len = -length;
        
        if (len < size)
        {
            for (int i = size - len; i < size; i+= 8)
            {
                printf("seq: %4d\t %3d %3d %3d %3d %3d %3d %3d %3d\n",
                       i,
                       *(array + i + 0),
                       *(array + i + 1),
                       *(array + i + 2),
                       *(array + i + 3),
                       *(array + i + 4),
                       *(array + i + 5),
                       *(array + i + 6),
                       *(array + i + 7));
            }
        }
        else
            printf("isnt less\n");
    }
    else    //print all array
    {
        printf("not implemented yet!\n");
    }
}


void Update_Channels_Points (void)
{
    PWM_Update_CH1(ch1_pwm);
    PWM_Update_CH2(ch2_pwm);
    PWM_Update_CH3(ch3_pwm);
    PWM_Update_CH4(ch4_pwm);
    PWM_Update_CH5(ch5_pwm);
    PWM_Update_CH6(ch6_pwm);
}

void Update_Channels_Points13 (void)
{
    PWM_Update_CH1(ch1_pwm);
    PWM_Update_CH2(ch2_pwm);
    PWM_Update_CH3(ch3_pwm);
}

void Update_Channels_Points46 (void)
{
    PWM_Update_CH4(ch4_pwm);
    PWM_Update_CH5(ch5_pwm);
    PWM_Update_CH6(ch6_pwm);
}

int index_pwm1 = 0;
void PWM_Update_CH1 (unsigned short pwm)
{
    pwm1[index_pwm1] = pwm;
    if (index_pwm1 < SIMUL_LENGTH_MS)
        index_pwm1++;
}

int index_pwm2 = 0;
void PWM_Update_CH2 (unsigned short pwm)
{
    pwm2[index_pwm2] = pwm;
    if (index_pwm2 < SIMUL_LENGTH_MS)
        index_pwm2++;
}

int index_pwm3 = 0;
void PWM_Update_CH3 (unsigned short pwm)
{
    pwm3[index_pwm3] = pwm;
    if (index_pwm3 < SIMUL_LENGTH_MS)
        index_pwm3++;
}

int index_pwm4 = 0;
void PWM_Update_CH4 (unsigned short pwm)
{
    pwm4[index_pwm4] = pwm;
    if (index_pwm4 < SIMUL_LENGTH_MS)
        index_pwm4++;
}

int index_pwm5 = 0;
void PWM_Update_CH5 (unsigned short pwm)
{
    pwm5[index_pwm5] = pwm;
    if (index_pwm5 < SIMUL_LENGTH_MS)
        index_pwm5++;
}

int index_pwm6 = 0;
void PWM_Update_CH6 (unsigned short pwm)
{
    pwm6[index_pwm6] = pwm;
    if (index_pwm6 < SIMUL_LENGTH_MS)
        index_pwm6++;
}


void Vector_UShort_To_File (FILE * f, char * v_name, unsigned short * v_data, int v_len)
{
    int len = 0;
    char str [100] = { 0 };
    
    len = strlen(v_name);    
    fwrite(v_name, 1, len, f);
    fwrite("\n", 1, sizeof("\n") - 1, f);
    
    for (int i = 0; i < v_len; i++)
    {
        len = sprintf(str, "%d ", v_data[i]);
        fwrite(str, 1, len, f);
    }
    fwrite("\n", 1, sizeof("\n") - 1, f);
    
}


void Vector_UChar_To_File (FILE * f, char * v_name, unsigned char * v_data, int v_len)
{
    int len = 0;
    char str [100] = { 0 };
    
    len = strlen(v_name);    
    fwrite(v_name, 1, len, f);
    fwrite("\n", 1, sizeof("\n") - 1, f);
    
    for (int i = 0; i < v_len; i++)
    {
        len = sprintf(str, "%d ", v_data[i]);
        fwrite(str, 1, len, f);
    }
    fwrite("\n", 1, sizeof("\n") - 1, f);
    
}


void Variable_UShort_To_File (FILE * f, char * v_name, unsigned short data)
{
    int len = 0;
    char str [100] = { 0 };
    
    len = strlen(v_name);    
    fwrite(v_name, 1, len, f);
    fwrite("\n", 1, sizeof("\n") - 1, f);

    len = sprintf(str, "%d\n", data);
    fwrite(str, 1, len, f);
    
}


void Variable_UChar_To_File (FILE * f, char * v_name, unsigned char data)
{
    int len = 0;
    char str [100] = { 0 };
    
    len = strlen(v_name);    
    fwrite(v_name, 1, len, f);
    fwrite("\n", 1, sizeof("\n") - 1, f);

    len = sprintf(str, "%d\n", data);
    fwrite(str, 1, len, f);
    
}

//--- end of file ---//

