#ifndef _APP_SYSTEM_INFO_H_
#define _APP_SYSTEM_INFO_H_

#include "esp_err.h"




typedef struct clock_list
{
    uint8_t clock_action;  // 闹钟类型  1:定时闹钟;2:每日任务;3:每周任务;4:周末任务;5:工作日任务;
    uint8_t loopflag;      // 循环次数
    time_t nowClock;       // 闹钟时间
    char clock_time[20];   // 闹钟时间字符串
    char clock_name[20];   // 闹钟名字
}clock_list_t;


typedef struct __clock_message {
    enum {
        SYSTEM_CLOCK_INSTER,
        SYSTEM_CLOCK_DELETE,
    } id;
    clock_list_t clockList;
} clock_msg_t;

void pp_clock_init(void);
void pp_setSntp_time(void);
void get_current_time(void);
void clock_system_insert(uint8_t action, char *clock_info, char *name);
void rec_wakeup();
void http_setTime_init(char *time,int len) ;
void wakeup_rec_start(int n ,int inter);
esp_err_t i2c_led_init(void);
#endif