#ifndef _APP_MQTT_H_
#define _APP_MQTT_H_

#include "esp_err.h"




#define CLOCK_QUERY 410
#define PARAMETER_QUERY 420
#define MUSIC_QUERY 450

#define CLOCK_SETTING 411
#define MUSIC_SETTING 451
#define MUSIC_VOL_SETTING 455
#define ADD_MUSIC_TO_LIST_SETTING 453
#define MUSIC_MODE_SETTING 456
#define MUSIC_DELET_SETTING 457


#define MUSIC_SINGLE_NOTICE 452
#define MUSIC_PLAYORPAUSE_NOTICE 454
#define CONTANT_SINGLE_NOTICE 461

#define MUSIC_URL_GET 230


#define CONTANT_REPLY 361
#define MUSIC_MODE_REPLY 356
#define MUSIC_DELETE_REPLY 357
#define MUSIC_ADD_REPLY 353
#define VOLUME_REPLY 355
#define CONTROL_PLAY_REPLY 354
#define SINGLE_MUSIC_REPLY 352
#define MUSIC_LIST_REPLY 351
#define CLOCK_LIST_REPLY 311

void app_mqtt_start(void);
int pp_mqtt_client_publish(char *command, char *data_buffer, int commandType, int pub_length, char *seq);
void pp_play_networkmusic_by_id(int num);
int pp_get_music_count();

void reple_single_music_play(char *type,int result, char *music_id, char *seq) ; //切歌
void reply_control_play(char *type, int result, int playing, char *seq); //暂停
void reply_control_vol(char *type,int result, int volume, char *seq);

#endif