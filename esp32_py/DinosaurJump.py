''' 恐龙小游戏 '''
''' 参考 https://blog.csdn.net/zcy2333/article/details/125848327 '''

import machine
import random
import time

oled = 0

def menu_entry_DinosaurJump(s_oled,key_map):
    oled = s_oled
    
    ''' 退出 '''
    key0 = key_map[0][0]
    key0_st = key_map[0][1]
    ''' 暂停 '''
    key1 = key_map[1][0]
    key1_st = key_map[1][1]
    ''' 跳跃 '''
    key2 = key_map[2][0]
    key2_st = key_map[2][1]
    ''' 加快 '''
    key3 = key_map[3][0]
    key3_st = key_map[3][1]
    
    get_time = time.time()
    game_time = time.time()
    jump = 0
    jump_num = [0,8,15,21,25,29,32,34,35]
    down = False
    score = 0
    gameover = False
    pause = False
    far = 0
    speed = 2
    
    
    box1 = random.randint(40,120)
    box2 = random.randint(40,120) + box1
    box3 = random.randint(40,120) + box2
    
    while True:
        # exit
        key0_st = key0.value()
        if key0_st != 1:
            if key0_st == 0:
                break
        # pause
        key1_st = key1.value()
        if key1_st != 1:
            if key1_st == 0:
                pause = bool(1-pause)
        if pause:
            time.sleep(1/40)
            continue
        if not gameover:
            key3_st = key3.value()
            if key3_st != 1:
                if key3_st == 0:
                    speed += 1
            key2_st = key2.value()
            if key2_st != 1:
                if key2_st == 0 and jump == 0:
                    jump = 1
            if jump != 0:
                jump += 1 if not down else -1
                if jump == 0:
                    down = False
                    score += 1
            if jump == 8:
                down = True
            far += speed
            score = far // 30
            speed += score // 100
            
            oled.fill(0)
            oled.line(0,63,128,63,1)
            oled.text('SCORE:'+str(score),0,1)
            get_time = 'TIME:'+str(int(time.time()-game_time))
            oled.text(get_time,140-len(get_time)*10,1)
            
            #画小恐龙的外形
            for i in range(6):
                oled.line(15-i,51-jump_num[jump],15-i,58-jump_num[jump],1)
                oled.line(17-i,51-jump_num[jump],17-i,54-jump_num[jump],1)
                oled.line(11-i,55-jump_num[jump],11,58-jump_num[jump],1)
            oled.line(14,55-jump_num[jump],14,60-jump_num[jump],1)
            oled.line(10,55-jump_num[jump],10,60-jump_num[jump],1)
            oled.line(14,52-jump_num[jump],14,53-jump_num[jump],0)
            oled.line(13,52-jump_num[jump],13,53-jump_num[jump],0)
            
            #画障碍物
            for i in range(8):
                oled.line(i+box1-far,55,i+box1-far,61,1)
            
            for i in range(8):
                oled.line(i+box2-far,55,i+box2-far,61,1)
            
            for i in range(8):
                oled.line(i+box3-far,55,i+box3-far,61,1)
            #判断当前障碍物过了屏幕，就把它变最后一个
            if box1+8-far <= 0:
                box1 = box2 
                box2 = box3 
                box3 = box2 + random.randint(40,120)
            #判断是否碰到障碍物
            if 14 > box1-far > 2 and 60-jump_num[jump] > 55:
                gameover = True
        else:
            oled.text('GAME_OVER',128//2-35,30)
            #再次按下按钮初始化并重新开始
            key2_st = key2.value()
            if key2_st != 1:
                if key2_st == 0:
                    score = 0
                    game_time = time.time()
                    gameover = False
                    far = 0
                    box1 = random.randint(50,120)
                    box2 = random.randint(50,120) + box1
                    box3 = random.randint(50,120) + box2
                    speed = 2
                    continue
        time.sleep(1/40)
        oled.show()   
    return 0