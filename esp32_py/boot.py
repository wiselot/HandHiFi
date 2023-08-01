# This file is executed on every boot (including wake-boot from deepsleep)
#import esp
#esp.osdebug(None)
#import webrepl
#webrepl.start()

from machine import I2C,Pin
from ssd1306 import SSD1306_I2C
import time
import framebuf

I2C_OLED_SCL_PIN = const(22)
I2C_OLED_SDA_PIN = const(21)
OLED_FONT_HEIGT = const(8)
OLED_FONT_WIDTH = const(8)

KEY0_GPIO = const(35)
KEY1_GPIO = const(34)
KEY2_GPIO = const(5)
KEY3_GPIO = const(16)

# functions

from Timer import menu_entry_timer
from WifiTool import menu_entry_wifi_tool
from  funny import menu_entry_funny

#init

i2c = I2C(1,scl=Pin(I2C_OLED_SCL_PIN), sda=Pin(I2C_OLED_SDA_PIN))
oled = SSD1306_I2C(128, 64, i2c)

key0 = Pin(KEY0_GPIO, Pin.IN, Pin.PULL_UP)
key0_st = key0.value()
key1 = Pin(KEY1_GPIO, Pin.IN, Pin.PULL_UP)
key1_st = key1.value()
key2 = Pin(KEY2_GPIO, Pin.IN, Pin.PULL_UP)
key2_st = key2.value()
key3 = Pin(KEY3_GPIO, Pin.IN, Pin.PULL_UP)
key3_st = key3.value()

key_map = [(key0,key0_st),(key1,key1_st),(key2,key2_st),(key3,key3_st)]

menu_list = ["Wifi Tool","Timer","Funny"]
menu_point_list = [menu_entry_wifi_tool,menu_entry_timer,menu_entry_funny]

def menu_main_boot(index):
    ret = 0
    print("User enter " + menu_list[index])
    oled.fill(0)
    oled.show()
    # boot apps
    
    ret = menu_point_list[index](oled,key_map)
    
    print("Return to the main menu")
    return ret

import lightUI

# 主界面
lightUI = lightUI.lightUI(oled,key_map)
lightUI.Chooser("ESP32 Menu",menu_list,menu_main_boot)
#lightUI.DialogInfo("ok")