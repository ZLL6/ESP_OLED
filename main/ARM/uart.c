#include "uart.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "global.h"
#include "stdint.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *TAG = "Uart task";
#define ARM_UART_NUM    UART_NUM_2    



static QueueHandle_t Uart_Queue;
static TaskHandle_t  Uart_TaskHandle;
static uint8_t Uart_Rx_Buffer[UART_BUFFER_SIZE];
static uint8_t Uart_Tx_Buffer[UART_BUFFER_SIZE];


char* hexArrayToHexStringWithSpaces(const unsigned char* hexArray, size_t arrayLength) {
    // ����������ַ������ȣ�������ֹ��'\0'
    size_t hexStringLength = arrayLength * 2 + (arrayLength - 1) + 1; // +1 for the null terminator
    
    // �����ڴ�ռ���ַ�����������ֹ��'\0'
    char* hexString = (char*)malloc(hexStringLength);
    if (!hexString) {
        return NULL; // �ڴ����ʧ��
    }
    
    // ��ʼ���ַ���ָ��
    char* ptr = hexString;
    
    // ת�������ӿո�
    for (size_t i = 0; i < arrayLength; ++i) {
        // ת������16�����ַ�
        sprintf(ptr, "%02X", hexArray[i]);
        
        // �ƶ�����һ��λ�ã�׼����ӡ�ո������Ҫ��
        ptr += 2;
        
        // ��ӡ�ո񣬳������һ���ַ�
        if (i < arrayLength - 1) {
            *ptr = ' ';
            ++ptr;
        }
    }
    
    // �����ַ�������ֹ��
    *ptr = '\0';
    
    return hexString;
}

void Hex_Print(uint8_t*hexArray, int  arrayLength)
{
    // ת������Ϊ�ַ���
    char* hexString = hexArrayToHexStringWithSpaces(hexArray, arrayLength);
    
    // ��ӡ���
    if (hexString) {
        ESP_LOGI(TAG,"%s\n", hexString);
        free(hexString); // �ͷŷ�����ڴ�
    } else {
        ESP_LOGI(TAG,"Memory allocation failed.\n");
    }
    
    return;
}


static void UART_Init()
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    ESP_ERROR_CHECK(uart_driver_install(ARM_UART_NUM, UART_BUFFER_SIZE, UART_BUFFER_SIZE, 0, NULL, ESP_INTR_FLAG_LOWMED));
    ESP_ERROR_CHECK(uart_param_config(ARM_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ARM_UART_NUM, ESP_UART_TX_IO, ESP_UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

static int UART_Recv()
{
    return uart_read_bytes(ARM_UART_NUM,Uart_Rx_Buffer,UART_BUFFER_SIZE,portTICK_PERIOD_MS);//100ms
}

static int UART_Send(uint8_t* buff,int len)
{
    return uart_write_bytes(ARM_UART_NUM,buff,len);
}

static void UART_ARM_Task(void *pvParameters)
{
    UART_Init();
    //xQueueReceive //xQueueSend
    Uart_Queue = xQueueCreate(UART_QUEUE_SIZE,sizeof(uint8_t));
    int read_len = 0;
    uint8_t cnt = 0;
    while(1)
    {
        read_len = UART_Recv();
        if(read_len > 0){
            Hex_Print(Uart_Rx_Buffer,read_len);
            //UART_Send(Uart_Rx_Buffer,read_len);
            cnt++;
            if(xQueueSend(OLED_Queue,&cnt, 0 / portTICK_PERIOD_MS)!=pdPASS){    ////当队列为空时，0不等队列
                ESP_LOGI(TAG, "send to OLED queue failed");
            }else{
                ESP_LOGI(TAG, "send to OLED queue:%d--%d",uxQueueMessagesWaiting(OLED_Queue),uxQueueSpacesAvailable(OLED_Queue));
            }
        }
    }
    //vTaskDelete(Uart_TaskHandle);
}

void ARM_Task_Init(void)
{
    BaseType_t xStatus = xTaskCreate(UART_ARM_Task, "uart_queue_task", UART_TASK_SIZE,
                                        NULL, UART_TASK_PRIO, &Uart_TaskHandle);
    if (xStatus != pdPASS) {
        vTaskDelete(Uart_TaskHandle);
        // Force exit from function with failure
        ESP_LOGI(TAG, "Uart task create failed");
    } else {
        ESP_LOGI(TAG, "Uart task create scuessful");
        //vTaskSuspend(Uart_TaskHandle); // Suspend serial task while stack is not started
    }

}



