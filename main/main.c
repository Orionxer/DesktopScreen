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

#include "debug_log.h"
#include "bsp_gpio.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "ft6336.h"
#include "screen.h"
#include "buzzer.h"
#include "display.h"
#include "flash.h"

/******************************************************
 * @brief   VSCode ESP-IDF 环境注意事项
 * @note    导入项目时，如果缺少.vscode文件夹，可以使用
 *          ESP-IDF: EXPLORER: COMMANDS中的 Advanced功能
 *          选择 Add .vscode subdirectory files生成
 * @note    VSCode中如果遇到了CMake文件错误(比如误触CMake Configure命令)，不管如何编译都无法清除错误
 *          CMake Error at CMakeLists.txt:7 (include):include could not find requested file:
 *          /tools/cmake/project.cmakeCMake (include)
 *          可以尝试Ctrl + Shift + P, 输入 CMake : Reset CMake Tools Extension State
 ******************************************************/

// 项目名称
#define PROJECT_NAME "\x1b[33m ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ ____ \n||D |||e |||s |||k |||t |||o |||p |||S |||c |||r |||e |||e |||n ||\n||__|||__|||__|||__|||__|||__|||__|||__|||__|||__|||__|||__|||__||\n|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|\n\x1b[0m\n"

void app_main(void)
{
    printf(PROJECT_NAME);
    // print_log_demo();
    // touch_ft6336_init();
    // screen_init();
    // display_task_init();
    // buzzer_init();
    flash_init();
    flash_write_single_wifi("My_SSID", "My_SSID");
    // flash_erase_all();
    while (1)
    {
        flash_read_single_wifi();
        vTaskDelay(pdMS_TO_TICKS(1000));
        // get_ft6336_touch_sta(&position);
        // test_toggle_pin();
        // print_task_stack_info();
    }
}
