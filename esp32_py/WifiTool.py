import time
import network
import lightUI
from esp32 import NVS

wifi_list = []
wlan = 0
panel = 0

''' Wifi Tool '''
def menu_entry_wifi_tool(oled,key_map):
    global wifi_list
    global wlan,panel
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    all_wifi_list = wlan.scan()
    
    panel = lightUI.lightUI(oled,key_map)
    if not len(all_wifi_list):
        print("Sorry,we cant get any wifi!")
        panel.DialogInfo("Sorry,we cant get any wifi!")
        return 1
        
    for wifi in all_wifi_list:
        wifi_list.append(str(wifi[0]))
    
    return panel.Chooser("Wifi Tool",wifi_list,chooser_callback)

def get_passwd_nvs(ssid):
    if ssid == "b'TP-LINK_ACF5'":
        return "13967187659"
    elif ssid == "b'WISELOT-WIFI'":
        return "k7omain%"
    else:
        return None
def chooser_callback(index):
    global wlan,panel
    print("Choose wifi " + wifi_list[index])
    if not wlan.isconnected():
        passwd = get_passwd_nvs(str(wifi_list[index]))
        if passwd == None:
            print("Sorry,We can't get the password!")
            panel.DialogInfo("Sorry,We can't get the password!")
            return 1
        else:
            print("Connecting to wifi " + wifi_list[index])
            wlan.connect(wifi_list[index],passwd)
            delay = 15
            while not wlan.isconnected() and delay:
                time.sleep_ms(500)
                delay -= 1
                pass
            if not delay:
                print("Sorry,We can't connect to the wifi!")
                panel.DialogInfo("Sorry,We can't connect to the wifi!")
                return 1
            else:
                print('network config:', wlan.ifconfig())
                panel.DialogInfo("Connect to " + str(wifi_list[index]) + "success!")
                return 0
    else:
        print("Already wifi connected.")
    return 0