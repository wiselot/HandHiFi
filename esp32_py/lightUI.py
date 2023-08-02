from ssd1306 import SSD1306_I2C
import time
import framebuf

OLED_FONT_HEIGT = const(8)
OLED_FONT_WIDTH = const(8)

class lightUI:
    def __init__(self,oled,key_map):
        self.oled = oled
        self.s_width = oled.width
        self.s_height = oled.height
        self.key_map = key_map
        self.key0 = key_map[0][0]
        self.key0_st = key_map[0][1]
        self.key1 = key_map[1][0]
        self.key1_st = key_map[1][1]
        self.key2 = key_map[2][0]
        self.key2_st = key_map[2][1]
        self.key3 = key_map[3][0]
        self.key3_st = key_map[3][1]
    def Chooser(self,title,menu_list,callback,click=0):
        '''
        实现可翻页的选择UI
        '''
        u = OLED_FONT_HEIGT+1
        page_size = (self.s_height - OLED_FONT_HEIGT -1)// (2 + OLED_FONT_HEIGT)
        page_num = len(menu_list) // page_size + 1
        page = 0
        on_page = min(len(menu_list)-page*page_size,page_size)
        turn = 1
        turn_s = 0
        turn_p = 0
        choose = 0
        k = 0
        ret = 0
        self.oled.fill(0)
        self.oled.text(title,max((self.s_width - len(title)*OLED_FONT_WIDTH)//2,0),0,1)
        self.oled.show()
        while True:
            # exit
            self.key0_st = self.key0.value()
            if self.key0_st != 1:
                if self.key0_st == 0:
                    break
            # page
            self.key1_st = self.key1.value()
            if self.key1_st != 1:
                if self.key1_st == 0:
                    page+=1
                    turn = 1
                    if page >= page_num:
                        page = 0
            # next
            self.key2_st = self.key2.value()
            if self.key2_st != 1:
                if self.key2_st == 0:
                    turn_s = 1
                    choose+=1
                    if choose >= on_page:
                        choose = 0
            # enter
            self.key3_st = self.key3.value()
            if self.key3_st != 1:
                if self.key3_st == 0:
                    turn_p = 1
            if turn_p:
                ret |= callback(page*page_size + choose)
                if ret:
                    print("Menu List return err code " + str(ret))
                if click:
                    return ret
                # repaint
                self.oled.fill(0)
                self.oled.text(title,max((self.s_width - len(title)*OLED_FONT_WIDTH)//2,0),0,1)
                self.oled.show()
                turn = 0
                turn_p = 0
            if turn_s:
                self.oled.fill_rect(0,0,2*OLED_FONT_WIDTH,OLED_FONT_HEIGT,0)
                self.oled.text('*' + str(choose),0,0,1)
                self.oled.show()
                turn_s = 0
            if turn:
                self.oled.fill_rect(0,u,self.s_width,self.s_height,0)
                for i in range(page*page_size,page*page_size + min(len(menu_list)-page*page_size,page_size)):
                    self.oled.line(0,u,self.s_width,u)
                    u+=2
                    self.oled.text(str(k) + " " + menu_list[i],4,u,1)
                    u+=OLED_FONT_HEIGT
                    k+=1
                on_page = min(len(menu_list)-page*page_size,page_size)
                u = OLED_FONT_HEIGT+1
                k = 0
                turn = 0
                choose = 0
                self.oled.show()
            time.sleep_ms(100)
        return ret
    def DialogInfo(self,text,clock=60):
        '''
        带翻页的信息展示UI
        '''
        dialogsx = self.s_width // 12
        dialogsy = self.s_height // 6
        dialogex = dialogsx * 11
        dialogey = dialogsy * 5
        dialogcwi = dialogex - dialogsx
        dialogche = dialogey - dialogsy
        
        self.oled.line(dialogsx,dialogsy,dialogex,dialogsy)
        self.oled.line(dialogsx,dialogsy,dialogsx,dialogey)
        self.oled.line(dialogex,dialogsy,dialogex,dialogey)
        self.oled.line(dialogsx,dialogey,dialogex,dialogey)
        #self.oled.rect(dialogsx,dialogsy,dialogex,dialogey)
        self.oled.fill(0)
        self.oled.text("*Info*",max(dialogcwi - OLED_FONT_WIDTH * 6,0),dialogsy + 1,1)
        self.oled.line(dialogsx,dialogsy + OLED_FONT_HEIGT + 1,dialogex,dialogsy + OLED_FONT_HEIGT + 1)
        self.oled.show()
        line_size = dialogcwi // OLED_FONT_WIDTH
        col_size = dialogche // OLED_FONT_HEIGT - 1
        col_num = len(text) // line_size + 1
        turn = 1
        scoll = 0
        ti = clock * 10
        ti_s = len(str(clock)) + 1
        while ti:
            # Timer
            self.oled.fill_rect(0,0,OLED_FONT_WIDTH * ti_s,OLED_FONT_HEIGT,0)
            self.oled.text(str(ti//10) + "s" ,0,0,1)
            self.oled.show()
            ti -= 1
            # scoll
            self.key2_st = self.key2.value()
            if self.key2_st != 1:
                if self.key2_st == 0:
                    turn = 1
            if turn:
                self.oled.fill_rect(dialogsx + 1,dialogsy + OLED_FONT_HEIGT + 2,dialogex - 1,dialogey - 1,0)
                for i in range(0,col_size):
                    self.oled.text(text[(i+scoll)*line_size:(scoll+i+1)*line_size],dialogsx + 1,dialogsy + OLED_FONT_HEIGT*(i+1) + 2)
                self.oled.show()
                turn = 0
                scoll += 1
                if scoll >= col_num: scoll = 0
            # enter
            self.key3_st = self.key3.value()
            if self.key3_st != 1:
                if self.key3_st == 0:
                    break
            time.sleep_ms(100)
        
        self.oled.show()
    def graphTimerWaitExit(self,x=0,y=0,key=1,delay=60):
        ti = delay * 10
        ti_s = len(str(delay)) + 1
        while ti:
            #key exit
            if key:
                self.key0_st = self.key0.value()
                if self.key0_st != 1:
                    if self.key0_st == 0:
                        break
            # Timer
            self.oled.fill_rect(x,y,OLED_FONT_WIDTH * ti_s,OLED_FONT_HEIGT,0)
            self.oled.text(str(ti//10) + "s" ,x,y,1)
            self.oled.show()
            ti -= 1
            time.sleep_ms(100)