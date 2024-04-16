#include "stdio.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "freertos/task.h"
#include "global.h"
#include "OLED.h"

static const char *TAG = "oled task";
static TaskHandle_t  OLED_TaskHandle;
QueueHandle_t OLED_Queue;

#define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0     /*!< I2C master will not check ack from slave */


/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO, // 18
        .scl_io_num = I2C_MASTER_SCL_IO, // 19
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * @description: OLED ����һ���ֽ�
 * @return       ������Ϣ
 * @param {uint8_t} data ��Ҫ���͵����ݣ����ݻ�������
 * @param {uint8_t} cmd_ 1:�������� 0:��������
 */
esp_err_t OLED_WR_Byte(uint8_t data, uint8_t cmd_)
{

    uint8_t write_buf[2] = {((cmd_ == 1) ? (0x40) : (0x00)), data};

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_ADDR << 1) | 0, ACK_CHECK_EN);
    i2c_master_write(cmd, write_buf, 2, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;

    /* ret = i2c_master_write_to_device(I2C_MASTER_NUM, OLED_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS); */

    // return ret;
}

/**
 * @description: OLED ��Ļ��ʼ��
 * @return       ��
 */
void OLED_Init(void)
{
    CodeTabInit();

    OLED_WR_Byte(0xAE, OLED_CMD); //--display off
    OLED_WR_Byte(0x00, OLED_CMD); //---set low column address
    OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
    OLED_WR_Byte(0x40, OLED_CMD); //--set start line address
    OLED_WR_Byte(0xB0, OLED_CMD); //--set page address
    OLED_WR_Byte(0x81, OLED_CMD); // contract control
    OLED_WR_Byte(0xFF, OLED_CMD); //--128
    OLED_WR_Byte(0xA1, OLED_CMD); // set segment remap
    OLED_WR_Byte(0xA6, OLED_CMD); //--normal / reverse
    OLED_WR_Byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3F, OLED_CMD); //--1/32 duty
    OLED_WR_Byte(0xC8, OLED_CMD); // Com scan direction
    OLED_WR_Byte(0xD3, OLED_CMD); //-set display offset
    OLED_WR_Byte(0x00, OLED_CMD); //
    OLED_WR_Byte(0xD5, OLED_CMD); // set osc division
    OLED_WR_Byte(0x80, OLED_CMD); //
    OLED_WR_Byte(0xD8, OLED_CMD); // set area color mode off
    OLED_WR_Byte(0x05, OLED_CMD); //
    OLED_WR_Byte(0xD9, OLED_CMD); // Set Pre-Charge Period
    OLED_WR_Byte(0xF1, OLED_CMD); //
    OLED_WR_Byte(0xDA, OLED_CMD); // set com pin configuartion
    OLED_WR_Byte(0x12, OLED_CMD); //
    OLED_WR_Byte(0xDB, OLED_CMD); // set Vcomh
    OLED_WR_Byte(0x30, OLED_CMD); //
    OLED_WR_Byte(0x8D, OLED_CMD); // set charge pump enable
    OLED_WR_Byte(0x14, OLED_CMD); //
    OLED_WR_Byte(0xAF, OLED_CMD); //--turn on oled panel
    OLED_Clear();
}

/**
 * @description: OLED ��Ļ ��������
 * @return       ��
 * @param {uint8_t} x ����x�ᣬ��Χ0~127
 * @param {uint8_t} y ����y�ᣬ��Χ0~63
 */
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    OLED_WR_Byte(0xb0 + y, OLED_CMD);
    OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    OLED_WR_Byte((x & 0x0f), OLED_CMD);
}

/**
 * @description: OLED ����
 * @return       ��
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);
        OLED_WR_Byte(0x00, OLED_CMD);
        OLED_WR_Byte(0x10, OLED_CMD);
        for (n = 0; n < 128; n++)
            OLED_WR_Byte(0, OLED_DATA);
    }
}

/**
 * @description: OLED ��ʾ�����ַ�
 * @return       ��
 * @param {uint8_t} x ��ʾ�ַ���x���꣬��Χ0~127
 * @param {uint8_t} y ��ʾ�ַ���y���꣬�ַ���СΪ16��ȡֵ0,2,4,6���ַ���С6��ȡֵ0,1,2,3,4,5,6,7
 * @param {uint8_t} chr ��ʾ�ĵ����ַ������ֿ��г��ֵ��ַ�
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr)
{
    uint8_t c = 0;
    uint8_t i = 0;
    c = chr - ' ';
    if (x >= 127)
    {
        return;
    }
    OLED_Set_Pos(x, y);
    for (i = 0; i < 8; i++)
        OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
    OLED_Set_Pos(x, y + 1);
    for (i = 0; i < 8; i++)
        OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
}

/**
 * @description: OLED ��ʾ����
 * @return       ��
 * @param {uint8_t} x ��ʾ���ֵ�x����
 * @param {uint8_t} y ��ʾ���ֵ�y����
 * @param {uint8_t} no ��ʾ�������ֿ��е����
 */
void OLED_ShowOneChinese(uint8_t x, uint8_t y, uint16_t code)
{
    uint8_t t;
    const uint8_t* pCndata;
    pCndata = SearchCnCode(code);
    if(pCndata != NULL){
        OLED_Set_Pos(x, y);
        for (t = 0; t < 16; t++)
        {
            OLED_WR_Byte(*pCndata++, OLED_DATA);
        }
        OLED_Set_Pos(x, y + 1);
        for (t = 0; t < 16; t++)
        {
            OLED_WR_Byte(*pCndata++, OLED_DATA);
        }
    }
    
}

void OLED_ShowStrings(uint8_t x, uint8_t y, uint8_t* pWords)
{
    uint16_t xPos = x;
    uint16_t value;

    uint8_t* p;
    p = pWords;

    while(*p != '\0')
    {
        if(*p < 0x80){
            OLED_ShowChar(xPos,y,*p);
            p++;
            xPos += (WROD_WIDTH/2+1);  
            if(xPos+(WROD_WIDTH/2+1) >= OLED_WIDTH){
                return;
            }   
        }else
        {
            value = *p << 8;
            p++;
            value = value | *p;
            p++;
            OLED_ShowOneChinese(xPos,y,value);   
            xPos += (WROD_WIDTH+1);     
            if(xPos+(WROD_WIDTH+1) >= (OLED_WIDTH-1)){

                // y += WORD_HIGHT;
                // xPos = 0;
                // if(y >= (OLED_HEIGHT-1)){
                return;
                // }
            }
        }
        
        
    }
}

int countDigits(int num) {
    int count = 0;

    // ���������������� num Ϊ 0���򷵻� 1
    if (num == 0) {
        return 1;
    }

    // �� num ���г������㣬ÿ�γ��� 10��ֱ�� num ��Ϊ 0
    while (num != 0) {
        num /= 10;
        count++;
    }

    return count;
}
void intToASCIIArray(int num, uint8_t* array, int count)
{
    if(array == NULL){
        return;
    }

    uint8_t i = count - 1;
    while(count)
    {
        array[i] = '0' + (num % 10);
        count--;
        i--;
        num /= 10;
    }
    return;
}

/**
 * @description: OLED ��ʾ����
 * @return       ��
 * @param {uint8_t} x ��ʾ���ֵĵ�һ��λ�õ�x����
 * @param {uint8_t} y ��ʾ���ֵĵ�һ��λ�õ�y����
 * @param {uint32_t} num ����ʾ������
 * @param {uint8_t} len ��ʾ��ռ�ĳ��ȣ�������С������Ҫ��ʾ�����ֵĳ���
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num)
{
    uint8_t buff[20];
    memset(buff,' ', sizeof(buff));
    uint8_t count = countDigits(num);

    intToASCIIArray(num,buff,count);
    for(uint8_t i = 0; i < count; i++)
    {
        OLED_ShowChar(x + 8 * i, y, buff[i]);
        if(x + 8 * i + 8 > OLED_WIDTH){
            return;
        }
    }
    
}
/***********������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    	for(x=x0;x<x1;x++)
	    {      
	    	OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
	    }
	}
}
#define oled_delay_ms 100
uint32_t task_cnt = 0;
void oled_task(void *arg)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    // OLED��Ļ��ʼ��
    OLED_Init();

    // // ��ʾ����

    // OLED_ShowStrings(2,0,(uint8_t*)"I love ������");
    // OLED_ShowStrings(2,4,(uint8_t*)"��are a sha bi���");
    // // ��ʾ����
    // OLED_ShowNum(0, 6, 1235678);
    uint8_t value;
    while(1)
    {
        //OLED_DrawBMP(0,0,32,4,good);
        if(xQueueReceive(OLED_Queue,&value,portMAX_DELAY) == pdPASS){       ////当队列为空时，0不等队列
            ESP_LOGE(TAG, "recv:%d\n   leave:%d",value,uxQueueMessagesWaiting(OLED_Queue));
            OLED_ShowStrings(2,0,(uint8_t*)"recv:");
            OLED_ShowNum(2, 2, value);
        }else{
            ESP_LOGE(TAG, "OLED recv failed: %d---%d\n",uxQueueMessagesWaiting(OLED_Queue),uxQueueSpacesAvailable(OLED_Queue));
        }
        if(task_cnt % 2 == 0){
            //OLED_Clear();
        }
        //ESP_LOGE(TAG, "oled task...");
        //vTaskDelay(oled_delay_ms / portTICK_RATE_MS);
        task_cnt++;
    }
}

void OLED_Task_Init(void)
{
    OLED_Queue = xQueueCreate(OLED_QUEUE_SIZE,sizeof(uint8_t));
    BaseType_t xStatus = xTaskCreate(oled_task, "oled_task", 1024 * 2, (void *)0, 10, &OLED_TaskHandle);
    if (xStatus != pdPASS) {
        vTaskDelete(OLED_TaskHandle);
        // Force exit from function with failure
        ESP_LOGI(TAG, "OLED task create failed");
    } else {
        ESP_LOGI(TAG, "OLED task create scuessful");
        //vTaskSuspend(OLED_TaskHandle); // Suspend serial task while stack is not started
    }
}
