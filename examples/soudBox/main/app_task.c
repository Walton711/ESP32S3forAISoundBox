#include "app_task.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "sdkconfig.h"
#include "esp_log.h"

#include "wifi_service.h"
#include "smart_config.h"
#include "blufi_config.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include <sys/time.h>
#include "esp_netif_sntp.h"
#include "esp_http_client.h"
#include "esp_action_exe_type.h"
#include "player_action.h"
#include "esp_dispatcher.h"
#include "audio_def.h"
#include "audio_thread.h"
#include "app_system_info.h"
#include "app_mqtt.h"


static QueueHandle_t  player_cmd_queue = NULL;
static QueueHandle_t  file_cmd_queue = NULL;
static QueueHandle_t  led_cmd_queue = NULL;

static const char               *TAG                = "APPTASK";

char g_music_list[20][400] = {0};



char *rtrim(char *str)
{
    if (str == NULL || *str == '\0')
    {
        return str;
    }

    int len = strlen(str);
    char *p = str + len - 1;
    while (p >= str && *p == '\"')
    {
        *p = '\0';
        --p;
    }

    return str;
}

//去除首部空格
char *ltrim(char *str)
{
    if (str == NULL || *str == '\0')
    {
        return str;
    }

    int len = 0;
    char *p = str;
    while (*p != '\0' && *p == '\"')
    {
        ++p;
        ++len;
    }

    memmove(str, p, strlen(str) - len + 1);

    return str;
}
extern int g_rec_status;
extern int g_music_playlist;
void parse_http_response(char *buf, int resLen)
{
    // 解析返回；
    int len =0;
    char *pLastCurrent= malloc(resLen);
    char *cmdpLastCurrent= malloc(resLen);
    char *byeCurrent= malloc(resLen);
    char *errorCurrent = malloc(resLen);
    char *pLast= pLastCurrent;
    char *cmdpLast= cmdpLastCurrent;
    char *bye = byeCurrent;
    char *error = errorCurrent;
    char cmdbuf[1] = {0};

    error = strstr(buf, "识别失败");
    if (NULL != error)
    {
       printf("音频识别失败.............\n");
       g_rec_status = 0;
       g_music_playlist = -1;
       pp_music_play("file://spiffs/noack/no_5.wav",6,1);
    }
    free(errorCurrent);
    bye = strstr(buf, "再见");
    if (NULL != bye)
    {
       printf("再见.............\n");
       g_rec_status = 0;
       g_music_playlist = -1;
    }
    free(byeCurrent);
    pLast = strstr(buf, "\"voice_url\":\"");
    if (NULL != pLast)
    {
        pLast = pLast + 13; 
        while (pLast[0] != '\"')
        {
            len++;
            pLast++;
            if(len >= resLen)
            {
                break;
            }
        }
        char rec_buffer[100] = {0};
        memcpy(rec_buffer, pLast - len, len);

        if(rec_buffer[0] == 'h')
        {
            pp_music_play(rec_buffer,6,1);
        }
        
    }
#if 1
    len = 0;
    cmdpLast = strstr(buf, "\"cmd\":");
    if (NULL != cmdpLast) //cmd
    {
        cmdpLast = cmdpLast + 6; 
        while (cmdpLast[0] != ',')
        {
            len++;
            cmdpLast++;
            if(len >= resLen)
            {
                break;
            }
        }
        
        memcpy(cmdbuf, cmdpLast - len, len);
        if(cmdbuf[0] == '1') //clock
        {
            len = 0;
            char *timepcurrent = malloc(resLen);
            char *timepLast = timepcurrent;
            timepLast = strstr(buf, "\"time\":\"");
            if (NULL != timepLast) //time
            {
                timepLast = timepLast + 8; 
                while (timepLast[0] != '\"')
                {
                    len++;
                    timepLast++;
                    if(len >= resLen)
                    {
                        break;
                    }
                }
                char timebuf[16] = {0};
                memcpy(timebuf, timepLast - len, len);
                printf("闹钟len = %d,time = %s\n",len, timebuf);
                if(timebuf[0] == '2') 
                {
                    clock_system_insert(cmdbuf[0]-'0', timebuf, timebuf);
                }
                free(timepcurrent);
                timepcurrent = NULL;
            }
            
        }
        else if(cmdbuf[0] == '2') // music
        {
            len = 0;
            char *typecurrent = malloc(resLen);
            char *typepLast = typecurrent;
            typepLast = strstr(buf, "\"type\":\"");
            if (NULL != typepLast) //type
            {
                typepLast = typepLast + 8; 
                while (typepLast[0] != '\"')
                {
                    len++;
                    typepLast++;
                    if(len >= resLen)
                    {
                        break;
                    }
                }
                char tpyebuf[1] = {0};
                memcpy(tpyebuf, typepLast - len, len);
                printf("2len = %d,tpyebuf = %c\n",len, tpyebuf[0]);
                free(typecurrent);
                typecurrent = NULL;
            }
            
            char *current = malloc(resLen);
            char *music_url = music_url;
            music_url = strstr(buf, "\"music_url\":[");

            if (NULL != current) //type
            { 
                
                music_url = music_url + 14; 
                // printf("music_url = %s\n",music_url);
                
                while (music_url[0] != ']')
                {
                    len++;
                    music_url++;
                    if(len >= resLen)
                    {
                        break;
                    }
                }
                
                char musicbuf[1024*8] = {0};
                memcpy(musicbuf, music_url - len, len);
                printf("len = %d,musicbuf = %s\n",len, musicbuf);
                
                //"","",""
                char *temp = strtok(musicbuf,",");
                int num = 0;
                while(temp)
                {
                    memcpy(&g_music_list[num], temp, strlen(temp));
                    
                    temp = strtok(NULL,",");
                    num++;
                    if(num >= 20)
                    {
                        num = 20;
                        break;
                    }
                }
                printf("num = %d\n",num);
               
                for(int i=0;i<num;i++)
                {
                    rtrim(g_music_list[i]);
                    ltrim(g_music_list[i]);
                    printf("len = %d, temp = %s\n",strlen(g_music_list[i]), g_music_list[i]);
                }
                pp_musicList_play();
                
                free(current);
                current = NULL;
                
                // free(music_url); 
                
            }
           
            
        }
    }

#endif
    len = 0;
    // free(pLast);
    // free(cmdpLast);
    
    free(pLastCurrent);
    free(cmdpLastCurrent);
    pLastCurrent = NULL;
    cmdpLastCurrent = NULL;
}

void parse_http_time(char *buf, int resLen)
{
    // 解析返回；
    int len =0;
    char *pLast= malloc(resLen);;
    
    pLast = strstr(buf, "\"data\":");
    if (NULL != pLast)
    {
        // printf("pLast = %s\n", pLast);
        pLast = pLast + 7; 
        while (pLast[0] != '}'){
            len++;
            pLast++;
            if(len >= resLen){
                break;
            }
        }
        char rec_buffer[15] = {0};
        memcpy(rec_buffer, pLast - len, len);
        http_setTime_init(rec_buffer, len);
        ESP_LOGI(TAG, "len = %d,rec_buffer = %s\n",len, rec_buffer);
    }
}

uint8_t http_status = 0;
/****************************************** http **********************************************/
FILE *rFp;
uint32_t voice_total_len;
char* local_response_buffer;
// HTTP 请求的处理函数
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    // 缓存http响应的buffer
    static char *output_buffer;
    static char rec_buffer[1024*10];
    // 已经读取的字节数
    static int output_len;
    local_response_buffer = (char *) evt->user_data;
    
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            http_status = 1;
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
                printf("output_len=%d, evt->data_len = %d\n", output_len, evt->data_len);
            }
            else {
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                    printf("evt->data_len = %d\n", evt->data_len);
                } else {
                    // if (output_buffer == NULL) {
                        //output_buffer = (char *) malloc(evt->data_len/*esp_http_client_get_content_length(evt->client)*/);
                        // output_len = 0;
                        // if (output_buffer == NULL) {
                        //     ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        //     return ESP_FAIL;
                        // }
                    // }
                    printf("evt->data_len = %d,output_len = %d\n", evt->data_len, output_len);
                    memcpy(rec_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            // parse_http_response((char *)evt->data, strlen((char *)evt->data));
            // printf("output_len = %d\n", output_len);
            // printf("output_buffer = %s\n", rec_buffer);
            break;
        case HTTP_EVENT_ON_FINISH:
            printf("output_len = %d\n", output_len);
            printf("output_buffer = %s\n", rec_buffer);
            parse_http_response(rec_buffer,  output_len);
            memset(rec_buffer, 0, 1024*10);
            break;
        case HTTP_EVENT_DISCONNECTED:
           

            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            http_status = 0;
            break;
        case HTTP_EVENT_REDIRECT:
        break;
    }
    return ESP_OK;
}

void http_send_url(char *request, uint32_t request_len)
{

    esp_http_client_config_t config = {
        // .url ="https://testdhome-api.scstit.com:9443/parse/openapi/parse/asr",
        .url = "https://testdhome-api.scstit.com:9443/parse/openapi/parse/dialogue",
        .event_handler = _http_event_handler,
        .buffer_size = 50*1024,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");
    esp_http_client_set_header(client, "AppId", "h5f972d6a6e30b45");
    esp_http_client_set_header(client, "AppVersion", "1.0.0");
    esp_http_client_set_header(client, "Signature", "fec6de6985374edbb0e81bc558a0c55d");
    esp_http_client_set_header(client, "CITY", "%%E5%%B9%%BF%%E4%%B8%%9C");
    esp_http_client_set_header(client, "DEVICE_ID", "abcdefg");
    esp_http_client_set_header(client, "area", "%%E5%%B9%%BF%%E4%%B8%%9C");
    esp_http_client_set_header(client, "weatherType", 3);
    
    esp_err_t err = esp_http_client_open(client, request_len);
    printf("https_with_url request_len =%ld\n",request_len);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, request, request_len);
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
           int codRet = esp_http_client_get_status_code(client);
           int lenRet =  esp_http_client_get_content_length(client);
           ESP_LOGI(TAG, "codRet = %d,lenRet=%d", codRet, lenRet);
        } else {
            ESP_LOGI(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
    }
    esp_http_client_cleanup(client);
    printf("ok\n");
}

void http_rec_check_url(char *request, uint32_t request_len)
{
    esp_http_client_config_t config = {
        // .url ="https://testdhome-api.scstit.com:9443/parse/openapi/parse/asr",
        // .url = "https://testdhome-api.scstit.com:9443/parse/openapi/parse/dialogue",
        //.url = "http://116.205.135.166/aihome/base/api",
        .url = "http://139.159.141.213/aihome/base/api",
        
        .event_handler = _http_event_handler,
        .timeout_ms = 20000,
        .buffer_size = 16*1024,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");
    esp_http_client_set_header(client, "DeviceId", "abcdef");
    esp_http_client_set_header(client, "type", "0");
    esp_http_client_set_header(client, "city", ""); //广州
    esp_http_client_set_header(client, "time", "2024-08-18");
    esp_http_client_set_header(client, "weatherType", "6");
    esp_http_client_set_header(client, "RoleType ", "abcd");

    esp_err_t err = esp_http_client_open(client, request_len);
    printf("https_with_url request_len =%ld\n",request_len);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, request, request_len);
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
           int codRet = esp_http_client_get_status_code(client);    
           int lenRet =  esp_http_client_get_content_length(client);
           ESP_LOGI(TAG, "codRet = %d, lenRet=%d", codRet, lenRet);  
        } else {
            ESP_LOGI(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
    }
    esp_http_client_cleanup(client);
    printf("ok\n");
}


esp_err_t _http_getTime_handler(esp_http_client_event_t *evt)
{
    // 缓存http响应的buffer
    static char *output_buffer;
    // 已经读取的字节数
    static int output_len;
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_REDIRECT:
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            http_status = 1;
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
                printf("output_len=%d, evt->data_len = %d\n", output_len, evt->data_len);
                printf("output_buffer = %s\n", output_buffer);
            }
            else {
                printf("evt->data = %s\n",(char *) evt->data);
                parse_http_time((char *) evt->data, 50);
                break;
            }
            
            break;
        case HTTP_EVENT_ON_FINISH:
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            http_status = 0;
            break;
    }
    return ESP_OK;
}

void http_getNetwork_time()
{
    esp_http_client_config_t config = {
        // .url ="https://testdhome-api.scstit.com:9443/parse/openapi/parse/asr",
        .url = "http://116.205.135.166/aihome/base/getCurrentTimeMillis",
        .event_handler = _http_getTime_handler,
        .timeout_ms = 30000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 1);
    
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, " ", 1);

        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK) {
           int codRet = esp_http_client_get_status_code(client);
           int lenRet =  esp_http_client_get_content_length(client);
           ESP_LOGI(TAG, "codRet = %d,lenRet=%d", codRet, lenRet);
        } else {
            ESP_LOGI(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
    }
    esp_http_client_cleanup(client);
    printf("ok\n");
}


int file_init()
{
    rFp = fopen("/spiffs/rec.pcm","w+");
    if(rFp == NULL)
    {
        printf("open file fail\n");
        return -1;
    }
    printf("file open success\n");
    return 0;
}

int file_save_recorde(char *buffer,int len)
{
    if(rFp == NULL)
    {
        return -1;
    }
    int r = fwrite(buffer, 1, len, rFp);
    voice_total_len += len;
    printf("r = %d, voice_total_len = %ld\n",r,  voice_total_len);
    return 0;
}

int file_finish_recorde()
{
    printf("finish!!!!rec len = %ld\n", voice_total_len);
    if (rFp) {
        char* voiceData = malloc(voice_total_len);
        if (voiceData) {
            fseek(rFp, 0, SEEK_SET);
            int rlen = fread(voiceData, 1, voice_total_len, rFp);
            printf("rlen = %d, voiceData[] = %x\n",rlen, voiceData[0]);
            fclose(rFp);
            http_rec_check_url(voiceData, voice_total_len);
            remove("file://spiffs/rec.pcm");
        }
    }

    return 0;
}

void file_task_start()
{
    file_msg_t msg = {0};
    while(1)
    {
        xQueueReceive(file_cmd_queue, &msg, portMAX_DELAY);
        // printf("file_task_start msg = %d\n", msg.msg);
        if(msg.msg == 1)
        {
            // file_init();
        }
        else if(msg.msg == 2)
        {
            // printf("++++++msg.write_buffer[] = %d+++++++\n", msg.write_buffer[100]);
            file_save_recorde(msg.write_buffer,msg.write_len);
        }
        else if(msg.msg == 3)
        {

            http_rec_check_url(msg.write_buffer, msg.write_len);
            // file_finish_recorde(1);
        }
        // vTaskDelay(1);
    }
    vTaskDelete(NULL);
}

int pp_file_write(char *buffer, int len)
{
    file_msg_t msg = {0};
    msg.msg = 3;
    msg.write_buffer = buffer;
    msg.write_len = len;
    if(0) {
        if (xQueueSendFromISR(file_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    } else {
        if (xQueueSend(file_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    }
    return 0;
}

int pp_file_cmd(int n, int interrupt_flag)
{
    file_msg_t msg = {0};
    msg.msg = n;
    if(interrupt_flag) {
        if (xQueueSendFromISR(file_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    } else {
        if (xQueueSend(file_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    }
    return 0;
}

int file_task_init()
{
    file_cmd_queue = xQueueCreate(10, sizeof(file_msg_t));
    BaseType_t ret = xTaskCreate(file_task_start, "FILE_Task", 13*1024, NULL, 4, NULL);
    printf("file_task_init ret = %d\n", ret);
    // printf("in file_task_init the min free stack size is %ld \r\n", (int32_t)uxTaskGetStackHighWaterMark(NULL));
    return 0;
}


void file_stop_recorde()
{
    fclose(rFp);
}



FILE *mFp;
esp_err_t _http_getMusic_handler(esp_http_client_event_t *evt)
{
    // 缓存http响应的buffer
    // 已经读取的字节数
    static int output_len;
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_REDIRECT:
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {

                if(mFp == NULL)
                {
                    break;
                }
                fwrite(evt->data, 1, evt->data_len, mFp);
                output_len += evt->data_len;
                printf("output_len333=%d, evt->data_len = %d\n", output_len, evt->data_len);
            }
            else {
                break;
            }
            
            break;
        case HTTP_EVENT_ON_FINISH:
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            output_len = 0;
            if(mFp != NULL)
            {
                // fclose(mFp);
            }
            
            break;
    }
    return ESP_OK;
}

void http_getMusic(char *url_buffer)
{
    char url_buf[70] = {0};
    snprintf(url_buf, 60, "http://116.205.135.166/dingding/%s" ,url_buffer);
    printf("url_buf = %s\n", url_buf);
    esp_http_client_config_t config = {
        // .url = "http://116.205.135.166/dingding/welcome.wav",
        // .url = "http://116.205.135.166/dingding/system/network.wav",
        // .url = "http://116.205.135.166/dingding/wakeup/wakeup1.wav",
        // .url = "http://116.205.135.166/dingding/system/1.wav",
        // .url = "http://116.205.135.166/dingding/clock/1.MP3",
        // .url = "http://116.205.135.166/dingding/system/2.wav",
        // .url = "http://116.205.135.166/dingding/noack/no_5.wav",
        .url = url_buf,
        .event_handler = _http_getMusic_handler,
        .timeout_ms = 10000000,
        .buffer_size = 8*1024,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 1);
    // mFp = fopen("/spiffs/welcome/welcome.wav","w+");
    // mFp = fopen("/spiffs/system/network.wav","w+");
    // mFp = fopen("/spiffs/wakeup/wakeup1.wav","w+");
    char url_file[40] = {0};
    snprintf(url_file, 40, "/spiffs/%s" ,url_buffer);
    printf("url_file = %s\n", url_file);
    mFp = fopen(url_file, "w+");
    //  mFp = fopen("/spiffs/clock/1.MP3","w+");
    // mFp = fopen("/spiffs/system/1.wav","w+");
    // mFp = fopen("/spiffs/system/2.wav","w+");
    //  mFp = fopen("/spiffs/noack/no_5.wav","w+");
    if(mFp == NULL)
    {
        printf("open file fail\n");
        return ;
    }
    printf("file open success\n");

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, " ", 1);

        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }
        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK) {
           int codRet = esp_http_client_get_status_code(client);
           int lenRet =  esp_http_client_get_content_length(client);
           ESP_LOGI(TAG, "codRet = %d,lenRet=%d", codRet, lenRet);
        } else {
            ESP_LOGI(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
    }
    esp_http_client_cleanup(client);
    printf("ok\n");
}

void get_all()
{
    // http_getMusic("welcome/welcome.wav");
    // vTaskDelay(pdMS_TO_TICKS(10000));
    // http_getMusic("system/network.wav");
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // http_getMusic("wakeup/wakeup1.wav");
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // http_getMusic("wakeup/wakeup2.wav");
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // http_getMusic("wakeup/wakeup3.wav");
    
    // vTaskDelay(pdMS_TO_TICKS(2000));
    ///////////////////////////////////////////
    // http_getMusic("wakeup/wakeup4.wav");
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // http_getMusic("system/1.wav");
    // vTaskDelay(pdMS_TO_TICKS(7000));
    // http_getMusic("system/2.wav");
    // vTaskDelay(pdMS_TO_TICKS(7000));
    // http_getMusic("noack/no_5.wav");
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // http_getMusic("clock/1.MP3");
    
}


void parse_raw_data(char *buf, int resLen)
{
    // 解析返回；
    int len =0;
    char *pLast= malloc(resLen);;
    
    pLast = strstr(buf, "id:");
    if (NULL != pLast)
    {
        pLast = pLast + 7; 
        while (pLast[0] != '}') {
            len++;
            pLast++;
            if(len >= resLen) {
                break;
            }
        }
    }
}

int removeSubstring(char *data_buf,const char* mainStr) {

#if 0
    const char* remaining = mainStr;  // 剩余的字符串指针
    int current_index = 0;
    const char *found = strstr(remaining, "id:");
    current_index = found - remaining;
    if(current_index )

    //char* result = (char*)malloc(strlen(mainStr) + 1);  // 分配足够的内存来存储结果字符串
    char current[4098] = {0};  // 当前位置指针
    const char* remaining = mainStr;  // 剩余的字符串指针
    int index = 0;

    const char *found = strstr(remaining, "id:");
    if(found != NULL)
    {
        strncpy(current, remaining, found - remaining);
        index = found - remaining;
        remaining = found + 39;
        
        // printf("remaining1 = %s\n", remaining);
    }
    
    const char *found2 = strstr(remaining, "event:audio\ndata:");
    if(found2 != NULL)
    {
        // printf("remaining1 = %s\n", remaining);
        strncpy(current+index, remaining, found2 - remaining);
        index += found2 - remaining;
        remaining = found2 + strlen("event:audio\ndata:");
        printf("remaining2 = %s\n", remaining);
    }
    if((found == NULL) && (found2 ==NULL))
    {
        return 1;
        // memcpy(data_buf, mainStr, strlen(mainStr));
    }
    // printf("remaining3 = %s\n", data_buf);
    // printf("remaining3 = %s\n", remaining);
    memcpy(data_buf, current, strlen(current));
    #endif
    return 0;
}

// 解码时使用    base64DecodeChars
static const unsigned char base64_suffix_map[256] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 255,
    255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
    52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
    255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
    7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
    19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
    37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255 };
int base64_decode(unsigned char *indata, int inlen,unsigned char *outdata, int *outlen) {
    
    int ret = 0;
    if (indata == NULL || inlen <= 0 ) {
        return ret = -1;
    }
    if (inlen % 4 != 0) { // 需要解码的数据不是4字节倍数
        return ret = -2;
    }
    
    int t = 0, x = 0, y = 0, i = 0;
    unsigned char c = 0;
    int g = 3;
    
    while (x < inlen) {
        // 需要解码的数据对应的ASCII值对应base64_suffix_map的值
        c = base64_suffix_map[indata[x++]];
        if (c == 255) {
            printf("ff = %c\n",indata[x-1]);
            continue;;}// 对应的值不在转码表中
        if (c == 253) continue;// 对应的值是换行或者回车
        if (c == 254) { c = 0; g--; }// 对应的值是'='
        t = (t<<6) | c; // 将其依次放入一个int型中占3字节
        if (++y == 4) {
            outdata[i++] = (unsigned char)((t>>16)&0xff);
            if (g > 1) outdata[i++] = (unsigned char)((t>>8)&0xff);
            if (g > 2) outdata[i++] = (unsigned char)(t&0xff);
            y = t = 0;
        }
    }
    // if (outlen != NULL) {
        *outlen = i;
    // }
    return ret;
}

//TEST
#include <mbedtls/base64.h>
static uint8_t g_buffer_index = 0;
uint32_t g_total_num = 0;
int first_flag = 0;
int write_finish_flag = 0;
// HTTP 请求的处理函数
esp_err_t _event_handler(esp_http_client_event_t *evt)
{
    // 缓存http响应的buffer
    static char *output_buffer;
    // 已经读取的字节数
    static int output_len;
    local_response_buffer = (char *) evt->user_data;
    char removebuffer[] = "event:audio";
    
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            #if 1
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    // memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        // output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    // memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
                printf("output_len=%d, evt->data_len = %d\n", output_len, evt->data_len); 
            }
            else {
                int evtLen = 0;
                if (evt->user_data) {
                    printf("1111evt->data_len = %d\n", evt->data_len);
                }
                else 
                {
                    
                    unsigned char buffer[1024] = {0x00};
                    size_t len = 0;
                    unsigned char data_buf222[1024] = {0};
                    memset(data_buf222, 0, 1024);
                    memcpy(data_buf222, evt->data, evt->data_len);
                    evtLen = evt->data_len;
                    printf("333evt->data_len = %d\n", evtLen);
                    if(mbedtls_base64_decode(buffer, 1024, &len, data_buf222,  1024) != 0 ) {
                        printf( "failed\n" );
                        //break;
                    }
                    // int r  = base64_decode(data_buf222, evt->data_len, buffer, &len);
                    // evtLen += evt->data_len;
                    printf("222evt->data_len = %d , len = %d\n", evt->data_len, len);
                    pp_raw_play(data_buf222, evt->data_len,1);
                    // char data_buf111[4098] = {0};
                    // char data_buf222[4098] = {0};
                    // memset(data_buf111, 0, 4098);
                    // memset(data_buf222, 0, 4098);
                    // memcpy(data_buf222, evt->data, evt->data_len);
                    // printf("evt->data = %s\n", (char *)data_buf222);

                    // int rest = removeSubstring(data_buf111, data_buf222);
                    // if(rest == 1)
                    // {
                    //     memcpy(data_buf111, data_buf222, evt->data_len);
                    // }
                    // printf("Result: %s\n", data_buf111);
                    // pp_raw_play((unsigned char *)data_buf111, strlen(data_buf111), 1);
                    // free(result);  // 释放结果字符串的内存
                    
                    #if 0
                    if(first_flag ==0)
                    {
                        // printf("111 %s\n",(char *)evt->data);
                        evtLen = evt->data_len-57;
                        memcpy(data_buf111, evt->data+57, evt->data_len-57);
                        first_flag = 1;
                        // printf("222%s\n",data_buf111);
                    }
                    else
                    {
                        evtLen = evt->data_len;
                        memcpy(data_buf111, evt->data, evt->data_len);    
                    }
                    #endif
                    // mbedtls_base64_self_test(1);
                    #if 1
                    // unsigned char buffer[4098] = {0x00};
                    // int len = 0;
                    // printf("111 %s\n", data_buf111);
                    // memset(buffer, 0, 4098);
                    // int r  = base64_decode(data_buf111, evtLen, buffer, &len);
                    // if(mbedtls_base64_decode(buffer, sizeof(buffer), &len, data_buf111, evtLen) != 0 ) {
                    //     printf( "failed\n" );
                    //     //break;
                    // }
                    #endif
                    // printf("\n,2222 evtLen=%d, evt->data = %s\n",evtLen, data_buf111);
                    // pp_raw_play(data_buf111, evtLen, 1);
                }
                g_total_num += evtLen;
            }
            #endif

            break;
        case HTTP_EVENT_ON_FINISH: //finish flag 
            printf("finish g_total_num = %ld\n", g_total_num);
            
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            if (output_buffer != NULL) {
                output_buffer = NULL;
            }
            
            
            break;
        case HTTP_EVENT_REDIRECT:
        break;
    }
    return ESP_OK;
}


void http_test()
{
    esp_http_client_config_t config = {
        //.url = "https://testdhome-api.scstit.com:9443/parse/openapi/parse/test/tts", ///parse/openapi/parse/test
        //.url = "http://139.159.141.213/aihome/base/test",
        //.url = "http://139.159.141.213/aihome/base/api",
        .url = "https://digital.yfjsn.com/openapi/content/music/stream/tts2?text=%E6%88%91%E7%88%B1%E4%BD%A0%E4%B8%AD%E5%9B%BD,%E6%88%91%E7%88%B1%E4%BD%A0%E7%9A%84%E5%A4%A7%E5%A5%BD%E6%B2%B3%E5%B1%B1",
        .event_handler = _event_handler,
        .buffer_size = 1*1024,//4*1024,
        .timeout_ms = 10000000,
    };
    //const char *post_data = "text=在一个遥远的国度里,有一个小镇名叫艾文郡.这个小镇被一片神秘的森林环绕着,人们传说那里住着一只传说中的神兽,它被称为'森林守护者'.有一天,小镇的年轻勇士艾德温决定踏上征程,寻找这只神兽,希望得到它的祝福,保护自己的家园.艾德温在他的冒险中结识了一群志同道合的伙伴:玛丽亚,一个勇敢而聪明的年轻女巫,和她的忠诚伙伴,一只名叫雷霆的魔法狼.他们也遇到了一个来自远方的流浪诗人,他名叫巴尔托.这个团队充满了勇气,智慧和友谊,他们共同面对了许多挑战和危险,但始终不放弃对于寻找森林守护者的信念.在他们的旅途中,他们遇到了各种各样的生物和障碍:从森林中的魔法精灵到被诅咒的巨人,再到湖中的水怪和山上的狡猾食人魔.但是,他们也学会了彼此信任,相互支持,并且在困难时刻共同努力.最终,他们终于找到了森林守护者,一只巨大的翼龙,它拥有着无与伦比的力量和智慧.森林守护者并不像人们传说中那样凶恶和危险,相反,它是一只善良而慈爱的生物，它告诉艾德温和他的伙伴，勇气、友谊和团结才是真正的力量。它赐予他们祝福，并向他们传授了保护自己家园的方法，使他们成为了艾文郡的英雄。艾德温和他的伙伴们回到了小镇，带着森林守护者的祝福和智慧。他们成为了小镇的守护者，用他们的勇气和智慧保护着这片美丽的土地，让人们能够安居乐业。这个故事告诉我们，当我们勇敢地面对挑战，并与他人团结合作时，我们可以战胜一切困难，创造出更美好的未来.";
    //const char *post_data = "{\"text\":\"在一个遥远的国度里,有一个小镇名叫艾文郡.这个小镇被一片神秘的森林环绕着\"}";
    const char *post_data = "text=%E6%88%91%E7%88%B1%E4%BD%A0%E4%B8%AD%E5%9B%BD,%E6%88%91%E7%88%B1%E4%BD%A0%E7%9A%84%E5%A4%A7%E5%A5%BD%E6%B2%B3%E5%B1%B1";
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    // esp_http_client_set_header(client, "Content-Type", "application/json;charset=UTF-8");
    //esp_http_client_set_header(client, "Content-Type", "charset=UTF-8");
    //esp_http_client_set_header(client, "AppId", "h5f972d6a6e30b45");
    //esp_http_client_set_header(client, "AppVersion", "1.0.0");
    esp_err_t err = esp_http_client_open(client, strlen(post_data));

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    } else {
        int wlen = esp_http_client_write(client, post_data, strlen(post_data));
        if (wlen < 0) {
            ESP_LOGE(TAG, "Write failed");
        }

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
           int codRet = esp_http_client_get_status_code(client);
           int lenRet =  esp_http_client_get_content_length(client);
           ESP_LOGI(TAG, "codRet = %d,lenRet=%d", codRet, lenRet);
        } else {
            ESP_LOGI(TAG, "Error perform http request %s", esp_err_to_name(err));
        }
    }
    esp_http_client_cleanup(client);
    printf("ok123 = %d\n", strlen(post_data));
}

/*******************************************************************************************/



/*************************************music*************************************************/
#include <stdlib.h>
char *welcome_sound_lists[] = {
    "file://spiffs/welcome/welcome.wav"
};


extern esp_dispatcher_handle_t pp_dispatcher;
extern esp_audio_status_t  g_playFlag;
int test_flag = 1;
int pp_music_play(char *url, uint8_t level,int interrupt_flag)
{
    player_msg_t msg = { 0 };
    msg.id = PLAYER_CMD_PLAYMUSIC;
    memcpy(msg.url, url, strlen(url));
    msg.level = level;
    // printf("url = %s\n", msg.url);
    if(interrupt_flag) {
        if (xQueueSendFromISR(player_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    } else {
        if (xQueueSend(player_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    }
    return 0;
}







int pp_raw_play(unsigned char *data, int len,int interrupt_flag)
{
    action_arg_t arg = {
                        .data = data,
                        .len = len,
                    };
    esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_RAW_PLAY, &arg, NULL);
    #if 0
    player_msg_t msg = { 0 };
    msg.id = PLAYER_CMD_PLAYRAW;

    
    memcpy(msg.raw_data, data, len);
    msg.level = 1;
    msg.raw_length = len;
    
    if(interrupt_flag == 1) {
        if (xQueueSendFromISR(player_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    } else if(interrupt_flag == 0) {
        if (xQueueSend(player_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    }
    #endif
    return 0;
}


void wakeup_tone(int inter)
{
    char *wakeup_sound_lists[] = {
    "file://spiffs/wakeup/wakeup1.wav",
    "file://spiffs/wakeup/wakeup2.wav",
    "file://spiffs/wakeup/wakeup3.wav",
    "file://spiffs/wakeup/wakeup4.wav"
    // "file://spiffs/wakeup/wakeup5.wav"
    // "file://spiffs/wakeup/wakeup6.wav"
    // "file://spiffs/wakeup/wakeup7.wav"
    };
    int r = rand() % 4;
    if(r<4) {
        printf("r = %d\n", r);
        pp_music_play(wakeup_sound_lists[r], 5, inter);
    }
}



#define SYSTEM_PLAYLIST (BIT0)
static EventGroupHandle_t       music_evt        = NULL;

static int g_playlistFlag=1;

int g_list_num = 10;
extern int g_networkmusic_playlist;
void music_playList_system(void *args)
{
    int current_num=0;
    int current_num2 = 0;
    g_music_playlist = 0;
    g_networkmusic_playlist = 0;
    int url_len = 0;
    
    while(1)
    {
        if(current_num == g_music_playlist)
        {
            url_len = strlen(g_music_list[current_num]);
            printf("current_num = %d, len = %d, g_music_list = %s\n",current_num, url_len, g_music_list[current_num]);
            pp_music_play(g_music_list[current_num],6,0);
            led_blink_onoff(1);
            current_num ++;
            if(current_num == g_list_num)
            {
                g_music_playlist = -1;
                break;
            }
        }
       
        vTaskDelay(10);
    }
    // xEventGroupClearBits(music_evt, SYSTEM_PLAYLIST);
    vTaskDelete(NULL);
}

#include "audio_element.h"
#include "audio_pipeline.h"
#include "raw_stream.h"
TaskHandle_t music_task;
TaskHandle_t networkmusic_task;

void pp_playControl(int n) //start & pause
{
    if(n == 1)
    {
        esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_RESUME, NULL, NULL);
    }
    else if(n == 0)
    {
        esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_PAUSE, NULL, NULL);
    }
}

void pp_setVolNum(int n)
{
    action_arg_t arg;
    arg.len = 1;
    arg.data = n;
    esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_VOLUME_UP, &arg, NULL);
}

extern char g_networkmusic_list[50][15];
void networkmusic_playList_system(void *args)
{
    int current_num2 = 0;
    g_networkmusic_playlist = 0;
    int url_len = 0;
    
    while(1)
    {
        if(current_num2 == g_networkmusic_playlist)
        {
            printf("playing  = %d\n", current_num2);
            led_blink_onoff(1);
            pp_play_networkmusic_by_id(current_num2);
            reple_single_music_play("report", 1, g_networkmusic_list[current_num2], "5263234234") ;
            current_num2 ++;
            if(current_num2 == 50)
            {
                g_networkmusic_playlist = -1;
                break;
            }
        }
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}



void pp_stop_musicList_play()
{
    esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
    g_music_playlist = -1;
    led_blink_onoff(2);
    if(music_task != NULL)
    {
        vTaskDelete(music_task);
    }
    
}

void pp_stop_networkmusicList_play()
{
    esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
    g_networkmusic_playlist = -1;
    led_blink_onoff(2);
    if(networkmusic_task != NULL)
    {
        vTaskDelete(networkmusic_task);
    }
    
}

int pp_networkmusicList_play()
{
    pp_stop_musicList_play();
    BaseType_t ret = xTaskCreate(networkmusic_playList_system, "PlayListTask", 5*1024, NULL, 5, &networkmusic_task);
    return 0;
}
int pp_musicList_play()
{
    pp_stop_networkmusicList_play();
    BaseType_t ret = xTaskCreate(music_playList_system, "PlayListTask", 10*1024, NULL, 5, &music_task);
    return 0;
}




#include "raw_stream.h"
#include "audio_mem.h"
extern audio_element_handle_t i2s_h;
void music_play_system(void *args)
{
    bool running = true;
    player_msg_t msg;
    while (running)
    {
        xQueueReceive(player_cmd_queue, &msg, portMAX_DELAY);
        printf("g_playFlag = %d, msg.url = %s\n", g_playFlag, msg.url);
        switch (msg.id) {
            case PLAYER_CMD_PLAYMUSIC:
                    char urlTemp[500] = {0};
                    memcpy(urlTemp, msg.url, strlen(msg.url));
                    action_arg_t arg = {
                        .data = urlTemp,
                        .len = strlen(urlTemp),
                    };
                    // if(g_playFlag == AUDIO_STATUS_STOPPED || g_playFlag == AUDIO_STATUS_FINISHED)
                    if(msg.level >= 3)
                    {
                        if(pp_dispatcher == NULL)
                        {
                            printf("pp_dispatcher error \n");
                            break;
                        }
                        // while(http_status == 1 );
                        esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_PLAY, &arg, NULL);
                        ESP_LOGI(TAG, "[Step 8.0] Initialize Done");
                    }
                    else
                    {
                        if(msg.level < 3 )
                        {
                            esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
                            ESP_LOGI(TAG, "[Step 8.0] stop Done");
                            vTaskDelay(pdMS_TO_TICKS(1000));
                            esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_PLAY, &arg, NULL);
                            ESP_LOGI(TAG, "[Step 8.1] Initialize Done");
                        }
                    }
            break;
            case PLAYER_CMD_PLAYRAW:

            break;
        }
        //vTaskDelay(1);
        //play raw music
        // if(rawRingIndex != 0)
        // {
        //     int ret = raw_stream_write(i2s_h, (char *)raw_ringbuffer, rawLength);
        //     rawRingIndex = rawRingIndex - rawLength;
        //     printf("el->state = %d\n", audio_element_get_state(i2s_h));
        //     rawLength = 0;
        //     // vTaskDelay(5);
        // }
        
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}


void pp_music_init()
{
    // pipeRawinit();
    player_cmd_queue = xQueueCreate(2, sizeof(player_msg_t));
    BaseType_t ret = xTaskCreate(music_play_system, "Task", 22*1024, NULL, 1, NULL);
    printf("aaaaaa ret = %d\n", ret);
}
/*******************************************************************************************/
#include "driver/gpio.h"
int g_led_msg = 0;
void tone_led_system(void *args)
{
    bool running = true;
    
    while (running)
    {
        switch (g_led_msg) {
            case 1: // 闪
                gpio_set_direction(3, GPIO_MODE_OUTPUT);
                gpio_set_level(3, 0);
                vTaskDelay(100);
                gpio_set_direction(3, GPIO_MODE_OUTPUT);
                gpio_set_level(3, 1);
                vTaskDelay(100);
            break;
            case 2: //开
                gpio_set_direction(3, GPIO_MODE_OUTPUT);
                gpio_set_level(3, 0);
            break;
            case 3: //关
                gpio_set_direction(3, GPIO_MODE_OUTPUT);
                gpio_set_level(3, 1);
            break;
        }
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}



void pp_led_init()
{
    BaseType_t ret = xTaskCreate(tone_led_system, "LEDTask", 1024, NULL, 6, NULL);
    printf("pp_led_init ret = %d\n", ret);
}


void led_blink_onoff(int n)
{
    g_led_msg = n;
}