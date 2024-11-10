/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"

#include "esp_peripherals.h"
#include "audio_mem.h"
#include "audio_setup.h"
#include "esp_dispatcher_dueros_app.h"
#include "esp_player_wrapper.h"
#include "duer_audio_action.h"

#include "display_service.h"
#include "dueros_service.h"
#include "wifi_service.h"
#include "airkiss_config.h"
#include "smart_config.h"
#include "blufi_config.h"

#include "input_key_service.h"
#include "input_key_com_user_id.h"
#include "esp_dispatcher.h"
#include "esp_action_exe_type.h"
#include "wifi_action.h"
#include "display_action.h"
#include "dueros_action.h"
#include "recorder_action.h"
#include "player_action.h"
#include "audio_recorder.h"
#include "esp_delegate.h"
#include "audio_thread.h"
#include "app_task.h"
#include "esp_vad.h"
#include "app_system_info.h"
#include "app_mqtt.h"


#define DUER_REC_READING (BIT0)

#define VAD_SAMPLE_RATE_HZ 16000
#define VAD_FRAME_LENGTH_MS 30
#define VAD_BUFFER_LENGTH (VAD_FRAME_LENGTH_MS * VAD_SAMPLE_RATE_HZ / 1000)

static const char               *TAG            = "AISOUNDBox";
esp_dispatcher_dueros_speaker_t *dueros_speaker = NULL;
static EventGroupHandle_t       duer_evt        = NULL;

int g_rec_status = 0; //对话标识
void stop_rec()
{
    xEventGroupClearBits(duer_evt, DUER_REC_READING);
    file_stop_recorde();
    ESP_LOGE(TAG, "Read Stopped");
}

void start_rec()
{
    ESP_LOGI(TAG, "rec_engine_cb - AUDIO_REC_VAD_START");
    xEventGroupSetBits(duer_evt, DUER_REC_READING);
}

int g_recStopFlag = 0;
extern uint32_t g_rec_count;
extern int spk_count;
extern void http_rec_check_url(char *request, uint32_t request_len);
static void voice_read_task(void *args)
{
    // const int buf_len = 2 * 1024;
    const int buf_len = 3 * 1024;
    const int buf_len2 = 250 * buf_len;
    
    char *voiceData = audio_calloc(1, buf_len);
    char *voiceData2 = audio_calloc(1, buf_len2);
    bool runing = true;
    esp_dispatcher_dueros_speaker_t *d = (esp_dispatcher_dueros_speaker_t *)args;
    vad_handle_t vad_inst = vad_create(VAD_MODE_3);
    static uint32_t ret_len = 0;
    while (runing)
    {
        EventBits_t bits = xEventGroupWaitBits(duer_evt, DUER_REC_READING, false, true, portMAX_DELAY);
        if (bits & DUER_REC_READING) {
            
            int ret = audio_recorder_data_read(d->recorder, voiceData, buf_len, portMAX_DELAY);
            
            
            ESP_LOGE(TAG,"ret_len = %d, voiceData[0] = %d\n", ret, voiceData[100]);
            #if 1
            if (ret == 0 || ret == -1) {
                xEventGroupClearBits(duer_evt, DUER_REC_READING);
                ESP_LOGE(TAG, "Read Finished");
            }
            else 
            {
                printf("Speech SILENCE, g_rec_count = %ld\n", g_rec_count);
                // Feed samples to the VAD process and get the result
                vad_state_t vad_state = vad_process(vad_inst, voiceData, VAD_SAMPLE_RATE_HZ, VAD_FRAME_LENGTH_MS);
                if (vad_state == VAD_SPEECH) {
                    ESP_LOGI(TAG, "Speech detected");
                    g_rec_count = 100;
                }
                else if(vad_state == VAD_SILENCE) //3s stop
                {
                    // printf("Speech SILENCE, g_rec_count = %ld\n", g_rec_count);
                    if(g_rec_count == 50) //没说话，5秒提示，13秒结束
                    {
                        printf("are you living?\n");
                        //esp_dispatcher_execute(dueros_speaker->dispatcher, ACTION_EXE_TYPE_REC_WAV_TURN_OFF, NULL, NULL);
                    }
                    if((g_rec_count >= 90) && (g_rec_count < 100))
                    {
                        printf("rec die\n");
                        g_rec_status = 0;
                        ret_len = 0;
                        esp_dispatcher_execute(dueros_speaker->dispatcher, ACTION_EXE_TYPE_REC_WAV_TURN_OFF, NULL, NULL);
                        pp_music_play("file://spiffs/noack/no_4.wav", 6, 0);
                    }
                    if(g_rec_count >= 115) //说完话等1.5s结束
                    {
                        printf("rec stop\n");
                        esp_dispatcher_execute(dueros_speaker->dispatcher, ACTION_EXE_TYPE_REC_WAV_TURN_OFF, NULL, NULL);

                    }
                }

                memcpy(voiceData2 + ret_len, voiceData, ret);
                ret_len += ret;
                // pp_file_write(voiceData, ret);
            }
            #endif
        }
        printf("g_recStopFlag = %d, spk_count = %d, g_rec_count=%ld\n",g_recStopFlag, spk_count, g_rec_count);

        if(g_recStopFlag == 1 || spk_count == 150)
        {
            led_blink_onoff(2);
            if(spk_count == 150) //说话超时；
            {
                g_rec_status = 0;
            }
            http_rec_check_url(voiceData2, ret_len);
            // pp_file_write(voiceData2, ret_len);
            xEventGroupClearBits(duer_evt, DUER_REC_READING);
            g_recStopFlag = 0;
            spk_count = 0;  

            printf("finish!!!!rec len = %ld\n", ret_len);

            memset(voiceData2, 0, ret_len);
            ret_len = 0;
        }
    }

    xEventGroupClearBits(duer_evt, DUER_REC_READING);
    free(voiceData);
    free(voiceData2);
    vTaskDelete(NULL);
}

esp_audio_status_t  g_playFlag = AUDIO_STATUS_STOPPED;
int g_music_playlist = -1;
int g_networkmusic_playlist = -1;

static void esp_audio_callback_func(esp_audio_state_t *audio, void *ctx)
{
    ESP_LOGE(TAG, "ESP_AUDIO_CALLBACK_FUNC, st:%d,err:%d,src:%x",
             audio->status, audio->err_msg, audio->media_src);
    // ESP_LOGE(TAG, "audio->status = %d", audio->status);
    if (audio->status == AUDIO_STATUS_FINISHED) 
    {
        g_playFlag = AUDIO_STATUS_FINISHED;
        if(g_music_playlist >= 0)
        {
            g_music_playlist ++;
        }
        if(g_networkmusic_playlist >= 0)
        {
            g_networkmusic_playlist++;
        }

        if(g_rec_status == 2)
        {
            wakeup_rec_start(0,0);
            // g_rec_status = 0;
        }
        printf("+++++++++++AUDIO_STATUS_FINISHED,status = %d+++++++++++\n", audio->status);
    }
    else if(audio->status == AUDIO_STATUS_STOPPED)
    {
        ESP_LOGE(TAG, "AUDIO_STATUS_STOPPED");
        g_playFlag = AUDIO_STATUS_STOPPED;

    }
    else if(audio->status == AUDIO_STATUS_RUNNING)
    {
        g_playFlag = AUDIO_STATUS_RUNNING;
    }
    else if(audio->status == AUDIO_STATUS_PAUSED)
    {
        ESP_LOGE(TAG, "AUDIO_STATUS_PAUSED");
         g_playFlag = AUDIO_STATUS_PAUSED;
    }
    
}

static esp_err_t rec_engine_cb(audio_rec_evt_t *event, void *user_data)
{
    esp_dispatcher_dueros_speaker_t *d = (esp_dispatcher_dueros_speaker_t *)user_data;
    if (AUDIO_REC_WAKEUP_START == event->type) {
        ESP_LOGI(TAG, "rec_engine_cb - AUDIO_REC_WAKEUP_START");
    } else if (AUDIO_REC_VAD_START == event->type) {
        ESP_LOGI(TAG, "rec_engine_cb - AUDIO_REC_VAD_START");
        xEventGroupSetBits(duer_evt, DUER_REC_READING);
    } else if (AUDIO_REC_VAD_END == event->type) {
        xEventGroupClearBits(duer_evt, DUER_REC_READING);
        ESP_LOGI(TAG, "rec_engine_cb - AUDIO_REC_VAD_STOP, state:%d", dueros_service_state_get());
    } else if (AUDIO_REC_WAKEUP_END == event->type) {
        ESP_LOGI(TAG, "rec_engine_cb - AUDIO_REC_WAKEUP_END");
    } else if (AUDIO_REC_COMMAND_DECT <= event->type) {
        recorder_sr_mn_result_t *mn_result = event->event_data;
        ESP_LOGI(TAG, "rec_engine_cb - AUDIO_REC_COMMAND_DECT");
        ESP_LOGI(TAG, "command %d, phrase_id %d, prob %f, str: %s", event->type, mn_result->phrase_id, mn_result->prob, mn_result->str);
        // esp_audio_sync_play(player, tone_uri[TONE_TYPE_HAODE], 0);
    } else {
        ESP_LOGE(TAG, "Unkown event");
    }

    return ESP_OK;
}

wifi_service_event_t g_wifiState = 0;
static esp_err_t wifi_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    ESP_LOGE(TAG, "event type:%d,source:%p, data:%p,len:%d,ctx:%p",
             evt->type, evt->source, evt->data, evt->len, ctx);
    esp_dispatcher_dueros_speaker_t *d = (esp_dispatcher_dueros_speaker_t *)ctx;
    if (evt->type == WIFI_SERV_EVENT_CONNECTED) {
        g_wifiState = WIFI_SERV_EVENT_CONNECTED;
        d->wifi_setting_flag = false;
        // esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_DUER_CONNECT, NULL, NULL);
        esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_DISPLAY_WIFI_CONNECTED, NULL, NULL);

        // static int g_connect_flag = -1;

        // if(g_connect_flag != -1)
        // {
        //     pp_music_play("file://spiffs/system/1.wav", 6, 0);
        // }
        // g_connect_flag ++ ;
    } else if (evt->type == WIFI_SERV_EVENT_DISCONNECTED) {
        g_wifiState = WIFI_SERV_EVENT_DISCONNECTED;
        // esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_DUER_DISCONNECT, NULL, NULL);
        esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_DISPLAY_WIFI_CONNECTED, NULL, NULL);
        
    } else if (evt->type == WIFI_SERV_EVENT_SETTING_TIMEOUT) {
        g_wifiState = WIFI_SERV_EVENT_SETTING_TIMEOUT;
        d->wifi_setting_flag = false;
        
    }
    return ESP_OK;
}


static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    esp_dispatcher_dueros_speaker_t *d = (esp_dispatcher_dueros_speaker_t *)ctx;
    printf("[ * ] [Set]evt->data,%d, %d, %d\n", (int )evt->data, (int )evt->source, (int )evt->type);
    switch ((int)evt->data) {
        case INPUT_KEY_USER_ID_MUTE:  //1 进入识别   配对
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) { //进入识别
                ESP_LOGI(TAG, "[ * ] [ACTION]识别");
                wakeup_rec_start(1,0);
            }  else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) { //长按 配对
                ESP_LOGI(TAG, "[ * ] [ACTION]配网");
                if (d->wifi_setting_flag == false) {
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_WIFI_SETTING_START, NULL, NULL);
                    d->wifi_setting_flag = true;
                } else {
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_WIFI_SETTING_STOP, NULL, NULL);
                    d->wifi_setting_flag = false;
                }
            }
            break;
        case INPUT_KEY_USER_ID_PLAY: //2 上一首
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) { //释放 音量减
                ESP_LOGI(TAG, "[ * ] [Vol-] input key event");
                esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_VOLUME_DOWN, NULL, NULL);

            }  else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) { //长按 上一首
                if(g_music_playlist != -1)
                {
                    g_music_playlist --;
                    
                    if(g_music_playlist <= 1){
                        g_music_playlist = 10;
                    }
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
                }
                if(g_networkmusic_playlist != -1)
                {
                    g_networkmusic_playlist --;
                    
                    if(g_networkmusic_playlist <= 1){
                        g_networkmusic_playlist = pp_get_music_count() - 1;
                    }
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
                }
            }
            break;
        case INPUT_KEY_USER_ID_SET: //3 播放/暂停
             if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) { //释放 播放/暂停
                ESP_LOGI(TAG, "[ * ] [ACTION]播放");
                if(g_playFlag == AUDIO_STATUS_PAUSED)
                {
                    // reply_control_play("report", 1, 0, "5263234234"); 
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_RESUME, NULL, NULL);
                }
                else if(g_playFlag == AUDIO_STATUS_RUNNING)
                {
                    // reply_control_play("report", 1, 1, "5263234234"); 
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_PAUSE, NULL, NULL);
                }
            }  else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) { //长按
                g_music_playlist = -1;
                esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
            }
            break;
        case INPUT_KEY_USER_ID_VOLDOWN: //4 下一首
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) { //释放 音量加
                ESP_LOGI(TAG, "[ * ] [Vol+] input key event");
                esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_VOLUME_UP, NULL, NULL);
            }  else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) { //长按 下一首
                if(g_music_playlist != -1)
                {
                    g_music_playlist ++;
                    
                    if(g_music_playlist >= 10){
                        g_music_playlist = 1;
                    }
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
                }

                if(g_networkmusic_playlist != -1)
                {
                    g_networkmusic_playlist ++;
                    
                    if(g_networkmusic_playlist >= pp_get_music_count()) {
                        g_networkmusic_playlist = 1;
                    }
                    esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
                }

            }
            break;
        case INPUT_KEY_USER_ID_VOLUP: // 5 模式切换
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) { //释放 静音
                ESP_LOGI(TAG, "[ * ] [Vol mute] ACTION_EXE_TYPE_AUDIO_MUTE_ON");
                esp_dispatcher_execute(d->dispatcher, ACTION_EXE_TYPE_AUDIO_MUTE_ON, NULL, NULL);
            }  else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) { //长按
                ESP_LOGI(TAG, "[ * ] [ACTION]切换模式");
                pp_music_play("file://spiffs/system/network.wav", 6, 0);
            }
            break;
    }

    return ESP_OK;
}

static esp_err_t initialize_ble_stack(void)
{
    esp_err_t ret = ESP_OK;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret) {
       ESP_LOGE(TAG, "%s init bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}


#include "esp_spiffs.h"
void spiflash()
{
    // 初始化 SPIFFS
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",  // 指定 SPIFFS 的挂载路径
      .partition_label = "SPIFFS",  // 分区标签，如果为 NULL，则使用默认的 SPIFFS 分区
      .max_files = 5,  // SPIFFS 可以打开的最大文件数
      .format_if_mount_failed = true  // 如果挂载失败，是否格式化 SPIFFS
    };

    // 注册 SPIFFS 到 VFS
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    // 检查 SPIFFS 是否成功初始化
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            // ESP_LOGE("TAG", "无法挂载或格式化文件系统");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            // ESP_LOGE("TAG", "未找到 SPIFFS 分区");
        } else {
            // ESP_LOGE("TAG", "无法初始化 SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    // 写入文件
    FILE* f = fopen("/spiffs/rec.pcm", "w+");  // 打开一个文件进行写入
    if (f == NULL) {
        // ESP_LOGE("TAG", "无法打开文件进行写入");
        return;
    }
    fprintf(f, "123");  // 向文件中写入数据
    fclose(f);  // 关闭文件

    // 读取文件
    char line[64];
    f = fopen("/spiffs/rec.pcm", "r");  // 打开一个文件进行读取
    if (f == NULL) {
        // ESP_LOGE("TAG", "无法打开文件进行读取");
        return;
    }
    fgets(line, sizeof(line), f);  // 从文件中读取一行数据
    fclose(f);  // 关闭文件

    // 打印文件内容
    printf("从文件中读取：'%s'\n", line);
}

extern void http_test();
extern void http_getMusic();
extern void get_all();
esp_dispatcher_handle_t pp_dispatcher;
void duer_app_init(void)
{
 
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "ADF version is %s", ADF_VER);

    ESP_LOGI(TAG, "Step 1. Create dueros_speaker instance");
    dueros_speaker = audio_calloc(1, sizeof(esp_dispatcher_dueros_speaker_t));
    AUDIO_MEM_CHECK(TAG, dueros_speaker, return);
    
    pp_dispatcher = esp_dispatcher_get_delegate_handle();
    dueros_speaker->dispatcher = pp_dispatcher;
    // i2c_led_init();
    ESP_LOGI(TAG, "[Step 2.0] Create esp_periph_set_handle_t instance and initialize Touch, Button, SDcard");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);
    audio_board_key_init(set);
    // audio_board_sdcard_init(set, SD_MODE_1_LINE);
    spiflash();
    ESP_LOGI(TAG, "[Step 2.1] Initialize input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    dueros_speaker->input_serv = input_key_service_create(&input_cfg);
    input_key_service_add_key(dueros_speaker->input_serv, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(dueros_speaker->input_serv, input_key_service_cb, (void *)dueros_speaker);

    ESP_LOGI(TAG, "[Step 3.1] Register display service execution type");
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->disp_serv,
                                ACTION_EXE_TYPE_DISPLAY_TURN_OFF, display_action_turn_off);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->disp_serv,
                                ACTION_EXE_TYPE_DISPLAY_TURN_ON, display_action_turn_on);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->disp_serv,
                                ACTION_EXE_TYPE_DISPLAY_WIFI_SETTING, display_action_wifi_setting);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->disp_serv,
                                ACTION_EXE_TYPE_DISPLAY_WIFI_CONNECTED, display_action_wifi_connected);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->disp_serv,
                                ACTION_EXE_TYPE_DISPLAY_WIFI_DISCONNECTED, display_action_wifi_disconnected);

    ESP_LOGI(TAG, "[Step 5.0] Initialize esp player");
    dueros_speaker->player = setup_player(esp_audio_callback_func, NULL);
    esp_player_init(dueros_speaker->player);

    ESP_LOGI(TAG, "[Step 4.0] Initialize recorder engine");
    dueros_speaker->recorder = setup_recorder(rec_engine_cb, dueros_speaker);

    ESP_LOGI(TAG, "[Step 4.1] Register wanted recorder execution type");
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->recorder,
                                ACTION_EXE_TYPE_REC_WAV_TURN_OFF, recorder_action_rec_wav_turn_off);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->recorder,
                                ACTION_EXE_TYPE_REC_WAV_TURN_ON, recorder_action_rec_wav_turn_on);
                              
    ESP_LOGI(TAG, "[Step 5.1] Register wanted player execution type");
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_RAW_PLAY, player_raw_play);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_PLAY, player_action_play);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_PAUSE, player_action_pause);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_RESUME, player_action_resume);

    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_VOLUME_UP, player_action_vol_up);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_VOLUME_DOWN, player_action_vol_down);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_STOP, esp_player_music_stop);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_MUTE_ON, player_action_mute_on);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->player, ACTION_EXE_TYPE_AUDIO_MUTE_OFF, player_action_mute_off);

    ESP_LOGI(TAG, "[Step 7.0] Create Wi-Fi service instance");
    wifi_config_t sta_cfg = {0};
    strncpy((char *)&sta_cfg.sta.ssid, CONFIG_WIFI_SSID, 12);
    strncpy((char *)&sta_cfg.sta.password, CONFIG_WIFI_PASSWORD, 9);
    
    // memcpy(sta_cfg.sta.ssid, "AIstar2.4G", 11);
    // memcpy(sta_cfg.sta.password, "aistar88792356", 15);
    wifi_service_config_t cfg = WIFI_SERVICE_DEFAULT_CONFIG();
    cfg.evt_cb = wifi_service_cb;
    cfg.cb_ctx = dueros_speaker;
    cfg.setting_timeout_s = 60;
    
    dueros_speaker->wifi_serv = wifi_service_create(&cfg);
    
    ESP_LOGI(TAG, "[Step 7.1] Register wanted display service execution type");
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->wifi_serv,
                                ACTION_EXE_TYPE_WIFI_CONNECT, wifi_action_connect);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->wifi_serv,
                                ACTION_EXE_TYPE_WIFI_DISCONNECT, wifi_action_disconnect);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->wifi_serv,
                                ACTION_EXE_TYPE_WIFI_SETTING_STOP, wifi_action_setting_stop);
    esp_dispatcher_reg_exe_func(pp_dispatcher, dueros_speaker->wifi_serv,
                                ACTION_EXE_TYPE_WIFI_SETTING_START, wifi_action_setting_start);

    ESP_LOGI(TAG, "[Step 7.2] Initialize Wi-Fi provisioning type(AIRKISS, SMARTCONFIG or ESP-BLUFI)");
    int reg_idx = 0;

    esp_wifi_setting_handle_t h = NULL;
#ifdef CONFIG_AIRKISS_ENCRYPT
    airkiss_config_info_t air_info = AIRKISS_CONFIG_INFO_DEFAULT();
    air_info.lan_pack.appid = CONFIG_AIRKISS_APPID;
    air_info.lan_pack.deviceid = CONFIG_AIRKISS_DEVICEID;
    air_info.aes_key = CONFIG_DUER_AIRKISS_KEY;
    h = airkiss_config_create(&air_info);
#elif (defined CONFIG_ESP_SMARTCONFIG)
    smart_config_info_t info = SMART_CONFIG_INFO_DEFAULT();
    h = smart_config_create(&info);
#elif (defined CONFIG_ESP_BLUFI_PROVISIONING)
    initialize_ble_stack();
    h = blufi_config_create(NULL);
#endif
    
    esp_wifi_setting_register_notify_handle(h, (void *)dueros_speaker->wifi_serv);
    wifi_service_register_setting_handle(dueros_speaker->wifi_serv, h, &reg_idx);
    wifi_service_set_sta_info(dueros_speaker->wifi_serv, &sta_cfg);
    wifi_service_connect(dueros_speaker->wifi_serv);

    duer_evt = xEventGroupCreate();
    audio_thread_create(NULL, "voice_read_task", voice_read_task, dueros_speaker, 25 * 1024, 5, true, 1);
    vTaskDelay(pdMS_TO_TICKS(2000));
#if 1
    pp_clock_init();
    pp_music_init();

    vTaskDelay(pdMS_TO_TICKS(7000));
    http_getNetwork_time();

    pp_music_play("file://spiffs/welcome/welcome.wav", 5, 0);
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    printf("网路状态：g_wifiState = %d\n", g_wifiState);
    if(g_wifiState == WIFI_SERV_EVENT_CONNECTED)
    {
        pp_music_play("file://spiffs/system/1.wav", 6, 0);
    }
    else if(g_wifiState == WIFI_SERV_EVENT_DISCONNECTED)
    {
        pp_music_play("file://spiffs/system/2.wav", 6, 0);
    }

    rec_wakeup();
    pp_led_init();
    // led_blink_onoff(1);
    vTaskDelay(pdMS_TO_TICKS(10000));
    // file_task_init();
    ESP_LOGI(TAG, "[Step 8.0] Initialize Done,123");

    app_mqtt_start();
    
    //http_test();
    i2c_led_init();
    vTaskDelay(pdMS_TO_TICKS(10000));
    // pp_music_play("https://static-sichuanshutian-open.haifanwu.com/WaterMark/hifive/COPYRIGHT/B7B810B0F232/mp3_128/B7B810B0F232.mp3?param=LhcJJT4uNa4AkWr1dspm2PyrGqDBIXG0AAQ7GSRebcY51l1Dq671Iqdx3TyNExqYjGAysaAHGgJrgRpOdytzY2tbsYK8U-tbc8qroHr-sB5KZxM8mBwYkwSrzeitr2BsUcXnQ6W6uHKicZPiN8fin82s_tHEUTz-TQQTsJ_4IKA&sign=5bd2343662d500922611a0d51b7a3975&t=1724614574",5,0);
    // pp_music_play("file://spiffs/clock/1.MP3", 6, 0);
    // pp_music_play("http://116.205.135.166/dingding/system/networkdisconnect.wav",5,0);
    // get_all();
#endif    
}
