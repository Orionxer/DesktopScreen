#ifndef _BUZZER_H
#define _BUZZER_H

// 蜂鸣器提示音模式
typedef enum
{
    BUZZER_STOP,            // 停止蜂鸣器
    BUZZER_START_UP,        // 开机
    BUZZER_SINGLE_CLICK,    // 单击
    BUZZER_SUCCESS,         // 成功
    BUZZER_FAIL,            // 失败
    BUZZER_LOW_BAT,         // 低压
    BUZZER_MAX_RESERVED,    // 预留
}en_buzzer_type_t;

void buzzer_init(void);
void buzzer_play(en_buzzer_type_t type);
void buzzer_play_cycle(en_buzzer_type_t type);
void buzzer_stop(void);

#endif // _BUZZER_H
