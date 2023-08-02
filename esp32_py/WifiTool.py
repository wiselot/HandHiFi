import time
import network
import lightUI
from esp32 import NVS

wifi_list = []
wlan = 0
panel = 0

NVS_WIFI_STORAGE_NAMESPACE = const("spstorage")
NVS_WIFI_STORAGE_STAT_KEY = const("spstat")
NVS_WIFI_STORAGE_WIFI_SUFFIX = const("spwifi_")
NVS_WIFI_STORAGE_MAXNUM = const(32)

''' Wifi Tool '''
def menu_entry_wifi_tool(oled,key_map):
    global wifi_list
    global wlan,panel
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if wlan.isconnected():
        print("[Info] Wifi Tool: Already connected to wifi")
        panel.DialogInfo("Already connected.",5)
        print(wlan.ifconfig())
        return 0
    all_wifi_list = wlan.scan()
    
    panel = lightUI.lightUI(oled,key_map)
    if not len(all_wifi_list):
        print("[Error] Wifi Tool: Sorry,we cant get any wifi!")
        panel.DialogInfo("Sorry,we cant get any wifi!",10)
        return 1
        
    for wifi in all_wifi_list:
        wifi_list.append(wifi[0].decode('utf-8'))
    
    return panel.Chooser("Wifi Tool",wifi_list,chooser_callback)

def nvs_wifi_init():
    try:
        nvs = NVS(NVS_WIFI_STORAGE_NAMESPACE)
        nvs.set_i32(NVS_WIFI_STORAGE_STAT_KEY,0)
        nvs.commit()
    except Exception as e:
        print("[Error] Wifi Init: nvs init failed : return")
        print(e)
        return 1
    return 0

def get_bit_stat(stat,n):
    return (stat & (1 << n)) >> n
def del_bit_stat(stat,n):
    stat &= ~(1 << n)
def set_bit_stat(stat,n):
    stat |= (1 << n)

def get_passwd_nvs(ssid):
    nvs = NVS(NVS_WIFI_STORAGE_NAMESPACE)
    try:
        nvs.get_i32(NVS_WIFI_STORAGE_STAT_KEY)
    except OSError:
        if nvs_wifi_init():
            return None
    try:
        arr = [] * 2
        buf =  bytearray(128)
        stat = nvs.get_i32(NVS_WIFI_STORAGE_STAT_KEY)
        for i in range(0,NVS_WIFI_STORAGE_MAXNUM):
            if get_bit_stat(stat,i):
                try:
                    nvs.get_blob(NVS_WIFI_STORAGE_WIFI_SUFFIX + str(i) , buf)
                    arr = buf.decode('utf-8').split(' ')
                    if ssid == arr[0]:
                        return arr[1]
                except Exception as e:
                    print(e)
                    
    except Exception as e:
        print("[Error] NVS Get: Get wifi failed : return.")
        return None
    return None
                    
def chooser_callback(index):
    global wlan,panel
    wifi_ssid = wifi_list[index]
    print("[Info] Wifi Tool: User choose wifi " + wifi_ssid)
    if not wlan.isconnected():
        passwd = get_passwd_nvs(wifi_ssid)
        if passwd == None:
            print("[Error] Wifi Tool: Sorry,We can't get the password!")
            panel.DialogInfo("Sorry,We can't get the password!",10)
            return 1
        else:
            print("[Info] Wifi Tool: Connecting to wifi " + wifi_ssid)
            wlan.connect(wifi_ssid,passwd)
            delay = 15
            while not wlan.isconnected() and delay:
                time.sleep_ms(500)
                delay -= 1
                pass
            if not delay:
                print("[Error] Wifi tool: Sorry,We can't connect to the wifi!")
                panel.DialogInfo("Sorry,We can't connect to the wifi!",10)
                return 1
            else:
                print('[Info] Wifi Tool: Connected to ' + wifi_ssid)
                print(wlan.ifconfig())
                panel.DialogInfo("Connected to " + wifi_ssid,5)
                return 0
    else:
        print("[Info] Wifi Tool: Already connected to wifi")
        panel.DialogInfo("Already connected.",5)
        print(wlan.ifconfig())
    return 0