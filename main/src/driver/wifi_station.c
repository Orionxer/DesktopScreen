#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_mac.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi_station.h"
#include "debug_log.h"
#include "flash.h"

// ** 重连5次依然连不上，需要检查SSID和Password是否正确 或 WIFI距离太远 或 加密方式不对(比如WPA3)
// WiFi最大重连次数
#define WIFI_RETRY_MAX_TIMES    5
// 测试 SSID
#define WIFI_EXAMPLE_SSID   "VTK"
// 测试 Password
#define WIFI_EXAMPLE_PASSWORD   "AA12345678@"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define ESPTOUCH_DONE_BIT  BIT2

// SmartConfig的账号密码缓存
uint8_t sc_ssid[33] = {0};
uint8_t sc_password[65] = {0};

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;

/**
 * @brief WiFi连接状态回调函数
 * @param arg 
 * @param event_base 
 * @param event_id 
 * @param event_data 
 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // esp_wifi_connect();
        DBG_LOGI("WiFi Station Start");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < WIFI_RETRY_MAX_TIMES)
        {
            esp_wifi_connect();
            s_retry_num++;
            DBG_LOGW("retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        DBG_LOGE("connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        DBG_LOGI("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        DBG_LOGI("Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        DBG_LOGI("Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        DBG_LOGI("Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            DBG_LOGI("Set MAC address of target AP: " MACSTR " ", MAC2STR(evt->bssid));
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        // 缓存SmartConfig回调的SSID与密码，在配网任务成功时写入Flash
        memcpy(sc_ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(sc_password, evt->password, sizeof(evt->password));
        DBG_LOGI("SSID = %s, PASSWORD = %s", ssid, password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            DBG_LOGI("RVD_DATA:");
            print_hex_table(rvd_data, sizeof(rvd_data));
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

void wifi_init_station(void)
{
    // [x] 合并nvs分支后移除Flash初始化代码
    DBG_LOGD("Initializing WiFi Station");

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    // SmartConfig
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    DBG_LOGD("wifi_init_sta finished.");
}

// WIFI信号强度
typedef enum
{
    WIFI_RSSI_EXCELLENT,    // > -50 dBm
    WIFI_RSSI_GOOD,         // -50 to -60 dBm
    WIFI_RSSI_FAIR,         // -60 to -70 dBm
    WIFI_RSSI_WEAK,         // < -70 dBm
}en_wifi_rssi_quality_t;

// TODO 根据RSSI更新对应的WiFi图标
void wifi_read_rssi(void)
{
    int rssi = 0;
    esp_err_t err = esp_wifi_sta_get_rssi(&rssi);
    if (err == ESP_OK)
    {
        en_wifi_rssi_quality_t quality;
        if (rssi > -50)
        {
            quality = WIFI_RSSI_EXCELLENT;
        }
        else if (rssi <= -50 && rssi > -60)
        {
            quality = WIFI_RSSI_GOOD;
        }
        else if (rssi <= -60 && rssi > -70)
        {
            quality = WIFI_RSSI_FAIR;
        }
        else
        {
            quality = WIFI_RSSI_WEAK;
        }
        DBG_LOGI("The RSSI is %d dBm, quality is %d", rssi, quality);
    }
    else
    {
        DBG_LOGW("Failed to Read RSSI");
    }
}

uint8_t wifi_connect(char *ssid, char *password)
{
    DBG_LOGD("WiFi Connecting");
    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    strcpy((char *)wifi_config.sta.sae_h2e_identifier, "");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();


    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        DBG_LOGI("connected to ap SSID:%s password:%s", WIFI_EXAMPLE_SSID, WIFI_EXAMPLE_PASSWORD);
        return 1;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        DBG_LOGW("Failed to connect to SSID:%s, password:%s", WIFI_EXAMPLE_SSID, WIFI_EXAMPLE_PASSWORD);
    }
    else
    {
        DBG_LOGE("UNEXPECTED EVENT");
    }
    return 0;
}

void wifi_disconnect(void)
{
    DBG_LOGD("WiFi Disconnecting");
    esp_wifi_disconnect();
}

uint8_t wifi_read_state(void)
{
    // DBG_LOGD("Reading WiFi State");
    wifi_ap_record_t ap_info = {0};
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK)
    {
        DBG_LOGD("WiFi Connected: %s", ap_info.ssid);
        return 1;
    }
    else if (err == ESP_ERR_WIFI_NOT_CONNECT)
    {
        return 0;
    }
    return 0;
}

void wifi_test_flash(void)
{
    // 尝试读取Flash WiFi信息
    if (flash_read_single_wifi())
    {
        stc_wifi_t wifi = get_wifi_info();
        // DBG_LOGI("SSID = %s, Password = %s", wifi.ssid, wifi.password);
        wifi_connect(wifi.ssid, wifi.password);
    }
    else
    {
        // DBG_LOGW("Empty WiFi, will not connect");
        DBG_LOGW("Empty WiFi, Will Start SmartConfig");
        wifi_test_smartconfig();
    }
}

static void smartconfig_example_task(void * parm)
{
    DBG_LOGI("Start SmartConfig...");
    memset(sc_ssid, 0, sizeof(sc_ssid));
    memset(sc_password, 0, sizeof(sc_password));
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    // TODO 增加配网超时1分钟，以及配网失败的提示
    // TODO 配网使用小程序
    while (1)
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (uxBits & WIFI_CONNECTED_BIT)
        {
            DBG_LOGI("WiFi Connected to ap");
            // [x] 配网成功则写入将WiFi信息写入Flash
            flash_write_single_wifi((char *)sc_ssid, (char *)sc_password);
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            DBG_LOGD("smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

void wifi_test_smartconfig(void)
{
    xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
}

