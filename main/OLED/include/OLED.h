#ifndef __OLED_H__
#define __OLED_H__

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "OLEDFont.h"

#define OLED_QUEUE_SIZE     20


#define I2C_MASTER_SCL_IO 19        /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 18        /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 100

#define OLED_ADDR 0x3C // OLED的IIC地址，逻辑分枝�?读出�?

#define OLED_CMD 0
#define OLED_DATA 1

#define OLED_WIDTH  128         
#define OLED_HEIGHT 8

// 函数声明
esp_err_t i2c_master_init(void);
esp_err_t OLED_WR_Byte(uint8_t data, uint8_t cmd_);
void OLED_Init(void);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr);
void OLED_Clear(void);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num);
void OLED_ShowOneChinese(uint8_t x, uint8_t y, uint16_t code);

#endif /* __OLED_H__ */