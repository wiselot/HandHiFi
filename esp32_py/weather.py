''' weather tool '''
import urequests
import lightUI

city_req = ['nanjing','beijing']
oled = 0
panel = 0
OLED_FONT_HEIGT = const(8)
OLED_FONT_WIDTH = const(8)

def menu_entry_weather(s_oled,key_map):
    global oled,panel
    oled = s_oled
    panel = lightUI.lightUI(oled,key_map)
    return panel.Chooser("Weather",city_req,weather_callback)

def weather_callback(index):
    print("[Info] Weather Tool : User choose " + city_req[index])
    url = 'https://api.seniverse.com/v3/weather/now.json?key=SK6E_7MBhPJr0_Cs3&location=' + city_req[index] + '&language=en&unit=c'
    try:
        r = urequests.get(url)
        arr = r.json()['results'][0]['location']['name'],r.json()['results'][0]['now']['temperature'],r.json()['results'][0]['now']['text']
        city = arr[0]
        temp = arr[1]
        weather = arr[2]
    except Exception as e:
        print("[Error] Weather Tool: We can't get weather data,check your internet connection!")
        print(e)
        panel.DialogInfo("We can't get weather data,check your internet connection!",10)
        return 1
    try:
        oled.fill(0)
        oled.text("Weather:",0,0,1)
        oled.text("City: " + city,0,OLED_FONT_HEIGT,1)
        oled.text("Temp: " + temp + " degree",0,OLED_FONT_HEIGT*2,1)
        oled.text("weather: " + weather,0,OLED_FONT_HEIGT*3,1)
        
        panel.graphTimerWaitExit(0,OLED_FONT_HEIGT*4,1)
    except Exception as e:
        print("[Error] Weather Tool: We can't display weather data")
        print(e)
        return 1
    return 0
