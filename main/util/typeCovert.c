#include "typeCovert.h"
#include "stdint.h"
#include "string.h"


int DecStringToDecInt(char* src,int* value)
{
    if(src == NULL){
        return -1;
    }
    int index=0;
    int tmp_value = 0;
    while(src[index])
    {
        if(src[index]>='0' && src[index]<='9'){
            index ? (tmp_value *= 10) : (tmp_value *= 0);
            tmp_value += (src[index]-'0');
            index++;
        }else{
            return -2;

        }       
    }
    *value = tmp_value;
    return 0;
}

int HexStringToDecInt(char* src,int* value)
{
    if(src == NULL){
        return -1;
    }
    int index=0;
    int tmp_value = 0;
    while(src[index])
    {
        index ? (tmp_value *= 16) : (tmp_value *= 0);
        if(src[index]>='0' && src[index]<='9'){            
            tmp_value += (src[index]-'0');
        }else if(src[index]>='a' && src[index]<='f'){
            tmp_value += (src[index]-'a');
        }else if(src[index]>='A' && src[index]<='F'){
             tmp_value += (src[index]-'A');
        }else{
            return -2;
        } 
        index++; 
    }
    *value = tmp_value;
    return 0;
}
