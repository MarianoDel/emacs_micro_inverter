//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS.C ###############################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------

#include <stdio.h>
// #include <math.h>

// Types Constants and Macros --------------------------------------------------


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Module Functions to Test ----------------------------------------------------
void Test_Functions (void);


// Module Auxiliary Functions --------------------------------------------------
void PrintOK (void);
void PrintERR (void);


// Module Functions ------------------------------------------------------------
int main (int argc, char *argv[])
{
    Test_Functions ();

    return 0;
}


void Test_Functions (void)
{
    printf("tested ok!\n");
    PrintOK();
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


