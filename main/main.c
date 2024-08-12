#include <stdio.h>

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"

// 项目名称
#define PROJECT_NAME "\n\x1b[33m ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ \n||D |||e |||s |||k |||t |||o |||p |||S |||c |||r |||e |||e |||n ||\n||__|||__|||__|||__|||__|||__|||__|||__|||__|||__|||__|||__|||__||\n|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|\n\x1b[0m\n"
// 日志系统
#define DBG_LOGD(format, ...) printf(LOG_COLOR(LOG_COLOR_BLUE) "D (%" PRIu32 ") %s: " format LOG_RESET_COLOR "\n", (uint32_t)esp_log_timestamp(), __func__, ##__VA_ARGS__)
#define DBG_LOGI(msg, ...) ESP_LOGI(__func__, msg, ##__VA_ARGS__)
#define DBG_LOGW(msg, ...) ESP_LOGW(__func__, msg, ##__VA_ARGS__)
#define DBG_LOGE(msg, ...) ESP_LOGE(__func__, msg, ##__VA_ARGS__)

/**
 * @brief   打印FreeRTOS任务堆栈信息
 * @note    需要打开宏 
 *          configUSE_TRACE_FACILITY
 *          configUSE_STATS_FORMATTING_FUNCTIONS
 *          configGENERATE_RUN_TIME_STATS
 * @note    最好在menuconfig中启用/禁用宏
 */
void print_task_stack_info(void)
{
    // 获取当前所有任务的数量
    UBaseType_t task_count = uxTaskGetNumberOfTasks();

    // 为每个任务分配一个 TaskStatus_t 结构体数组
    TaskStatus_t *task_status_array = pvPortMalloc(task_count * sizeof(TaskStatus_t));

    if (task_status_array == NULL)
    {
        printf("Failed to allocate memory for task status array\n");
        return;
    }

    // 获取所有任务的状态信息
    UBaseType_t tasks_count = uxTaskGetSystemState(task_status_array, task_count, NULL);

    // 计算任务的总运行时间
    UBaseType_t total_runtime = 0;
    for (UBaseType_t i = 0; i < tasks_count; i++)
    {
        total_runtime += task_status_array[i].ulRunTimeCounter;
    }

    // 打印自定义的任务信息表头
    printf("\nTask Name\tState\t\tPriority\tRemaining Stack\t\tCPU Usage (%%)\n");
    printf("-----------------------------------------------------------------------------------------\n");

    // 遍历每个任务并打印详细信息
    for (UBaseType_t i = 0; i < tasks_count; i++)
    {
        const char *state_str = "";

        switch (task_status_array[i].eCurrentState)
        {
        case eRunning:
            state_str = "Running";
            break;
        case eReady:
            state_str = "Ready";
            break;
        case eBlocked:
            state_str = "Blocked";
            break;
        case eSuspended:
            state_str = "Suspended";
            break;
        case eDeleted:
            state_str = "Deleted";
            break;
        default:
            state_str = "Unknown";
            break;
        }

        float cpu_usage = (total_runtime > 0) ? ((float)task_status_array[i].ulRunTimeCounter / (total_runtime * portNUM_PROCESSORS)) * 100.0f : 0;
        uint8_t color = (i % 2) ? 31 : 32;
        printf("\x1b[%dm%-10s\t%-10s\t%u\t\t%lu bytes\t\t%.2f%%\x1b[0m\n",
               color,
               task_status_array[i].pcTaskName,
               state_str,
               task_status_array[i].uxCurrentPriority,
               task_status_array[i].usStackHighWaterMark * sizeof(StackType_t),
               cpu_usage); // CPU 占用率百分比
    }

    // 释放分配的内存
    vPortFree(task_status_array);
}

// 模拟计算负载的函数
void simulate_workload(unsigned int milliseconds)
{
    TickType_t start_time = xTaskGetTickCount();
    TickType_t end_time = start_time + pdMS_TO_TICKS(milliseconds);

    while (xTaskGetTickCount() < end_time)
    {
        // 空转，忙等待
    }
}

void app_main(void)
{
    printf(PROJECT_NAME);
    DBG_LOGD("Wecome to DesktopScreen");
    DBG_LOGI("Wecome to DesktopScreen");
    DBG_LOGW("Wecome to DesktopScreen");
    DBG_LOGE("Wecome to DesktopScreen");
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        print_task_stack_info();
    }
}
