//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS_OK.C ############################
//---------------------------------------------

#include "tests_ok.h"
#include <stdio.h>

// Module Functions ------------------------------------------------------------
void PrintOK (void)
{
    PrintGreen("OK\n");
}


void PrintERR (void)
{
    PrintRed("ERR\n");
}


void PrintRed (char * msg)
{
    printf("\033[0;31m");    //red
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBoldRed (char * msg)
{
    printf("\033[1;31m");    //bold red
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintGreen (char * msg)
{
    printf("\033[0;32m");    //green
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBoldGreen (char * msg)
{
    printf("\033[1;32m");    //bold green
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintYellow (char * msg)
{
    printf("\033[0;33m");    //yellow
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBoldYellow (char * msg)
{
    printf("\033[1;33m");    //bold yellow
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBlue (char * msg)
{
    printf("\033[0;34m");    //blue
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBoldBlue (char * msg)
{
    printf("\033[1;34m");    //bold blue
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintMagenta (char * msg)
{
    printf("\033[0;35m");    //magenta
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBoldMagenta (char * msg)
{
    printf("\033[1;35m");    //bold magenta
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintCyan (char * msg)
{
    printf("\033[0;36m");    //cyan
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBoldCyan (char * msg)
{
    printf("\033[1;36m");    //bold cyan
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintWhite (char * msg)
{
    printf("\033[0;37m");    //white
    printf("%s", msg);
    printf("\033[0m");    //reset
}


void PrintBoldWhite (char * msg)
{
    printf("\033[1;37m");    //bold white
    printf("%s", msg);
    printf("\033[0m");    //reset
}


//--- end of file ---//


