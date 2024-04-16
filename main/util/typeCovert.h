#ifndef _TYPECOVERT_H
#define _TYPECOVERT_H

#include "stdio.h"
/* if src == 0,whe whole src to convert
    return 0 == OK，else error
 */
int DecStringToDecInt(char* src,int* value);
int HexStringToDecInt(char* src,int* value);

#endif
