#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "debug_log.h"

/**
 * @brief   不同等级日志的打印效果示例
 */
void print_log_demo(void)
{
    DBG_LOGD("Wecome to DesktopScreen");
    DBG_LOGI("Wecome to DesktopScreen");
    DBG_LOGW("Wecome to DesktopScreen");
    DBG_LOGE("Wecome to DesktopScreen");
    // 打印16进制
    uint8_t array[100] = {0};
    for (size_t i = 0; i < sizeof(array); i++)
    {
        array[i] = i;
    }
    print_hex_table(array, sizeof(array));
}

void print_hex_table(uint8_t *data, uint16_t len)
{
#if DBG_ENABLE
    // 打印标题行
    PRINT_ANSI_COLOR(ANSI_COLOR_CYAN);
    printf("     ");
    for (size_t i = 0; i < 0x10; i++)
    {
        printf("%02X ", (unsigned int)i);
    }
    PRINT_ANSI_COLOR(ANSI_COLOR_RESET);
    printf("\n");
	// 计算行数
	size_t rows = (len + 15) / 16; 
	for (size_t i = 0; i < rows; i++)
	{
		// 输出标题列
        PRINT_ANSI_COLOR(ANSI_COLOR_CYAN);
        printf("%04X ", (unsigned int)i * 16);
        // 输出数据列
        PRINT_ANSI_COLOR(ANSI_COLOR_MAGENTA);
		size_t j;
		for (j = 0; j < 16 && i * 16 + j < len; j++)
		{
			printf("%02X ", data[i * 16 + j]);
		}
		// 补齐空白列
		for (; j < 16; j++)
		{
			printf("   ");
		}
		printf("\n");
        PRINT_ANSI_COLOR(ANSI_COLOR_RESET);
	}
#endif
}

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