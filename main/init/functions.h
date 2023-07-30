#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

#define NO_ABORT_ERROR_CHECK(err,TAG,msg) ({ \
    if(err!=ESP_OK){                        \
        ESP_LOGE(TAG,msg);                  \
        return 1;}                           \                                    
})

//wifi
/* 初始化wifi */
void wifi_init();
/*  连接wifi
*  连接模式:
*  0: 提供ssid和passwd
*  1: 从NVS获取已保存wifi(1:尝试第一个,2:全部尝试)
*  3: 通过图形化和按键输出选择
*/
int wifi_connect(int mode,uint8_t *ssid,uint8_t *passwd);
/* 初始化存储wifi */
int build_nvs_storage_wifi();
/* 存储wifi */
int store_wifi(uint8_t *ssid,uint8_t *passwd);
/* 列出所有wifi */
int list_store_wifi();
/* 删除wifi
 * 0<=n<32: 按位号
 * n<0 | n>=32 : 按ssid和passwd
 */
int del_wifi(uint8_t *ssid,uint8_t *passwd,int n);

// ssd1306 oled
#define I2C_SSD1306_DEFAULT_SDA_IO      21
#define I2C_SSD1306_DEFAULT_SCL_IO      22
#define I2C_SSD1306_DEFAULT_FREQ        400000
#define I2C_SSD1306_ADDR                0x3c
#define I2C_SSD1306_DEFAULT_POER        1

#define OLED_DEFAULT_WIDTH              128
#define OLED_DEFAULT_HEIGHT             64

/* oled初始化 */
int oled_init(int sda,int scl,int freq,int poer_num);
/* oled刷新 */
int oled_write();
/* oled清屏 */
int oled_clear();
/* oled放置像素点 */
void oled_putpixel(uint8_t x,uint8_t y,uint8_t bit);
/* oled放置buf */
void oled_putframe(uint8_t x,uint8_t y,uint8_t ex,uint8_t ey,uint8_t *buf,uint8_t bit);
/* oled放置字符 */
void oled_putchar(uint8_t x,uint8_t y,int c,uint8_t bit);
/* oled加载字体 */
int oled_load_font(const uint8_t *font);

#endif