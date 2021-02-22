//----------------------------------------------------------
// #### SOME UTILS FUNCTIONS ###############################
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TESTS_RECURSIVE_UTILS.C ############################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "tests_recursive_utils.h"

#include <stdio.h>
#include <string.h>


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
float Recursive_Filter_Float (recursive_filter_t * filter, float new_input)
{
    int deep = 0;
    float output = 0.0;
    
    //load new_input on vector
    filter->last_inputs[0] = new_input;

    //check the deep of calc for the forward params
    if (filter->inputs_cntr < filter->b_size)
        deep = filter->inputs_cntr;
    else
        deep = filter->b_size;

    //forward params
    for (int i = 0; i < deep; i++)
    {
        output += filter->b_params[i] * filter->last_inputs[i];
        // printf("index: %d last_inputs: %f\n", i, filter->last_inputs[i]);
    }

    //check the deep of calc for recursive params
    if (filter->inputs_cntr < filter->a_size)
        deep = filter->inputs_cntr;
    else
        deep = filter->a_size;

    // printf("deep: %d counter: %d\n", deep, filter->inputs_cntr);
    
    //recursive params
    for (int i = 1; i < deep; i++)
    {
        output -= filter->a_params[i] * filter->last_outputs[i - 1];
        // printf("index: %d last_outputs: %f\n", i, filter->last_outputs[i - 1]);
    }

    // update the counter
    filter->inputs_cntr += 1;
    
    //fix next loop vectors
    // for (int i = 0; i < (filter->b_size - 1); i++)
    //     filter->last_inputs[i + 1] = filter->last_inputs[i];

    for (int i = (filter->b_size - 1); i > 0; i--)
        filter->last_inputs[i] = filter->last_inputs[i - 1];
    
    for (int i = (filter->a_size - 1); i > 0 ; i--)
        filter->last_outputs[i] = filter->last_outputs[i - 1];

    filter->last_outputs[0] = output;

    // printf("output: %f\n", output);
    return output;
}


void Recursive_Filter_Float_Show_Params (recursive_filter_t * filter)
{
    for (int i = filter->b_size - 1; i >= 0; i--)
        printf("b[%d]: %f ", i, filter->b_params[i]);

    printf("\n");
    for (int i = filter->a_size - 1; i >= 0; i--)
        printf("a[%d]: %f ", i, filter->a_params[i]);

    printf("\n");    
    
}


void Recursive_Filter_Float_Reset (recursive_filter_t * filter)
{
    for (int i = 0; i < filter->b_size; i++)
        filter->last_inputs[i] = 0.0;

    for (int i = 0; i < filter->a_size; i++)
        filter->last_outputs[i] = 0.0;

    filter->inputs_cntr = 0;
}

//--- end of file ---//

