#include "app_mqtt.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_bt_device.h"
#include "cJSON.H"
#include "app_task.h"


static const char *TAG = "app_mqtt_client";



#define MQTT_ADDRESS    "mqtt://183.220.37.43"     //MQTT连接地址
#define MQTT_PORT       1883                        //MQTT连接端口号
#define MQTT_CLIENT     "aiSoundBox_112233"              //Client ID（设备唯一）
#define MQTT_USERNAME   "digitalclient"                     //MQTT用户名
#define MQTT_PASSWORD   "tVcWyGXacali9feJ"                  //MQTT密码

char mac_buff[13] = {0};


#define BUFFER_SUBSCRIBE_TOPIC    "digital/oTN8xq3Nifk/%s/serverreply"      
#define BUFFER_SUBSCRIBE_TOPIC_QUERY   "digital/oTN8xq3Nifk/%s/query"
#define BUFFER_SUBSCRIBE_TOPIC_SETTING   "digital/oTN8xq3Nifk/%s/setting"
#define BUFFER_SUBSCRIBE_TOPIC_NOTICE   "digital/oTN8xq3Nifk/%s/notice"
#define BUFFER_SUBSCRIBE_TOPIC_SERVERREPLY   "digital/oTN8xq3Nifk/%s/serverreply"


char MQTT_SUBSCRIBE_TOPIC_QUERY[46] = {0};
char MQTT_SUBSCRIBE_TOPIC_SETTING[46] = {0};
char MQTT_SUBSCRIBE_TOPIC_NOTICE[46] = {0};
char MQTT_SUBSCRIBE_TOPIC_SERVERREPLY[46] = {0};



//MQTT客户端操作句柄
static esp_mqtt_client_handle_t     s_mqtt_client = NULL;


char *getproductId()
{
    return "oTN8xq3Nifk";
}
char *getdeviceId()
{
    snprintf(mac_buff, 13, ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));
    return mac_buff;
}

// /digital/{productId}/{deviceId}/notice
int get_public_topic_name(char *url, char *command)
{
    snprintf(url, 64, "digital/%s/%s/%s", getproductId(), getdeviceId(), command);
    return 0;
}


void reply_message(char *type, int result, cJSON *data, char *seq, int commandNum)
{
    cJSON *json = cJSON_CreateObject();
    // 向JSON对象中添加数据
    cJSON_AddNumberToObject(json, "result", result);
    
    // cJSON *data = cJSON_CreateObject();
    // cJSON_AddRawToObject(data, "music_id", music_id);

    cJSON_AddItemToObject(json, "data", data);

    // 将JSON对象转换为字符串
    char *json_string = cJSON_PrintUnformatted(json);
    
    pp_mqtt_client_publish(type, json_string, commandNum, strlen(json_string),seq);

    // 打印JSON字符串
    printf("reply_message = %s\n", json_string);
    if(json_string != NULL)
    {
        free(json_string);
        json_string = NULL;
    }
    // 释放JSON对象和字符串的内存
    cJSON_Delete(json);
}

void get_clock_info(char *seq)
{
    cJSON *json = cJSON_CreateObject();
    // 向JSON对象中添加数据
    
    cJSON *clockList = cJSON_CreateObject();
    
    cJSON_AddStringToObject(clockList, "clock_list", "[]");

    // 将JSON对象转换为字符串
    char *json_string = cJSON_Print(json);
    printf("351 = %s\n", json_string);

    pp_mqtt_client_publish("reply", json_string, 351, strlen(json_string), seq);

    // 打印JSON字符串
    // printf("%s\n", json_string);
    // 释放JSON对象和字符串的内存
    if(json_string != NULL)
    {
        free(json_string);
        json_string = NULL;
    }
    cJSON_Delete(json);

}


void get_music_info(char *seq)
{
    cJSON *json = cJSON_CreateObject();
    // 向JSON对象中添加数据
    
    
    cJSON *songList = cJSON_CreateObject();
    cJSON_AddNumberToObject(songList, "list_id", 52243);
    cJSON_AddStringToObject(songList, "music_list", "[]");

    cJSON_AddItemToObject(json, "songList", songList);

    cJSON_AddNumberToObject(json, "volume", 80);
    cJSON *music_playing = cJSON_CreateObject();
    cJSON_AddNumberToObject(music_playing, "play_mode", 0);
    cJSON_AddStringToObject(music_playing, "music_id", "");

    cJSON_AddItemToObject(json, "music_playing", music_playing);
    cJSON_AddNumberToObject(json, "loop_mode", 1);

    // 将JSON对象转换为字符串
    char *json_string = cJSON_Print(json);
    printf("350 = %s\n", json_string);

    pp_mqtt_client_publish("reply", json_string, 350, strlen(json_string), seq);


    // 打印JSON字符串
    // printf("%s\n", json_string);
    // 释放JSON对象和字符串的内存
    if(json_string != NULL)
    {
        free(json_string);
        json_string = NULL;
    }
    cJSON_Delete(json);

}



char g_networkmusic_list[50][15] = {0};
int g_current_networkMusic_num = -1;

int pp_get_music_count()
{
    for(int i=0;i<50;i++)
    {
        if(g_networkmusic_list[i][0]==NULL)
        {
            return i;
        }
    }
    return 0;
}

void pp_show_all_music_id()
{
    printf("MUSIC LIST : \n");
    for(int i=0;i<50;i++)
    {
        if(g_networkmusic_list[i][0]!=NULL)
        {
            printf("ID[%d] = %s\n", i, g_networkmusic_list[i]);
        }
    }
    printf("\n");
}
void pp_delete_all_music_list()
{
    memset(g_networkmusic_list, 0, strlen(g_networkmusic_list));
}

void pp_delete_music_by_name(char *name)
{
    int index = -1;
    for(int i=0;i<50;i++)
    {
        if(memcmp(g_networkmusic_list[i], name, strlen(name)) == 0)
        {
            memset(g_networkmusic_list[i], 0, strlen(g_networkmusic_list[i]));
        }
        index = i;
    }
    if(index != -1)
    {
        for(int i=index;i<50-1;i++)
        {
            if(g_networkmusic_list[i+1][0] == NULL)
            {
                break;
            }
            memset(g_networkmusic_list[i], 0, strlen(g_networkmusic_list[i]));
            memcpy(g_networkmusic_list[i], g_networkmusic_list[i+1], strlen(g_networkmusic_list[i+1]));
        }
    }
}



void pp_mqtt_require_networkMusic(char *music_id)
{
    cJSON *json = cJSON_CreateObject();
    // 向JSON对象中添加数据
    cJSON_AddStringToObject(json, "music_id", music_id);

    // 将JSON对象转换为字符串
    char *json_string = cJSON_PrintUnformatted(json);
    
    pp_mqtt_client_publish("request", json_string, 130, strlen(json_string), "5263234234");

    // 打印JSON字符串
    printf("%s\n", json_string);
    if(json_string != NULL)
    {
        free(json_string);
        json_string = NULL;
    }
    // 释放JSON对象和字符串的内存
    cJSON_Delete(json);
}

void reply_clock_list(int result, cJSON *data, char *seq)
{
    reply_message("reply", result, data, seq, CLOCK_LIST_REPLY);
 
}
void set_clock_list(cJSON *json, int *array_len, char *music_list_buffer) 
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    
    if (data != NULL && cJSON_IsObject(data)) {
        
        // 提取data对象
        cJSON *data2 = cJSON_GetObjectItemCaseSensitive(data, "data"); //"2024-8-10"
        if (data2 != NULL && cJSON_IsString(data2)) {
            printf("data ID: %s\n", data2->valuestring);

        }

        cJSON *clockTime = cJSON_GetObjectItemCaseSensitive(data, "clockTime"); //"23:05"
        if (clockTime != NULL && cJSON_IsString(clockTime)) {
            printf("clockTime ID: %s\n", clockTime->valuestring);
        }
        cJSON *repeat = cJSON_GetObjectItemCaseSensitive(data, "repeat"); //"23:05"
        if (repeat != NULL && cJSON_IsNumber(repeat)) {
            printf("repeat ID: %d\n", repeat->valueint);
        }

        // 提取music_list数组
        cJSON *repeat_day = cJSON_GetObjectItemCaseSensitive(data, "repeat_day");
        if (repeat_day != NULL && cJSON_IsArray(repeat_day)) {
            // 遍历music_list数组并打印每个元素
            int array_size = cJSON_GetArraySize(repeat_day);
            char *json_string = cJSON_Print(repeat_day);
            printf("%s\n", json_string);

            if(json_string != NULL){
                free(json_string);
                json_string = NULL;
            }
            
            for (int i = 0; i < array_size; i++) {
                cJSON *repeat_dayItem = cJSON_GetArrayItem(repeat_day, i);
                if (repeat_dayItem != NULL && cJSON_IsNumber(repeat_day)) {
                    printf("Repeat ID: %d\n", repeat_dayItem->valueint);
                }
            }
        }

    }
   
    
}


void set_music_list(cJSON *json, int *array_len, char *music_list_buffer) 
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        // 提取music_list数组
        cJSON *music_list = cJSON_GetObjectItemCaseSensitive(data, "music_list");
        
        if (music_list != NULL && cJSON_IsArray(music_list)) {
            // 遍历music_list数组并打印每个元素
            int array_size = cJSON_GetArraySize(music_list);
            
            char *json_string = cJSON_Print(music_list);

            // printf("%s\n", json_string);
            memcpy(music_list_buffer, json_string, strlen(json_string));
            if(json_string != NULL){
                free(json_string);
                json_string = NULL;
            }
            
            if(array_size > 100)
            {
                printf("music list is overflow\n");
                return ;
            }
            if(array_len != NULL)
            {
                *array_len = array_size;
            }
            pp_delete_all_music_list();
            for (int i = 0; i < array_size; i++) {
                cJSON *music_item = cJSON_GetArrayItem(music_list, i);
                if (music_item != NULL && cJSON_IsString(music_item)) {
                    // printf("Music ID: %s\n", music_item->valuestring);
                    memcpy(g_networkmusic_list[i], music_item->valuestring, strlen(music_item->valuestring));
                    printf("Music ID: %s\n", g_networkmusic_list[i]);
                }
            }
        }
    }
}
void reply_music_list(int result, int music_total, char *music_list, char *seq)
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "music_total", music_total);
    cJSON_AddRawToObject(data, "music_list", music_list);
    reply_message("reply", result, data, seq, MUSIC_LIST_REPLY);
 
}


void set_single_music_play(cJSON *json, char *url_buffer, char *music_id_temp) 
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        
        cJSON *music_id = cJSON_GetObjectItemCaseSensitive(data, "music_id");
        if (music_id != NULL && cJSON_IsString(music_id)) {
            printf("music_id = %s\n", music_id->valuestring);
            memcpy(music_id_temp, music_id->valuestring, strlen(music_id->valuestring));
            
        }
        cJSON *music_file_info = cJSON_GetObjectItemCaseSensitive(data, "music_file_info");
        if (music_file_info != NULL && cJSON_IsObject(music_file_info)) {
            const cJSON *file_url = cJSON_GetObjectItemCaseSensitive(music_file_info, "file_url");
            char *url = cJSON_PrintUnformatted(file_url);
            printf("file_url = %s\n", url);
            if(url != NULL)
            {
                memcpy(url_buffer, url, strlen(url));
                free(url);
                url = NULL;
            }
        }
        
    }
}

//切歌时主动上报id
void reple_single_music_play(char *type,int result, char *music_id, char *seq) 
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "music_id", music_id);
    reply_message(type, result, data, seq, SINGLE_MUSIC_REPLY);
}


void set_control_play(cJSON *json, int *control_flag)
{
     // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        cJSON *playing = cJSON_GetObjectItemCaseSensitive(data, "playing");
        if (playing != NULL && cJSON_IsNumber(playing)) {
            printf("playing = %d\n", playing->valueint);
            *control_flag  = playing->valueint;
        }

    }

}
//暂停主动上报
void reply_control_play(char *type, int result, int playing, char *seq)
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "playing", playing);
    reply_message(type, result, data, seq, CONTROL_PLAY_REPLY);
}


void set_control_vol(cJSON *json, int *vol)
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        cJSON *volume = cJSON_GetObjectItemCaseSensitive(data, "volume");
        if (volume != NULL && cJSON_IsNumber(volume)) {
            // printf("volume = %d\n", volume->valueint);
            *vol  = volume->valueint;
        }

    }
}
void reply_control_vol(char *type,int result, int volume, char *seq)
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "volume", volume);
    reply_message(type, result, data, seq, VOLUME_REPLY);
}

void get_contant_url(cJSON *json, int *id, char *url)
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        cJSON *content_id = cJSON_GetObjectItemCaseSensitive(data, "content_id");
        if (content_id != NULL && cJSON_IsNumber(content_id)) {
            *id  = content_id->valueint;
        }
        cJSON *music_file_info = cJSON_GetObjectItemCaseSensitive(data, "content_file_info");
        if (music_file_info != NULL && cJSON_IsObject(music_file_info)) {
            cJSON *file_url = cJSON_GetObjectItemCaseSensitive(music_file_info, "file_url");
            if (file_url != NULL && cJSON_IsString(file_url)) {
               memcpy(url, file_url->valuestring, strlen(file_url->valuestring));
            }
        }
    }

}
void reply_contant_url(int result, int content_id, char* seq)
{
    cJSON *data = cJSON_CreateObject();
     cJSON_AddNumberToObject(data, "content_id", content_id);

    reply_message("reply", result, data, seq, CONTANT_REPLY);
}

void set_music_to_list(cJSON *json, char *id)
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        cJSON *music_id = cJSON_GetObjectItemCaseSensitive(data, "music_id");
        if (music_id != NULL && cJSON_IsString(music_id)) {
            // printf("volume = %d\n", volume->valueint);
            memcpy(id, music_id->valuestring, strlen(music_id->valuestring));
        }

    }
}
void reply_music_to_list(int result, char *music_id, char* seq)
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "music_id", music_id);

    reply_message("reply", result, data, seq, MUSIC_ADD_REPLY);

}


void set_music_mode(cJSON *json, int *mode)
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        cJSON *loop_mode = cJSON_GetObjectItemCaseSensitive(data, "loop_mode");
        if (loop_mode != NULL && cJSON_IsNumber(loop_mode)) {
            *mode = loop_mode->valueint;
        }

    }
}
void reply_music_mode(int result, int mode, char* seq)
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "loop_mode", mode);
    reply_message("reply", result, data, seq, MUSIC_MODE_REPLY);
}

void set_music_delete(cJSON *json, char *id)
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        cJSON *music_id = cJSON_GetObjectItemCaseSensitive(data, "music_id");
        if (music_id != NULL && cJSON_IsString(music_id)) {
            memcpy(id, music_id->valuestring, strlen(music_id->valuestring));
        }

    }
}
void reply_music_delete(int result, char *music_id, char* seq)
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "music_id", music_id);
    reply_message("reply", result, data, seq, MUSIC_DELETE_REPLY);
}


void get_music_url(cJSON *json, int *res, char *url)
{
    // 提取data对象
    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL && cJSON_IsObject(data)) {
        cJSON *result = cJSON_GetObjectItemCaseSensitive(data, "result");
        if (result != NULL && cJSON_IsNumber(result)) {
            *res  = result->valueint;
        }
        cJSON *data2 = cJSON_GetObjectItemCaseSensitive(data, "data");
        if (data2 != NULL && cJSON_IsObject(data2)) {
            cJSON *music_file_info = cJSON_GetObjectItemCaseSensitive(data2, "music_file_info");
            if (music_file_info != NULL && cJSON_IsObject(music_file_info)) {
                cJSON *file_url = cJSON_GetObjectItemCaseSensitive(music_file_info, "file_url");
                if (file_url != NULL && cJSON_IsString(file_url)) {
                    // printf("file_url->valuestring = %s\n", file_url->valuestring);
                    memcpy(url, file_url->valuestring, strlen(file_url->valuestring));
                }

            }
        }
    }
}






void pp_insert_single_music_by_id(char *id)
{
    for(int i=0;i<50;i++)
    {
        if((g_networkmusic_list[i][0] == NULL))
        {
            memcpy(g_networkmusic_list[i], id, strlen(id));
            break;
        }
    }
    pp_show_all_music_id();
}

void pp_play_networkmusic_by_id(int num)
{
    if(num >= 0 && num <= 50)
    {
        printf("g_networkmusic_list[%d] = %s\n",num, g_networkmusic_list[num]);
        pp_mqtt_require_networkMusic(g_networkmusic_list[num]);
    }  
}

void pp_play_networkmusic_by_name(char *name)
{
    int index = -1;
    for(int i=0;i<50;i++)
    {
        if(memcmp(g_networkmusic_list[i], name, strlen(name)) == 0)
        {
            index = i;
        }
    }
    if(index != -1)
    {
        pp_play_networkmusic_by_id(index);
    }

}

int pp_find_networkmusic_in_list(char *name)
{
    for(int i=0;i<50;i++)
    {
        if(memcmp(g_networkmusic_list[i], name, strlen(name)) == 0)
        {
            return 1;
        }
    }
    return -1;
}

void parase_subscribe_data(char *topic, int topinc_len, char *data, int data_len) 
{
    if(topinc_len == NULL || data_len == NULL)
    {
        return ; 
    }
    if(memcmp(topic, MQTT_SUBSCRIBE_TOPIC_SERVERREPLY, topinc_len) == 0) //query
    {
        printf("QUERY DATA=%.*s\r\n", data_len, data);
        cJSON *json = cJSON_Parse(data);
        const cJSON *commandType = cJSON_GetObjectItemCaseSensitive(json, "commandType");
        if (cJSON_IsNumber(commandType)) {
            printf("commandType: %d\n", commandType->valueint);
            char seq[12] = {0};
            const cJSON *seqNumber = cJSON_GetObjectItemCaseSensitive(json, "seq");
            memcpy(seq, seqNumber->valuestring, strlen(seqNumber->valuestring));

            switch (commandType->valueint)
            {
                case PARAMETER_QUERY:

                    break;
                case CLOCK_QUERY:
                    get_clock_info(seq);
                    break;
                case MUSIC_QUERY:
                    // 350
                    get_music_info(seq);
                    break;

                case MUSIC_URL_GET:
                    char url[500] = {0};
                    int result = -1;
                    get_music_url(json, &result, url);
                    printf("result = %d, url = %s\n",result, url);
                    //play url .......
                    pp_music_play(url, 1, 1);
                break;

                default:
                    break;
            }
        }

        cJSON_Delete(json);
    }
    else if(memcmp(topic, MQTT_SUBSCRIBE_TOPIC_SETTING, topinc_len) == 0) //setting
    {
        cJSON *json = cJSON_Parse(data);
        const cJSON *commandType = cJSON_GetObjectItemCaseSensitive(json, "commandType");
        if (cJSON_IsNumber(commandType)) {
            // printf("setting commandType: %d\n", commandType->valueint);
            char seq[12] = {0};
            const cJSON *seqNumber = cJSON_GetObjectItemCaseSensitive(json, "seq");
            memcpy(seq, seqNumber->valuestring, strlen(seqNumber->valuestring));
            // printf("seq = %s\n", seq);
            switch (commandType->valueint)
            {
                case CLOCK_SETTING: //411 添加闹钟
                    char clock_list_buffer[1024] = {0};
                    int clock_len = 0;
                    cJSON *json_buf = cJSON_GetObjectItemCaseSensitive(json, "data");;
                    set_clock_list(json, &clock_len, clock_list_buffer);
                    if(json_buf != NULL) 
                    {
                        char *json_string = cJSON_Print(json_buf);
                        printf("%s\n", json_string);
                    }
                    
                    reply_clock_list(1, json_buf, seq);
                    //reply_music_list(1, array_len, music_list_buffer, seq);
                break;

                case MUSIC_SETTING: //451 下发音乐列表
                    int array_len = 0;
                    char music_list_buffer[1024] = {0};
                    set_music_list(json, &array_len, music_list_buffer);
                    //回复351
                    reply_music_list(1, array_len, music_list_buffer, seq);
                    //播放列表歌曲
                    pp_networkmusicList_play();

                break;
                case MUSIC_VOL_SETTING: //455
                     int volNum = -1;
                    set_control_vol(json, &volNum);
                    printf("volNum = %d\n", volNum);
                    reply_control_vol("reply", 1, volNum, seq);
                    pp_setVolNum(volNum);
                break;
                case MUSIC_URL_GET: //230
                    char url[500] = {0};
                    int result = -1;
                    get_music_url(json, &result, url);
                    printf("result = %d, url = %s\n",result, url);
                    //play url .......
                    pp_music_play(url, 1, 1);
                    
                break;
                case ADD_MUSIC_TO_LIST_SETTING: //453
                    char buf[20] = {0};
                    set_music_to_list(json, buf);
                    printf("music id add = %s\n", buf);
                    pp_insert_single_music_by_id(buf);
                    reply_music_to_list(1, buf, seq);
                break;
                case MUSIC_MODE_SETTING: //456
                    int mode = -1;
                    set_music_mode(json, &mode);
                    printf("mode = %d\n", mode);
                    reply_music_mode(1, mode, seq);
                break;
                case MUSIC_DELET_SETTING: //457
                    char music_id[20] = {0};
                    int res = -1;
                    set_music_delete(json, music_id);
                    if(pp_find_networkmusic_in_list(music_id) == 1)
                    {
                        res = 1;
                    }
                    reply_music_delete(res,music_id, seq);
                break;
                case MUSIC_PLAYORPAUSE_NOTICE: 
                    int play_flag = -1;
                    set_control_play(json, &play_flag);
                    reply_control_play("reply", 1, play_flag, seq);
                    if(play_flag == 1) //播放
                    {
                        printf("playing\n");
                        pp_playControl(play_flag);
                    }
                    else if(play_flag == 0) //暂停
                    {
                        printf("pausing\n");
                        pp_playControl(play_flag);
                    }
                break;
                default:
                break;
            }
        }

        cJSON_Delete(json);
    }
    else if(memcmp(topic, MQTT_SUBSCRIBE_TOPIC_NOTICE, topinc_len) == 0) //notice
    {
        cJSON *json = cJSON_Parse(data);
        const cJSON *commandType = cJSON_GetObjectItemCaseSensitive(json, "commandType");
        if (cJSON_IsNumber(commandType)) {
            // printf("notice commandType: %d\n", commandType->valueint);   
            char seq[12] = {0};
            const cJSON *seqNumber = cJSON_GetObjectItemCaseSensitive(json, "seq");
            memcpy(seq, seqNumber->valuestring, strlen(seqNumber->valuestring));
            // printf("seq = %s\n", seq); 
            switch (commandType->valueint)
            {
                case MUSIC_SINGLE_NOTICE://接收下发的一首音乐 452 立刻播放！
                    char music_id[20] = {0};
                    char url_buffer[500] = {0};
                    
                    set_single_music_play(json, url_buffer, music_id);
                    printf("url_buffer = %s, len = %d\n", url_buffer, strlen(url_buffer));

                    //回复352
                    reple_single_music_play("reply", 1, music_id, seq);

                    pp_delete_all_music_list();
                    pp_mqtt_require_networkMusic(music_id);
                    // pp_music_play(url_buffer, 1, 1);
                break;
                case MUSIC_PLAYORPAUSE_NOTICE: 
                        int play_flag = -1;
                        set_control_play(json, &play_flag);
                        reply_control_play("reply", 1, play_flag, seq);
                        if(play_flag == 1) //播放
                        {
                            printf("playing\n");
                            pp_playControl(play_flag);
                        }
                        else if(play_flag == 0) //暂停
                        {
                            printf("pausing\n");
                            pp_playControl(play_flag);
                        }
                break;
                case CONTANT_SINGLE_NOTICE:
                    char contant_url[256] = {0};
                    int contant_id = 0;
                    get_contant_url(json, &contant_id, contant_url);
                    printf("contant_url = %s\n", contant_url);
                    reply_contant_url(1, contant_id, seq);

                    pp_music_play(contant_url, 1, 1);
                break;
                
                default:
                break;
            }
        }
    } 
   
    
}



/*mqtt*/
//5263234234
#define PUBLISH_HEAD "{\"deviceMac\": \"%s\",\"time\": 175263451, \"commandType\": %d,\"seq\": \"%s\",\"data\":%s}"
int pp_mqtt_client_publish( char *command, char *data_buffer, int commandType, int pub_length, char *seq)
{
    char topic_name[64] = {0};
    get_public_topic_name(topic_name, command);
    printf("topic name = %s\n", topic_name);
    
    char publish_data[2048] = {0};

    snprintf(publish_data, 2048, PUBLISH_HEAD , getdeviceId(), commandType, seq, data_buffer);
    printf("publish_data = %s\n", publish_data);
    
    esp_mqtt_client_publish(s_mqtt_client, topic_name,
                    publish_data, strlen(publish_data), 1, 1);
    return 0;
}



/*publish*/
void music_playing(char *id)
{

}





void pp_mqtt_clent_subscribe()
{
    snprintf(MQTT_SUBSCRIBE_TOPIC_SERVERREPLY, 46, BUFFER_SUBSCRIBE_TOPIC_SERVERREPLY , getdeviceId());
    printf("MQTT_SUBSCRIBE_TOPIC_SERVERREPLY = %s\n", MQTT_SUBSCRIBE_TOPIC_SERVERREPLY);

    snprintf(MQTT_SUBSCRIBE_TOPIC_QUERY, 46, BUFFER_SUBSCRIBE_TOPIC_QUERY , getdeviceId());
    printf("MQTT_SUBSCRIBE_TOPIC_QUERY = %s\n", MQTT_SUBSCRIBE_TOPIC_QUERY);

    snprintf(MQTT_SUBSCRIBE_TOPIC_SETTING, 46, BUFFER_SUBSCRIBE_TOPIC_SETTING , getdeviceId());
    printf("MQTT_SUBSCRIBE_TOPIC_SETTING = %s\n", MQTT_SUBSCRIBE_TOPIC_SETTING);

    snprintf(MQTT_SUBSCRIBE_TOPIC_NOTICE, 46, BUFFER_SUBSCRIBE_TOPIC_NOTICE , getdeviceId());
    printf("MQTT_SUBSCRIBE_TOPIC_NOTICE = %s\n", MQTT_SUBSCRIBE_TOPIC_NOTICE);

    esp_mqtt_client_subscribe_single(s_mqtt_client, MQTT_SUBSCRIBE_TOPIC_SERVERREPLY, 1);
    esp_mqtt_client_subscribe_single(s_mqtt_client, MQTT_SUBSCRIBE_TOPIC_QUERY, 1);
    esp_mqtt_client_subscribe_single(s_mqtt_client, MQTT_SUBSCRIBE_TOPIC_SETTING, 1);
    esp_mqtt_client_subscribe_single(s_mqtt_client, MQTT_SUBSCRIBE_TOPIC_NOTICE, 1);
}



//MQTT连接标志
static bool   s_is_mqtt_connected = false;
/**
 * mqtt连接事件处理函数
 * @param event 事件参数
 * @return 无
 */
static void aliot_mqtt_event_handler(void* event_handler_arg,
                                        esp_event_base_t event_base,
                                        int32_t event_id,
                                        void* event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:  //连接成功
            ESP_LOGI(TAG, "mqtt connected");
            s_is_mqtt_connected = true;
            pp_mqtt_clent_subscribe();
            //连接成功后，订阅测试主题
            
            // 初始化
            char mqtt_pub_buff[] = "{\"deviceMac\":\"%s\",\"device_model\":\"1\",\"firmware\":\"1.0.0\",\"network_type\":1}";
            char mqtt_init_buff[90] = {0};
            snprintf(mqtt_init_buff, 90, mqtt_pub_buff , getdeviceId());
            pp_mqtt_client_publish("request", mqtt_init_buff, 101, strlen(mqtt_init_buff), "5263234234");

            led_blink_onoff(2);
            break;
        case MQTT_EVENT_DISCONNECTED:   //连接断开
            ESP_LOGI(TAG, "mqtt disconnected");
            s_is_mqtt_connected = false;
            break;
        case MQTT_EVENT_SUBSCRIBED:     //收到订阅消息ACK
            ESP_LOGI(TAG, " mqtt subscribed ack, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:   //收到解订阅消息ACK
            break;
        case MQTT_EVENT_PUBLISHED:      //收到发布消息ACK
            ESP_LOGI(TAG, "mqtt publish ack, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            printf("[REC]TOPIC=%.*s\r\n", event->topic_len, event->topic);       //收到Pub消息直接打印出来
            printf("[REC]DATA=%.*s\r\n\n\n", event->data_len, event->data);

            parase_subscribe_data(event->topic,event->topic_len, event->data, event->data_len);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            break;
    }
}
/** 启动mqtt连接
 * @param 无
 * @return 无
*/
void app_mqtt_start(void)
{
    static char mqtt_client_id_buff[13];
    snprintf(mqtt_client_id_buff, 13, ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));
   
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = MQTT_ADDRESS;
    mqtt_cfg.broker.address.port = MQTT_PORT;
    //Client ID
    mqtt_cfg.credentials.client_id = mqtt_client_id_buff;
    //用户名
    mqtt_cfg.credentials.username = MQTT_USERNAME;
    //密码
    mqtt_cfg.credentials.authentication.password = MQTT_PASSWORD;
    // mqtt_cfg.network.timeout_ms = 120;
    ESP_LOGE(TAG,"mqtt connect->clientId:%s,username:%s,password:%s",mqtt_cfg.credentials.client_id,
    mqtt_cfg.credentials.username,mqtt_cfg.credentials.authentication.password);
    //设置mqtt配置，返回mqtt操作句柄
    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    //注册mqtt事件回调函数
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, aliot_mqtt_event_handler, s_mqtt_client);
    //启动mqtt连接
    esp_mqtt_client_start(s_mqtt_client);

}



