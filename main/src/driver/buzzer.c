#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "buzzer.h"
#include "debug_log.h"

// 计算数组长度
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct
{
    int frequency; // 频率，单位Hz
    int time;      // 时长, 单位ms
}stc_buzzer_tone_t;


typedef struct
{
    const stc_buzzer_tone_t *array;
    int map_size;
}stc_buzzer_map_t;

typedef struct
{
    const stc_buzzer_map_t *map_ptr;
    int map_size;
    bool cycle_play;
    int index;
    int frequency;
    int time;
}stc_buzzer_player_t;

// 蜂鸣器播放实例
stc_buzzer_player_t g_buzzer = {0};

// 停止蜂鸣器
const stc_buzzer_tone_t tone_stop[] = 
{
    {0, 0},
};

// 上电提示音
const stc_buzzer_tone_t tone_start_up[] = 
{
    {5400, 180},
    {4760, 180},
    {4170, 180},
    {3510, 240},
};

// 成功提示音
const stc_buzzer_tone_t tone_success[] = 
{
    {2500, 600}
};

// 失败提示音
const stc_buzzer_tone_t tone_fail[] = 
{
    {2400, 300}, 
    {0, 60}, 
    {1500, 300}
};

// 单击提示音
const stc_buzzer_tone_t tone_click[] = 
{
    {2500, 100}
};

/**
 * @brief   蜂鸣器样式映射表
 * TODO     添加配网中提示
 */
const stc_buzzer_map_t buzzer_map[] =
{
    [BUZZER_STOP]           = {tone_stop, ARRAY_SIZE(tone_stop)},
    [BUZZER_START_UP]       = {tone_start_up, ARRAY_SIZE(tone_start_up)},
    [BUZZER_SINGLE_CLICK]   = {tone_click, ARRAY_SIZE(tone_click)},
    [BUZZER_SUCCESS]        = {tone_success, ARRAY_SIZE(tone_success)},
    [BUZZER_FAIL]           = {tone_fail, ARRAY_SIZE(tone_fail)},
};

// 定时器句柄
TimerHandle_t buzzer_timer_handle;

/**
 * @brief   蜂鸣器 软件定时器回调
 * @note    如果启用循环播放，在单个Tone播放完毕后停止60ms再开始播放
 */
void TimerCallback_Buzzer(TimerHandle_t param)
{
    if (g_buzzer.index < g_buzzer.map_size) // 说明可以播放下一个Tone
    {
        // 更新Tone
        g_buzzer.frequency = g_buzzer.map_ptr->array[g_buzzer.index].frequency;
        g_buzzer.time = g_buzzer.map_ptr->array[g_buzzer.index].time;
        // 驱动蜂鸣器
        // TODO bspPWM_BuzzerDriver(g_buzzer.frequency);
        DBG_LOGI("%dHz, %dms", g_buzzer.frequency, g_buzzer.time);
        // 重启定时器
        xTimerChangePeriod(param, pdMS_TO_TICKS(g_buzzer.time), 0);
        // 移动到下一个TONE
        g_buzzer.index++;
    }
    else // 播放完毕
    {
        // 驱动蜂鸣器 (停止)
        // TODO bspPWM_BuzzerDriver(0);
        DBG_LOGI("Buzzer Stop");
        if (g_buzzer.cycle_play) // 如果设置了循环播放
        {
            // 60ms后重新播放当前蜂鸣器样式
            xTimerChangePeriod(param, pdMS_TO_TICKS(60), 0);
            g_buzzer.index = 0;
        }
        else
        {
            memset(&g_buzzer, 0, sizeof(g_buzzer));
            xTimerStop(param, 0);
        }
    }
}

/**
 * @brief   初始化蜂鸣器
 */
void buzzer_init(void)
{
    DBG_LOGI("Initializing Buzzer Driver");
    // 创建软件定时器
    buzzer_timer_handle = xTimerCreate("BuzzerTimer",    // 定时器名字
                          pdMS_TO_TICKS(60), // 定时器周期
                          pdFALSE,              // 定时器是自动重载的
                          (void *)0,           // 定时器的ID，这里为0
                          TimerCallback_Buzzer);     // 定时器到期时调用的回调函数
    if (buzzer_timer_handle)
    {
        // 测试播放
        buzzer_play(BUZZER_START_UP);
    }
}

/**
 * @brief   蜂鸣器开始播放(一次)
 * @param   [in] type 模式枚举
 */
void buzzer_play(en_buzzer_type_t type)
{
    // 停止buzzer定时器
    xTimerStop(buzzer_timer_handle, 0);
    memset(&g_buzzer, 0, sizeof(g_buzzer));
    if (type >= BUZZER_MAX_RESERVED)
    {
        return;
    }
    // 设置全局buzzer
    g_buzzer.index = 0;
    // 获取并存储有效的Tone起始地址
    g_buzzer.map_ptr = &buzzer_map[type];
    // 计算实际需要播放的大小
    g_buzzer.map_size = buzzer_map[type].map_size;
    // 获取频率与时间(已经得到有效的起始地址以及对应的偏移位置)
    g_buzzer.frequency = g_buzzer.map_ptr->array[g_buzzer.index].frequency;
    g_buzzer.time = g_buzzer.map_ptr->array[g_buzzer.index].time;
    // 驱动蜂鸣器
    // TODO bspPWM_BuzzerDriver(g_buzzer.frequency);
    DBG_LOGI("%dHz, %dms", g_buzzer.frequency, g_buzzer.time);
    // 如果第一个频率为0默认关闭蜂鸣器
    if (g_buzzer.frequency > 0)
    {
        // 启动定时器
        xTimerChangePeriod(buzzer_timer_handle, pdMS_TO_TICKS(g_buzzer.time), 0);
        // 移动到下一个TONE
        g_buzzer.index++;
    }
    else
    {
        memset(&g_buzzer, 0, sizeof(g_buzzer));
    }
}

/**
 * @brief   蜂鸣器循环播放
 * @param   [in] type 模式枚举
 * @note    循环播放的的间隔待定
 */
void buzzer_play_cycle(en_buzzer_type_t type)
{
    // TODO 待定
}


/**
 * @brief   停止播放
 */
void buzzer_stop(void)
{
    buzzer_play(BUZZER_STOP);
}

