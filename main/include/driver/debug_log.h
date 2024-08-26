#ifndef _DEBUG_LOG_H
#define _DEBUG_LOG_H

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"

// [x] 添加整体打印开关
// [x] 添加等级宏控制
// [x] 添加16进制数据打印(需要优化换行显示)

/**
 * @brief   尽可能确保显示的终端支持ANSI的转移序列以及UTF-8的显示
 * @note    当终端不支持ANSI的转移序列，则无法正常显示颜色信息，
 *          可以通过设置宏COLOR_ENABLE为0关闭颜色输出
 * @note    当终端不支持UTF-8，则关于中文的显示会乱码
*/

/************************** 用户自定义宏 *********************************/
/**
 * @brief   调试输出总开关
 * @note    处于禁用状态时关闭所有输出
*/
#define DBG_ENABLE          1

/**
 * @brief   是否启用颜色输出
 * @note    如果显示的终端不支持ANSI转义序列，可以将此项设置为0以关闭颜色输出
*/
#define COLOR_ENABLE        1

/**
 * @brief   指定 输出打印等级
 * @note    1. 只会打印当前等级及其以上的等级的信息
 * @note    2. 假设 指定的打印等级 = DBG_LOG_WARNING,
 *          则只会打印 DBG_LOG_WARNING 和 DBG_LOG_ERROR两个等级的信息
*/
#define DBG_LOG_LEVEL       DBG_LOG_WARNING

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_RESET    "\x1b[0m"

#if COLOR_ENABLE
#define PRINT_ANSI_COLOR(...) printf(__VA_ARGS__)
#else
#define PRINT_ANSI_COLOR(...)
#endif


// 调试输出总开关 处于打开状态
#if DBG_ENABLE
/*****************************************
 * @brief   日志系统
 ****************************************/
// [调试]等级控制
#if DBG_LOG_LEVEL >= DBG_LOG_DEBUG
#define DBG_LOGD(format, ...) printf(LOG_COLOR(LOG_COLOR_BLUE) "D (%" PRIu32 ") %s: " format LOG_RESET_COLOR "\n", (uint32_t)esp_log_timestamp(), __func__, ##__VA_ARGS__)
#else
#define DBG_LOGD(...)
#endif

// [普通]等级控制
#if DBG_LOG_LEVEL >= DBG_LOG_INFO
#define DBG_LOGI(msg, ...) ESP_LOGI(__func__, msg, ##__VA_ARGS__)
#else
#define DBG_LOGI(...)
#endif

// [警告]等级控制
#if DBG_LOG_LEVEL >= DBG_LOG_WARNING
#define DBG_LOGW(msg, ...) ESP_LOGW(__func__, msg, ##__VA_ARGS__)
#else
#define DBG_LOGW(...)
#endif

// [错误]等级控制
#if DBG_LOG_LEVEL >= DBG_LOG_ERROR
#define DBG_LOGE(msg, ...) ESP_LOGE(__func__, msg, ##__VA_ARGS__)
#else
#define DBG_LOGE(...)
#endif

#else
#define DBG_LOGD(...)
#define DBG_LOGI(...)
#define DBG_LOGW(...)
#define DBG_LOGE(...)
#endif


void print_log_demo(void);

/**
 * @brief   以16进制打印数据
 * @param   [in] data   打印的数据数据
 * @param   [in] len    数据长度 
 * @note    输出格式 = %02X
*/
void print_hex_table(uint8_t *data, uint16_t len);

// 打印任务堆栈信息
void print_task_stack_info(void);

#endif // _DEBUG_LOG_H