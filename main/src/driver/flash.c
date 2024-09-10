#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "flash.h"
#include "debug_log.h"

stc_wifi_t wifi_info = {0};


void flash_init(void)
{
    DBG_LOGD("Initializing Flash");
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void flash_write_single_wifi(char *ssid, char *password)
{
    DBG_LOGD("Writing Flash");
    esp_err_t err;
    nvs_handle_t nvs_handle;
    err = nvs_open("wificonfig", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        DBG_LOGE("Error (%s) opening NVS handle", esp_err_to_name(err));
        return;
    }
    wifi_info.saved_flag = 1;
    strcpy(wifi_info.ssid, ssid);
    strcpy(wifi_info.password, password);
    ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, "wifi_saved_flag", wifi_info.saved_flag));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ssid", wifi_info.ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "password", wifi_info.password));
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
}

uint8_t flash_read_single_wifi(void)
{
    DBG_LOGD("Reading Flash");
    esp_err_t err;
    nvs_handle_t nvs_handle;
    err = nvs_open("wificonfig", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        DBG_LOGE("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return false;
    }
    err = nvs_get_u8(nvs_handle, "wifi_saved_flag", &wifi_info.saved_flag);
    switch (err)
    {
        case ESP_OK:
            // DBG_LOGD("Done");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            DBG_LOGW("No WiFi Information Storaged(Not Initialized Yet)");
            return false;
        default:
            DBG_LOGE("Error (%s) reading!", esp_err_to_name(err));
            return false;
    }
    if (wifi_info.saved_flag)
    {
        size_t ssid_len = sizeof(wifi_info.ssid);
        size_t password_len = sizeof(wifi_info.password);
        ESP_ERROR_CHECK(nvs_get_str(nvs_handle, "ssid", wifi_info.ssid, &ssid_len));
        ESP_ERROR_CHECK(nvs_get_str(nvs_handle, "password", wifi_info.password, &password_len));
        DBG_LOGI("WiFi Information Storaged, SSID = %s, Password = %s", wifi_info.ssid, wifi_info.password);
    }
    else
    {
        DBG_LOGW("No WiFi Information Storaged");
    }
    nvs_close(nvs_handle);
    return wifi_info.saved_flag;
}

stc_wifi_t get_wifi_info(void)
{
    return wifi_info;
}

void flash_erase_all(void)
{
    DBG_LOGW("Erasing ALL Flash");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
}
