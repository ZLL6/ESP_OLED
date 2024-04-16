#include "webserver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "string.h"
#include "stdio.h"
#include "web_data.h"

#define TAG "webserver"

static const char* GET = "GET";
static const char* POST = "POST"; 

typedef struct{
    char* url;
    void (*Get_fun)(int);
    void (*Post_fun)(char*,int);
}URL_Handler_t;

static int create_server_socket(in_port_t in_port, in_addr_t in_addr)
{
    int temp_socket = -1;
    struct sockaddr_in server = {
        .sin_port = in_port,
        .sin_family = AF_INET,
        .sin_addr.s_addr = in_addr
    };
    if((temp_socket = socket(AF_INET,SOCK_STREAM,0)) < 0){
        ESP_LOGE(TAG, "Failed to create socket");
        return -1;
    }
    int on =1;
    setsockopt(temp_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    if(bind(temp_socket,(struct sockaddr *)&server,sizeof(server)) < 0) {
        ESP_LOGE(TAG, "Failed to bind socket");
        return -1;
    }
    if(listen(temp_socket,1) < 0){
        ESP_LOGE(TAG, "Failed to listen socket");
        close(temp_socket);
		return -1;
    }
    return temp_socket;
}

static int accept_with_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_ms)
{
    int ret = -1;
    if(wait_ms > 0){
        fd_set accept_fdset;
        FD_ZERO(&accept_fdset);
        FD_SET(fd,&accept_fdset);
        struct  timeval timeout = {
            .tv_sec =wait_ms / 1000,
            .tv_usec = (wait_ms%1000)/1000,
        };
        do{
            ret = select(fd+1,&accept_fdset,NULL,NULL,&timeout);
        }while(ret < 0);
        if(ret <= 0){
            return -1;
        }
    }

    if(addr != NULL){
        socklen_t addrlen = sizeof(struct sockaddr_in);
        return accept(fd, (struct sockaddr*)addr, &addrlen);
    }else{
        return accept(fd, NULL, NULL);
    }
}

void web_response_body(int client_fd,const char* data, int dataLen)
{
    if(data != NULL && dataLen > 0){
        char tmpbuff[20];
        int t = dataLen / 1024 + 1;
        int len = dataLen % 1024;
        for(int i=0;i<t;i++)
        {
            int tlen = (i == (t-1))?len:1024;
            snprintf(tmpbuff,sizeof(tmpbuff),"%x\r\n",tlen);
            write(client_fd,tmpbuff,strlen(tmpbuff));
            write(client_fd,data+i*1024,tlen);
            write(client_fd,"\r\n",2);
        }
    }
    write(client_fd,"0\r\n\r\n",7);
}

void response_200_ok_to_server(int client_fd,const char* contentType,const char* data, int dataLen)
{
    char buffer[200];
    //head 
    int requestHeaderLen = snprintf(buffer,sizeof(buffer),"HTTP/1.1 200 OK\r\ncontent-type: %s;\r\ncontent-length: %u\r\nTransfer-Encoding: chunked\r\n\r\n",contentType,dataLen);
    if(requestHeaderLen > (sizeof(buffer)-1)){
        ESP_LOGE(TAG,"request header printf error!");
        return;
    }
    write(client_fd,buffer,requestHeaderLen);
    web_response_body(client_fd,data,dataLen);
}

void HTTP_Send_html_page(int client_fd)
{
    ESP_LOGE(TAG,"start send html data");
    response_200_ok_to_server(client_fd,"text/html",html_data,sizeof(html_data));
}

static URL_Handler_t method_buffer[] = {
    {"/data.html",HTTP_Send_html_page,data_html_post_process},
    {"/set.html",NULL,NULL},
};
void Get_Request_Method_url(char* buffer,char* method,char* url)
{
    const char* req = strchr(buffer,' ');
    if(req != NULL){
        memcpy(method,buffer,req-buffer);
        ESP_LOGE(TAG,"method_len:%d\n",req-buffer);
    }

    const char* req2 = strchr(req+1,' ');
    if(req2 != NULL){
        memcpy(url,req+1,req2-(req+1));
    }
    return;
}

char* getJSONFromHttpData(char* src)
{
  if('{' == src[0]){
    return src;
  }
  char* p = strstr(src,"\r\n\r\n{");
  if(p != NULL){
    p+=4;
  }
  return p;
}

static void Client_request_process(int client_fd, char* buffer,uint16_t len)
{
    char method[5] = {0};
    char url[20];
    int index = -1;
    ESP_LOGE(TAG,"recv:%s\r\n",buffer);
    Get_Request_Method_url(buffer,method,url);
    ESP_LOGE(TAG,"req_method:%s url:%s\r\n",method,url);

    for(int i=0;i<sizeof(method_buffer)/sizeof(method_buffer[0]);i++)
    {
        if(strcmp(method_buffer[i].url,url) == 0){
            index = i;
        }
    }
    if(index < 0){
        ESP_LOGE(TAG,"URL not match !!!");
        return;
    }
    ESP_LOGE(TAG,"URL:%s matched...",url);
    if(strcmp(method,GET) == 0){
        ESP_LOGE(TAG,"start process GET ");
        if(method_buffer[index].Get_fun){
            method_buffer[index].Get_fun(client_fd);
        }        
    }else if(strcmp(method,POST) == 0){
        ESP_LOGE(TAG,"start process POST ");
        if(method_buffer[index].Post_fun){
            char* msg = getJSONFromHttpData(buffer);
            if(msg != NULL){
                method_buffer[index].Post_fun(msg,client_fd);
            }            
        }
        
    }else{
        ESP_LOGE(TAG,"request method error ....");
    }
}
void WebServer_Task(void *arg)
{
    int server_fd = -1;
    int client_fd = -1;
    struct timeval tv = {
        .tv_sec = 3,
        .tv_usec = 0,
    };

    int recv_length = -1;
    char recv_method_url[1024*2] = {0};
    while(1)
    {
        vTaskDelay(10 / portTICK_RATE_MS);
        if(server_fd < 0){
            server_fd = create_server_socket(htons(80),htonl(INADDR_ANY));
            if(server_fd < 0){
                ESP_LOGE(TAG,"failed to create server socket");
                continue;
            }
        }
        client_fd = accept_with_timeout(server_fd,NULL,300);
        if(client_fd > 0){
            setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            do{
                recv_length = recv(client_fd,recv_method_url,sizeof(recv_method_url),10);
                if(recv_length <= 0){
                    break;
                }
                Client_request_process(client_fd,recv_method_url,recv_length);
            }while(recv_length > 0);
            
            
        }
    }
}

void webserver_start(void)
{
    if(xTaskCreate(WebServer_Task,TAG,1024*4,NULL,1,NULL) != pdPASS){
        ESP_LOGE(TAG, "webserver Task creation failed");
        return;
    }
}
