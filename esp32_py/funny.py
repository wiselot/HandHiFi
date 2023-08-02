import time

''' Funny '''
def menu_entry_funny(oled,key_map):
    
    key0 = key_map[0][0]
    key0_st = key_map[0][1]
    key1 = key_map[1][0]
    key1_st = key_map[1][1]
    key2 = key_map[2][0]
    key2_st = key_map[2][1]
    key3 = key_map[3][0]
    key3_st = key_map[3][1]
    
    oled.fill(0)
    oled.show()
    c = 0
    import res_kun
    img = [res_kun.IMG00000,res_kun.IMG00001,res_kun.IMG00002,res_kun.IMG00003,res_kun.IMG00004,res_kun.IMG00005,res_kun.IMG00006,res_kun.IMG00007,res_kun.IMG00008,res_kun.IMG00009,res_kun.IMG00010,res_kun.IMG00011,res_kun.IMG00012,res_kun.IMG00013,res_kun.IMG00014,res_kun.IMG00015,res_kun.IMG00016,res_kun.IMG00017,res_kun.IMG00018,res_kun.IMG00019,res_kun.IMG00020,res_kun.IMG00021,res_kun.IMG00022]
    
    while True:
        key0_st = key0.value()
        if key0_st != 1:
            if key0_st == 0:
                break
        c+=1
        if c > 22:	c = 0
        oled.blit(img[c],0,0)
        oled.show()
        time.sleep_ms(100)
        
    return 0