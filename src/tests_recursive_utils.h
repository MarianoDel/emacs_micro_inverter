//----------------------------------------------------------
// #### SOME UTILS FUNCTIONS ###############################
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TESTS_RECURSIVE_UTILS.H ###############################
//----------------------------------------------------------

// Prevent Recursive Inclusion -------------------------------------------------
#ifndef __TESTS_RECURSIVE_UTILS_H_
#define __TESTS_RECURSIVE_UTILS_H_

// Includes to help defines ----------------------------------------------------

// Module Exported Types Constants and Macros ----------------------------------

// b vector are the zeros of the polinomial
// a vector are the poles, the recursive part of the filter
typedef struct {
    float * b_params;
    float * a_params;
    int b_size;
    int a_size;

    float * last_inputs;    //equal size as b
    float * last_outputs;    //equal size as a
    int inputs_cntr;
    
} recursive_filter_t;


// Module Exported Functions ---------------------------------------------------
float Recursive_Filter_Float (recursive_filter_t *, float);
void Recursive_Filter_Float_Show_Params (recursive_filter_t *);
void Recursive_Filter_Float_Reset (recursive_filter_t *);


#endif    /* __TESTS_RECURSIVE_UTILS_H_ */

//--- end of file ---//

