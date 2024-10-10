#ifndef _APP_TASK_H_
#define _APP_TASK_H_

#include "esp_err.h"



typedef struct __player_message {
    enum {
        PLAYER_CMD_PLAYMUSIC,
        PLAYER_CMD_PLAYRAW,
    } id;
    char url[500];
    unsigned char raw_data[1024];
    int  level;
    int raw_length;
} player_msg_t;





typedef enum {
    MUSICIDEAL,
    MUSICPLAYING,
    MUSICPAUSE
} music_status_t;

typedef struct __file_message {
    int msg;
    char *write_buffer;
    int write_len;
} file_msg_t;



int file_init();
int file_save_recorde(char *buffer,int len);
int file_finish_recorde();
void file_stop_recorde();
int pp_music_play(char *url, uint8_t level,int interrupt_flag);
int pp_raw_play(unsigned char *data, int level,int interrupt_flag);

void pp_music_init();
void http_getNetwork_time();
int pp_musicList_play();
int pp_networkmusicList_play();
void wakeup_tone(int inter);
int file_task_init();
int pp_file_cmd(int n, int interrupt_flag);
int pp_file_write(char *buffer, int len);
void pp_led_init();
void led_blink_onoff(int n);
void pp_playControl(int n);
void pp_setVolNum(int n);
void pp_stop_musicList_play();
void pp_stop_networkmusicList_play();
#endif