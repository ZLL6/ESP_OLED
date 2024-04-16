#include "web_data.h"
#include "stdio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "string.h"
#include "cJSON.h"
#include "typeCovert.h"

static const char *TAG = "web_data";
extern void response_200_ok_to_server(int client_fd,const char* contentType,const char* data, int dataLen);
void ESP_MsgProc_CustomMsg(cJSON* pJson,int client_fd)
{
    cJSON *pSub = NULL; 
    pSub = cJSON_GetObjectItem(pJson, "Msg");
    if(NULL == pSub) {
        return;
    }
    int value = 0;
    if(HexStringToDecInt(pSub->valuestring,&value) == 0)
    {
        ESP_LOGE(TAG,"\r\nnum:%d\r\n",value);
    }else{
        ESP_LOGE(TAG,"format error");
    }
    response_200_ok_to_server(client_fd,"application/json",NULL,0);
}


static const struct{
  const char* method;
  void (*cb)(cJSON*,int);
}Fun_cb[] = {
    {.method = "CustomMsg",             .cb = ESP_MsgProc_CustomMsg},
};

void data_html_post_process(char* html_msg, int client_fd)
{
    ESP_LOGE(TAG,"\r\nJson:%s\r\n",html_msg);
    static int cb_num = sizeof(Fun_cb)/sizeof(Fun_cb[0]);
    cJSON* pJson = cJSON_Parse(html_msg);
    if(pJson != NULL){
        cJSON* pESPData = cJSON_GetObjectItem(pJson, "ESPData");
        if(pESPData != NULL){
            uint8_t i=0;
            for(i=0;i<cb_num;i++)
            {
                if(strcmp(pESPData->valuestring,Fun_cb[i].method) == 0){
                    ESP_LOGE(TAG,"\r\nenter pcs event:%s",Fun_cb[i].method);
                    Fun_cb[i].cb(pJson,client_fd);
                    break;
                }
            }
            if(i >= cb_num){
                ESP_LOGE(TAG,"\r\n no match any method....\r\n");
            }
        }
        cJSON_Delete(pJson);
    }
}
