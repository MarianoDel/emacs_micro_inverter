//----------------------------------------------------------
// #### SOME UTILS FUNCTIONS ###############################
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TESTS_VECTOR_UTILS.H ###############################
//----------------------------------------------------------

// Prevent Recursive Inclusion -------------------------------------------------
#ifndef __TESTS_VECTOR_UTILS_H_
#define __TESTS_VECTOR_UTILS_H_

// Includes to help defines ----------------------------------------------------
#include <stdio.h>


// Module Exported Functions ---------------------------------------------------
void Print_Array_Uchar (unsigned char * array, int size, int length);
void Print_Array_Ushort (unsigned short * array, int size, int length);

void Vector_Short_To_File (FILE * f, char * v_name, short * v_data, int v_len);
void Vector_UShort_To_File (FILE * f, char * v_name, unsigned short * v_data, int v_len);
void Vector_UChar_To_File (FILE * f, char * v_name, unsigned char * v_data, int v_len);
void Vector_Float_To_File (FILE * f, char * v_name, float * v_data, int v_len);

void Variable_UShort_To_File (FILE * f, char * v_name, unsigned short data);
void Variable_UChar_To_File (FILE * f, char * v_name, unsigned char data);

void ShowVectorFloat (char * s_comment, float * f_vect, int size);
void ShowVectorUShort (char * s_comment, unsigned short * int_vect, int size);
void ShowVectorShort (char * s_comment, short * int_vect, int size);
void ShowVectorInt (char * s_comment, int * int_vect, int size);

#endif    /* __TESTS_VECTOR_UTILS_H_ */

//--- end of file ---//

