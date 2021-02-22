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
#include "dsp.h"

#include "tests_vector_utils.h"
#include "tests_recursive_utils.h"
#include <stdio.h>
#include <string.h>



#define SIZEOF_SIGNAL    40

// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
float signal_input [SIZEOF_SIGNAL] = { 0.0 };
float signal_output [SIZEOF_SIGNAL] = { 0.0 };

// Module Private Utils Functions ----------------------------------------------
    

// Module Private Functions to Test --------------------------------------------


// Module Functions ------------------------------------------------------------
#define B_SIZE    3
#define A_SIZE    4
float b_vector [B_SIZE] = { 0.00296 , 0.0101, 0.00276 };
float a_vector [A_SIZE] = { 1., -1.02798, 1.02185, -0.87036 };
// #define B_SIZE    1
// #define A_SIZE    3
// float b_vector [B_SIZE] = { 1. };
// float a_vector [A_SIZE] = { 1., -0.05, 0.05 };

float ins_vector [B_SIZE] = { 0.0 };
float outs_vector [A_SIZE] = { 0.0 };
recursive_filter_t f1;
int main(void)
{
    f1.b_params = b_vector;
    f1.a_params = a_vector;
    f1.b_size = B_SIZE;
    f1.a_size = A_SIZE;
    f1.last_inputs = ins_vector;
    f1.last_outputs = outs_vector;

    Recursive_Filter_Float_Show_Params(&f1);
    Recursive_Filter_Float_Reset(&f1);
    
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
        signal_input[i] = i;

    for (int i = 0; i < SIZEOF_SIGNAL; i++)
        signal_output[i] = Recursive_Filter_Float(&f1, signal_input[i]);
    
    ShowVectorFloat("signal_output\n", signal_output, SIZEOF_SIGNAL);
    
    return 0;
}






//--- end of file ---//

