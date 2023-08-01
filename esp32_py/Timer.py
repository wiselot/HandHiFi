''' Timer '''
I2C_OLED_SCL_PIN = const(22)
I2C_OLED_SDA_PIN = const(21)
OLED_FONT_HEIGT = const(8)
OLED_FONT_WIDTH = const(8)

KEY0_GPIO = const(35)
KEY1_GPIO = const(34)
KEY2_GPIO = const(5)
KEY3_GPIO = const(16)
import time

def menu_entry_timer(oled,key_map):
    
    key0 = key_map[0][0]
    key0_st = key_map[0][1]
    key1 = key_map[1][0]
    key1_st = key_map[1][1]
    key2 = key_map[2][0]
    key2_st = key_map[2][1]
    key3 = key_map[3][0]
    key3_st = key_map[3][1]
    
    oled.fill(0)
    oled.text("KEY:0 E;1 S;2 P",0,0,1)
    oled.show()
    timer_count = 0
    timer_enable = 0
    c = 0
    while True:
        key0_st = key0.value()
        if key0_st != 1:
            if key0_st == 0:
                break
        key1_st = key1.value()
        if key1_st != 1:
            if key1_st == 0:
                timer_enable = 1
        key2_st = key2.value()
        if key2_st != 1:
            if key2_st == 0:
                timer_enable = 0
        c+=1
        if timer_enable:
            if c==10:
                timer_count+=1
                c = 0
            oled.fill_rect(32,32,32 + OLED_FONT_WIDTH * 9 ,32 + OLED_FONT_HEIGT,0);
            oled.text(timer_count_to_str(timer_count),32,32,1)
            oled.show()
        if c==10:
            c = 0
        time.sleep_ms(100)
    return 0
        
def timer_count_to_str(count):
    sec = count % 60
    min = count // 60
    hour = count // 3600
    return str(hour) + ':' + str(min) + ':' + str(sec)
