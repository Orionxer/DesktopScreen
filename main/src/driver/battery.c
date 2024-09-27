#include <string.h>
#include <stdint.h>
#include "debug_log.h"

/************************************************************************
 * @brief   [锂电池]相关定义
 * @note    根据实际情况修改R1和R2的值
 ***********************************************************************/
// 分压 上臂电阻 10KΩ
#define R1                                  10000
// 分压 下臂电阻 10KΩ
#define R2                                  10000
// 分压比
#define VOLTAGE_RATE ((((R1 + R2)) * 1.0) / R2)

// 计算数组长度
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// 电量电压关系结构体
typedef struct
{
    uint8_t     level;      // 电量
    uint32_t    voltage;    // 电压(mv)
}stc_battery_ocv_t;

/**
 * @brief   锂电池电量电压映射表
 * @note    锂电池的电量估算方式就是在两点之间线性插值
 * @note    分段函数越多则越接近真实的电量电压曲线
 * @ref     https://blog.csdn.net/Xhw3f586/article/details/135735061
 */
const stc_battery_ocv_t ocv_map[] = 
{
    {100,   4200},
    {90,    4060},
    {80,    3980},
    {70,    3920},
    {60,    3870},
    {50,    3820},
    {40,    3790},
    {30,    3770},
    {20,    3740},
    {10,    3680},
    {5,     3450},
    {0,     3000},
};

/************************************************************************
 * @brief   [锂电池]根据电压估算电量 
 * @param   [in] adc_voltage    ADC引脚采样电压，单位mV
 * @note    锂电池标称电压3.7V，满电电压4.2V，超过ADC采样电压的极限，因此需要
 *          经过[分压电路]才能安全采集锂电池电压
 * @note    普通锂电池放电截止电压范围: 2.75V 至 3V
 * @return  battery_level   电量百分比: 0-100
 ***********************************************************************/
uint8_t estimate_battery_level(uint32_t adc_voltage)
{
    // ADC 采样电压需要进行分压计算
    uint32_t vbat_mv = adc_voltage * VOLTAGE_RATE;
    // 如果电压超过3.3V，则正向偏移30mV
    if (vbat_mv > 3300)
    {
        vbat_mv += 30;
    }
    DBG_LOGD("The battery voltage is : %dmV\n", (int)vbat_mv);
    int array_size = ARRAY_SIZE(ocv_map);
    // 如果电压值超过最大电压
    if (vbat_mv >= ocv_map[0].voltage)
    {
        return 100;
    }
    // 如果电压值小于最低电压
    if (vbat_mv <= ocv_map[array_size - 1].voltage)
    {
        return 0;
    }
    // 锂电池的电量估算方式就是在两点之间线性插值
    for (size_t i = 1; i < array_size; i++)
    {
        // 记录第一个点
        uint8_t prev_level = ocv_map[i - 1].level;
        uint32_t prev_voltage = ocv_map[i - 1].voltage;
        // 如果电池电压大于第二个点，说明电池电压在两点之间
        if (vbat_mv > ocv_map[i].voltage)
        {
            // 计算两点之间的斜率
            float slope = (float)(prev_level - ocv_map[i].level) / (prev_voltage - ocv_map[i].voltage);
            // 计算y轴截距
            float intercept = prev_level - prev_voltage * slope;
            // 根据斜率、截距以及x坐标计算y坐标
            float level = slope * vbat_mv + intercept;
            // printf("The level is %f", level);
            return (uint8_t)level;
        }
        // 小于则继续循环
    }
    return 0;
}
