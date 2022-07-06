//----------------------------------------------------------
// #### SOME UTILS FUNCTIONS ###############################
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TESTS_VECTOR_UTILS.C ###############################
//----------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "tests_vector_utils.h"

#include <stdio.h>
#include <string.h>


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
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


void Vector_UShort_To_File (FILE * f, char * v_name, unsigned short * v_data, int v_len)
{
    int len = 0;
    char str [100] = { 0 };

    fwrite("ushort ", 1, sizeof("ushort ") - 1, f);    
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


void Vector_Short_To_File (FILE * f, char * v_name, short * v_data, int v_len)
{
    int len = 0;
    char str [100] = { 0 };
    
    fwrite("short ", 1, sizeof("short ") - 1, f);
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


void Vector_Float_To_File (FILE * f, char * v_name, float * v_data, int v_len)
{
    int len = 0;
    char str [100] = { 0 };
    
    fwrite("float ", 1, sizeof("float ") - 1, f);
    len = strlen(v_name);    
    fwrite(v_name, 1, len, f);
    fwrite("\n", 1, sizeof("\n") - 1, f);
    
    for (int i = 0; i < v_len; i++)
    {
        len = sprintf(str, "%f ", v_data[i]);
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


void ShowVectorFloat (char * s_comment, float * f_vect, int size)
{
    printf(s_comment);
    for (int i = 0; i < size; i+=8)
        printf("index: %03d - %f %f %f %f %f %f %f %f\n",
               i,
               *(f_vect+i+0),
               *(f_vect+i+1),
               *(f_vect+i+2),
               *(f_vect+i+3),
               *(f_vect+i+4),
               *(f_vect+i+5),
               *(f_vect+i+6),
               *(f_vect+i+7));
    
}


void ShowVectorUShort (char * s_comment, unsigned short * int_vect, int size)
{
    printf(s_comment);
    for (int i = 0; i < size; i+=8)
        printf("index: %03d - %d %d %d %d %d %d %d %d\n",
               i,
               *(int_vect+i+0),
               *(int_vect+i+1),
               *(int_vect+i+2),
               *(int_vect+i+3),
               *(int_vect+i+4),
               *(int_vect+i+5),
               *(int_vect+i+6),
               *(int_vect+i+7));
    
}


void ShowVectorShort (char * s_comment, short * int_vect, int size)
{
    printf(s_comment);
    for (int i = 0; i < size; i+=8)
        printf("index: %03d - %d %d %d %d %d %d %d %d\n",
               i,
               *(int_vect+i+0),
               *(int_vect+i+1),
               *(int_vect+i+2),
               *(int_vect+i+3),
               *(int_vect+i+4),
               *(int_vect+i+5),
               *(int_vect+i+6),
               *(int_vect+i+7));
    
}


void ShowVectorInt (char * s_comment, int * int_vect, int size)
{
    printf(s_comment);
    for (int i = 0; i < size; i+=8)
        printf("index: %03d - %d %d %d %d %d %d %d %d\n",
               i,
               *(int_vect+i+0),
               *(int_vect+i+1),
               *(int_vect+i+2),
               *(int_vect+i+3),
               *(int_vect+i+4),
               *(int_vect+i+5),
               *(int_vect+i+6),
               *(int_vect+i+7));
    
}

//--- end of file ---//

