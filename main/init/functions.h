#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

//wifi
/* 初始化wifi */
void wifi_init();
/*  连接wifi
*  连接模式:
*  0: 提供ssid和passwd
*  1: 从NVS获取已保存wifi(1:尝试第一个,2:全部尝试)
*  3: 通过图形化和按键输出选择
*/
int wifi_connect(int mode,uint8_t *ssid,uint8_t *passwd);
/* 初始化存储wifi */
int build_nvs_storage_wifi();
/* 存储wifi */
int store_wifi(uint8_t *ssid,uint8_t *passwd);
/* 列出所有wifi */
int list_store_wifi();
/* 删除wifi
 * 0<=n<32: 按位号
 * n<0 | n>=32 : 按ssid和passwd
 */
int del_wifi(uint8_t *ssid,uint8_t *passwd,int n);

#endif