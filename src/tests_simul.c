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



#define SIZEOF_SIGNAL    1000

// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
float signal_output [SIZEOF_SIGNAL] = { 0.0 };

// Module Private Utils Functions ----------------------------------------------
    

// Module Private Functions to Test --------------------------------------------


// Module Functions ------------------------------------------------------------
int main(void)
{
    // float b [5] = {1.0, 1.1, 1.2, 1.3, 1.4};
    // float a [2] = {1.0, 1.1};    
    // float inputs [5] = { 0.0 };
    // float outputs [2] = { 0.0 };
    // recursive_filter_t f1;
    
    // f1.b_params = b;
    // f1.a_params = a;
    // f1.b_size = sizeof(b) / sizeof(float);
    // f1.a_size = sizeof(a) / sizeof(float);
    // f1.last_inputs = inputs;
    // f1.last_outputs = outputs;

    // Recursive_Filter_Float_Show_Params(&f1);
    // Recursive_Filter_Float_Reset(&f1);

    float b [1] = {0.02};
    float a [2] = {1.0, -0.98};    
    float inputs [1] = { 0.0 };
    float outputs [2] = { 0.0 };
    recursive_filter_t f1;
    
    f1.b_params = b;
    f1.a_params = a;
    f1.b_size = sizeof(b) / sizeof(float);
    f1.a_size = sizeof(a) / sizeof(float);
    f1.last_inputs = inputs;
    f1.last_outputs = outputs;

    Recursive_Filter_Float_Show_Params(&f1);
    Recursive_Filter_Float_Reset(&f1);
    
    for (int i = 0; i < SIZEOF_SIGNAL; i++)
        signal_output[i] = Recursive_Filter_Float(&f1, 4095.0);

    ShowVectorFloat("signal_output\n", signal_output, SIZEOF_SIGNAL);
    
    return 0;
}






//--- end of file ---//

