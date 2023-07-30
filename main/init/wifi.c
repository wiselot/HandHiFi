#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "config.h"
#include "functions.h"

void wifi_init();
int wifi_connect(int mode,uint8_t *ssid,uint8_t *passwd);
int store_wifi(uint8_t *ssid,uint8_t *passwd);
int build_nvs_storage_wifi();
int list_store_wifi();
int del_wifi(uint8_t *ssid,uint8_t *passwd,int n);

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

static const char *TAG = "wifi station";

/* 存储空间名称 */
#define WIFI_STORAGE_NAMESPACE  "spstorage"
/* 最大存储wifi数 */
#define MAX_STORAGE_WIFI        32
/* 存储状态名称 */
#define WIFI_STORAGE_STAT_KEY   "spstat"

/* 删除置0 */
#define DEL_STORAGE_WIFI_STAT(stat,n)   ((stat)&=~(1<<(n)))
/* 添加置1 */
#define ADD_STORAGE_WIFI_STAT(stat,n)   ((stat)|=(1<<(n)))
/* 获取位 */
#define FER_STORAGE_WIFI_STAT(stat,n)   (((stat) >> (n)&1))

static struct storageWIFI{
    uint8_t ssid[32];
    uint8_t passwd[64];
}__attribute__ ((__packed__));

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static EventBits_t wifi_init_sta(uint8_t *ssid,uint8_t *passwd)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config={0};
    memcpy(wifi_config.sta.ssid,ssid,32);
    memcpy(wifi_config.sta.password,passwd,64);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // 定义事件组，获取wifi连接状态,WIFI_CONNECTED_BIT和WIFI_FAIL_BIT
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    // 检测事件
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ssid, passwd);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid, passwd);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    return bits;
}

void wifi_init(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
}

int wifi_connect(int mode,uint8_t *ssid,uint8_t *passwd){
    /*  连接模式:
     *  0: 提供ssid和passwd
     *  1: 从NVS获取已保存wifi(1:尝试第一个,2:全部尝试)
     *  3: 通过图形化和按键输出选择
     */
    if(mode==0){
        if(!ssid | !passwd){
            ESP_LOGE(TAG,"PROVIDE SSID AND PASSWD!");
            return 1;
        }
        wifi_init_sta(ssid,passwd);
    }
    else if(mode==1 || mode==2){
        nvs_handle_t nvs_handle;
        esp_err_t err;
        err = nvs_open(WIFI_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
        NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::namespace get failed");

        uint32_t stat;
        err = nvs_get_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,&stat);
        NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::stat get failed");

        // 读取成功
        struct storageWIFI st;
        size_t size = sizeof(struct storageWIFI);
        char key[16];
        int f = 0;
        int b = (mode==1)? 1: MAX_STORAGE_WIFI;
        for(int i=0;i<b ;i++){
            if(FER_STORAGE_WIFI_STAT(stat,i)){
                sprintf(key,"spwifi_%d",i);
                err = nvs_get_blob(nvs_handle,key,&st,&size);
                if(err==ESP_OK){
                    if(wifi_init_sta(st.ssid,st.passwd) & WIFI_CONNECTED_BIT){
                        printf("\033[0;35mConnect to wifi %s\n\033[0m",st.ssid);
                        f =1;
                        break;
                    }
                    else{
                        printf("\033[0;35mConnect to wifi %s failed!\n\033[0m",st.ssid);
                    }
                }
            }
        }
        nvs_close(nvs_handle);
        return !f;
    }
    else if(mode==3){
        #if GRAPH_CHOOSE_WIFI_ENABLE
            ESP_LOGW(TAG,"Graph mode is not provided.");
        #else
            ESP_LOGW(TAG,"Graph mode is not opened.See Config.h");
        #endif
    }
    else{
        ESP_LOGW(TAG,"UNEXPECTED Connect MODE");
        return 1;
    }
    return 0;
}

int build_nvs_storage_wifi()
// 初始化NVS存储wifi
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
/*
    err = nvs_flash_erase();
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE INIT FAILED!::flash erase failed");
*/
    err = nvs_open(WIFI_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE INIT FAILED!::namespace build failed");

    uint32_t stat = 0;
    err = nvs_set_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,stat);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE INIT FAILED!::stat build failed");

    err = nvs_commit(nvs_handle);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE INIT FAILED!::value store failed");
    return err!=ESP_OK;
}

int store_wifi(uint8_t *ssid,uint8_t *passwd)
// 未检测重复
{
    if(!ssid || !passwd)    return 1;
    nvs_handle_t nvs_handle;
    esp_err_t err;
    err = nvs_open(WIFI_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::namespace get failed");

    uint32_t stat;
    err = nvs_get_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,&stat);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::stat get failed");
    if(stat==0xffffffff){
        ESP_LOGW(TAG,"Unable to store wifi,the maxnum is 32!");
        return 1;
    }
    int i = 32;
    while(i--&&FER_STORAGE_WIFI_STAT(stat,32-i-1));
    //printf("i is =%d\n",i);
    char key[16];
    sprintf(key,"spwifi_%d",32-i-1);
    struct storageWIFI st;
    memcpy(st.ssid,ssid,32);
    memcpy(st.passwd,passwd,64);
    err = nvs_set_blob(nvs_handle,key,&st,sizeof(struct storageWIFI));
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::wifi store failed");

    ADD_STORAGE_WIFI_STAT(stat,32-i-1);
    err = nvs_set_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,stat);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::stat store failed");

    err = nvs_commit(nvs_handle);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::commit failed");
    nvs_close(nvs_handle);
    return 0;
}

int del_wifi(uint8_t *ssid,uint8_t *passwd,int n)
// 可以删除重复
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    err = nvs_open(WIFI_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::namespace get failed");

    uint32_t stat;
    err = nvs_get_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,&stat);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::stat get failed");

    int c = 0,f = 0;
    if(n>=0&&n<MAX_STORAGE_WIFI){
        DEL_STORAGE_WIFI_STAT(stat,n);
        err = nvs_set_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,stat);
        NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::stat store failed");
        c++;
    }
    else if(ssid&&passwd){
        struct storageWIFI st;
        size_t size = sizeof(struct storageWIFI);
        char key[16];
        for(int i=0;i<MAX_STORAGE_WIFI;i++){
            if(FER_STORAGE_WIFI_STAT(stat,i)){
                sprintf(key,"spwifi_%d",i);
                err = nvs_get_blob(nvs_handle,key,&st,&size);
                if(err==ESP_OK){
                    if(!strcmp((const char *)ssid,(const char *)st.ssid) && !strcmp((const char *)passwd,(const char *)st.passwd)){
                        DEL_STORAGE_WIFI_STAT(stat,i);
                        err = nvs_set_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,stat);
                        NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::stat store failed");
                        //break; // 可删除重复
                        c++;
                    }
                }
                else{
                    f++;
                    printf("Del WIFI(ssid:%s,passwd:%s)failed!\n",st.ssid,st.passwd);
                }
            }
        }
    }
    printf("DEL WIFI RESULT: found %d,failed %d\n",c,f);
    err = nvs_commit(nvs_handle);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::commit failed");
    nvs_close(nvs_handle);
    return c;
}

int list_store_wifi(){
    nvs_handle_t nvs_handle;
    esp_err_t err;
    err = nvs_open(WIFI_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::namespace get failed");
    
    uint32_t stat;
    err = nvs_get_u32(nvs_handle,WIFI_STORAGE_STAT_KEY,&stat);
    NO_ABORT_ERROR_CHECK(err,TAG,"WIFI NVS STORAGE VALUE FAILED!::stat get failed");
/*
    char s[32];
    itoa(stat,s,2);
    printf("STAT:%s\n",s);
*/
    struct storageWIFI st;
    size_t size = sizeof(struct storageWIFI);
    char key[16];
    for(int i=0;i<MAX_STORAGE_WIFI;i++){
        if(FER_STORAGE_WIFI_STAT(stat,i)){
            sprintf(key,"spwifi_%d",i);
            err = nvs_get_blob(nvs_handle,key,&st,&size);
            printf("STORED WIFI[%d]:",i);
            if(err==ESP_OK)
                printf("ssid:%s\tpasswd:%s\n",st.ssid,st.passwd);
            else
                printf("ERROR\n");
        }
    }
    nvs_close(nvs_handle);
    return 0;
}