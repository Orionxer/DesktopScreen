#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "debug_log.h"
#include "ft6336.h"

/******************************************************
 * @brief   触摸相关GPIO
 *****************************************************/

// 触摸 输出引脚（复位）
#define GPIO_OUTPUT_TP_RST    5
#define GPIO_OUTPUT_TP_RST_PIN_SEL  (1ULL<<GPIO_OUTPUT_TP_RST)

// 触摸 输入引脚（中断）
#define GPIO_INTPUT_TP_INT    4
#define GPIO_INTPUT_TP_INT_PIN_SEL  (1ULL<<GPIO_INTPUT_TP_INT)


#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t tp_gpio_evt_queue = NULL;

static void IRAM_ATTR tp_gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(tp_gpio_evt_queue, &gpio_num, NULL);
}

static void tp_gpio_task(void* arg)
{
    uint32_t io_num;
    TP_POSITION_T position = {0};
    for (;;)
    {
        if (xQueueReceive(tp_gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            DBG_LOGI("GPIO[%" PRIu32 "] intr, val: %d", io_num, gpio_get_level(io_num));
            if (io_num == GPIO_INTPUT_TP_INT)
            {
                // TODO 重置休眠定时器
                if (gpio_get_level(io_num) == 0)
                {
                    // TODO 重置触摸状态
                    get_ft6336_touch_sta(&position);
                }
                else
                {
                    // TODO 检查触摸状态
                }
            }
        }
    }
}

void touch_gpio_init(void)
{
    // DBG_LOGD("GPIO Init");
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // ** 触摸输出引脚
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_OUTPUT_TP_RST_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    // ** 触摸输入引脚
    // 触摸中断 双边沿中断
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    // bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INTPUT_TP_INT_PIN_SEL;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // create a queue to handle gpio event from isr
    tp_gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    xTaskCreate(tp_gpio_task, "tp_gpio_task", 2048, NULL, 10, NULL);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INTPUT_TP_INT, tp_gpio_isr_handler, (void *)GPIO_INTPUT_TP_INT);

    DBG_LOGI("Minimum free heap size: %"PRIu32" bytes", esp_get_minimum_free_heap_size());
}

void set_tp_rst_level(uint8_t level)
{
    gpio_set_level(GPIO_OUTPUT_TP_RST, level);
}


/******************************************************
 * @brief   屏幕相关GPIO
 *****************************************************/
//屏幕片选 0-有效
#define SCREEN_GPIO_OUTPUT_CS 27
#define SCREEN_GPIO_OUTPUT_CS_SEL ((1ULL<<SCREEN_GPIO_OUTPUT_CS))
//屏幕数据/指令选择 1-data 0-cmd
#define SCREEN_GPIO_OUTPUT_DC 14
#define SCREEN_GPIO_OUTPUT_DC_SEL ((1ULL<<SCREEN_GPIO_OUTPUT_DC))
//屏幕复位 0-reset
#define SCREEN_GPIO_OUTPUT_RES 12
#define SCREEN_GPIO_OUTPUT_RES_SEL ((1ULL<<SCREEN_GPIO_OUTPUT_RES))
//屏幕状态 1-busy 
#define SCREEN_GPIO_INTPUT_BUSY 13
#define SCREEN_GPIO_INTPUT_BUSY_SEL ((1ULL<<SCREEN_GPIO_INTPUT_BUSY))

void screen_gpio_init(void)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = SCREEN_GPIO_OUTPUT_CS_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = SCREEN_GPIO_OUTPUT_DC_SEL;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = SCREEN_GPIO_OUTPUT_RES_SEL;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = SCREEN_GPIO_INTPUT_BUSY_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

}

void ds_gpio_set_screen_cs(uint32_t level)
{
    gpio_set_level(SCREEN_GPIO_OUTPUT_CS, level);
}

void ds_gpio_set_screen_dc(uint32_t level)
{
    gpio_set_level(SCREEN_GPIO_OUTPUT_DC, level);
}

void ds_gpio_set_screen_rst(uint32_t level)
{
    gpio_set_level(SCREEN_GPIO_OUTPUT_RES, level);
}

int ds_gpio_get_screen_busy()
{
    return gpio_get_level(SCREEN_GPIO_INTPUT_BUSY);
}

