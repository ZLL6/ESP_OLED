#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "stdio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "string.h"
#include "freertos/queue.h"


extern QueueHandle_t OLED_Queue;
void OLED_Task_Init(void);
void ARM_Task_Init(void);
int DecStringToDecInt(char* src,int* value);
int HexStringToDecInt(char* src,int* value);
#endif
