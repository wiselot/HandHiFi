#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

#include "init/functions.h"

void app_main(void)
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
    
    printf("\033[0;35mBoot infromation print down.Functions init...\n\033[0m");

    uint8_t ssid[32] = "TP-LINK_ACF5";
    uint8_t passwd[64] = "13967187659";

    uint8_t ssid1[32] = "WISELOT-WIFI";
    uint8_t passwd1[64] = "k7omain%";

    uint8_t ssid2[32] = "WISELOT-WIFI";
    uint8_t passwd2[64] = "k7omain%";

    uint8_t ssid3[32] = "ERRRM";
    uint8_t passwd3[64] = "12345678";
    wifi_init();
    //build_nvs_storage_wifi();
    //wifi_connect(0,ssid,passwd);
    //store_wifi(ssid,passwd);
    //store_wifi(ssid1,passwd1);
    //list_store_wifi();
    //del_wifi(ssid1,passwd1,-1);
    list_store_wifi();
    wifi_connect(2,NULL,NULL);
}