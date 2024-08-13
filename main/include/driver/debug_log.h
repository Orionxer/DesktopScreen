#ifndef _DEBUG_LOG_H
#define _DEBUG_LOG_H

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"

// TODO 添加整体打印开关
// TODO 添加等级宏控制
// TODO 添加16进制数据打印(需要优化换行显示)

// 日志系统
#define DBG_LOGD(format, ...) printf(LOG_COLOR(LOG_COLOR_BLUE) "D (%" PRIu32 ") %s: " format LOG_RESET_COLOR "\n", (uint32_t)esp_log_timestamp(), __func__, ##__VA_ARGS__)
#define DBG_LOGI(msg, ...) ESP_LOGI(__func__, msg, ##__VA_ARGS__)
#define DBG_LOGW(msg, ...) ESP_LOGW(__func__, msg, ##__VA_ARGS__)
#define DBG_LOGE(msg, ...) ESP_LOGE(__func__, msg, ##__VA_ARGS__)


void print_log_demo(void);

// 打印任务堆栈信息
void print_task_stack_info(void);

#endif // _DEBUG_LOG_H