// 实现i2c初始化
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "functions.h"
#include "font.h"

static const char * TAG = "oled";

#define ACK_CHECK_EN        0x1
#define ACK_CHECK_DIS       0x0
#define ACK_VAL             0x0
#define NACK_VAL            0x1
#define CMD_REG             0x00
#define DATA_REG            0x40

static const uint8_t oled_init_cmd[] = {
    
    0x80,0xAE,//--turn off oled panel
    0x80,0x00,//---set low column address
    0x80,0x10,//---set high column address
    0x80,0x40,//--set start line address  Set Mapping RAM Display Start Line
    0x80,0x81,//--set contrast control register
    0x80,0xCF,// Set SEG Output Current Brightness
    0x80,0xA1,//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    0x80,0xC8,//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    0x80,0xA6,//--set normal display
    0x80,0xA8,//--set multiplex ratio(1 to 64)
    0x80,0x3F,//--1/64 duty
    0x80,0xD3,//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    0x80,0x00,//-not offset
    0x80,0xD5,//--set display clock divide ratio/oscillator frequency
    0x80,0x80,//--set divide ratio, Set Clock as 100 Frames/Sec
    0x80,0xD9,//--set pre-charge period
    0x80,0xF1,//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    0x80,0xDA,//--set com pins hardware configuration
    0x80,0x12,//
    0x80,0xDB,//--set vcomh
    0x80,0x40,//Set VCOM Deselect Level
    0x80,0x20,//-Set Page Addressing Mode (0x00/0x01/0x02)
    0x80,0x02,//
    0x80,0x8D,//--set Charge Pump enable/disable
    0x80,0x14,//--set(0x10) disable
    0x80,0xA4,// Disable Entire Display On (0xa4/0xa5)
    0x80,0xA6,// Disable Inverse Display On (0xa6/a7) 
    0x00,0xAF // 
};


static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_SSD1306_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t ssd1306_write_cmd(unsigned char cmd){
    unsigned char s[] = {CMD_REG,0x00};
    s[1] = cmd;
    return i2c_master_write_slave(I2C_SSD1306_DEFAULT_POER,s,sizeof(s));
}

static esp_err_t ssd1306_write_data(unsigned char data){
    unsigned char s[] = {DATA_REG,0x00};
    s[1] = data;
    return i2c_master_write_slave(I2C_SSD1306_DEFAULT_POER,s,sizeof(s));
}

static uint8_t oled_buffer[128*8];

typedef struct{
    uint8_t     width;
    uint8_t     height;
    uint8_t     poer_num;
    uint8_t     sda;
    uint8_t     scl;
    uint8_t     freq;
    const uint8_t     *font;
}oled_i2c_settings;

static oled_i2c_settings i2c_oled_settings;

int oled_init(int sda,int scl,int freq,int poer_num)
{
    esp_err_t esp_err;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq,
    };
    NO_ABORT_ERROR_CHECK(i2c_param_config(poer_num, &conf),TAG,"oled init:param config failed");
    NO_ABORT_ERROR_CHECK(i2c_driver_install(poer_num, I2C_MODE_MASTER, 0, 0, 0),TAG,"oled init:driver install failed");
    NO_ABORT_ERROR_CHECK(i2c_master_write_slave(poer_num, oled_init_cmd, 56),TAG,"oled init:write settings failed");
    
    i2c_oled_settings.width = OLED_DEFAULT_WIDTH;
    i2c_oled_settings.height = OLED_DEFAULT_HEIGHT;
    i2c_oled_settings.poer_num = poer_num;
    i2c_oled_settings.sda = sda;
    i2c_oled_settings.scl = scl;
    i2c_oled_settings.freq = freq;

    oled_load_font(Font7x13);
    oled_putchar(0,0,'a',1);
    oled_putchar(32,32,'G',1);
    oled_write();
    return 0;
}

/* oled 显示部分,默认128*64尺寸 */

/*清屏*/
int oled_clear()
{
    memset(oled_buffer,0,128*8);
    return oled_write();
}

/* 放置像素 */
void oled_putpixel(uint8_t x,uint8_t y,uint8_t bit)
{
    if(bit)
        oled_buffer[x+(y/8)*128] |= 1 << (y % 8);
    else
        oled_buffer[x+(y/8)*128] &= ~(1 << (y % 8));
}

/* 放置图像流 */
void oled_putframe(uint8_t x,uint8_t y,uint8_t ex,uint8_t ey,uint8_t *buf,uint8_t bit)
{
    if(!buf)    return;
    for(int i=y;i<ey;i++){
        for(int j=x;j<ex;j++){
            oled_putpixel(j,i,bit);
        }
    }
}

/* 加载字库 */
int oled_load_font(const uint8_t *font)
{
    if(!font) return 1;
    i2c_oled_settings.font = font;
    return 0;
}

/* 放置字符 */
void oled_putchar(uint8_t x,uint8_t y,int c,uint8_t bit)
{
    uint8_t fOffset, fWidth, fHeight, fBPL;
    uint8_t* tempChar;

    fOffset = i2c_oled_settings.font[0];
    fWidth = i2c_oled_settings.font[1];
    fHeight = i2c_oled_settings.font[2];
    fBPL = i2c_oled_settings.font[3];

    tempChar = (uint8_t*)&i2c_oled_settings.font[((c - 0x20) * fOffset) + 4];
    for (int j = 0; j < fHeight; j++) {
        for (int i = 0; i < fWidth; i++) {
            uint8_t z = tempChar[fBPL * i + ((j & 0xF8) >> 3) + 1];
            uint8_t b = 1 << (j & 0x07);
            if ((z & b) != 0x00)
            	oled_putpixel(x + i, y + j, bit);
            else
            	oled_putpixel(y + i, y + j, !bit);
        }
    }
}

/* 写入数据 */
int oled_write()
{
    uint8_t write_cmd[129];
    write_cmd[0] = DATA_REG;
    for (uint8_t page = 0;page < 8;page++)
    {
        ssd1306_write_cmd(0xb0 + page);
        ssd1306_write_cmd(0x00);
        ssd1306_write_cmd(0x10);
        for (size_t i = 1; i < 129; i++)
            write_cmd[i] = oled_buffer[(page*128)+i-1];
        NO_ABORT_ERROR_CHECK(i2c_master_write_slave(I2C_SSD1306_DEFAULT_POER,write_cmd, 129),\
                            TAG,"oled disp:buffer write failed");
    }
    return 0;
}
