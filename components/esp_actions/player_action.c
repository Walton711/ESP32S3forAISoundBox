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

#include "audio_error.h"
#include "esp_action_def.h"
#include "esp_audio.h"
#include "esp_log.h"
#include "raw_stream.h"
#include "app_mqtt.h"
static const char *TAG = "PLAYER_ACTION";
extern audio_element_handle_t el_raw_writer;
extern audio_element_handle_t i2s_h;
esp_err_t player_raw_play(void *instance, action_arg_t *arg, action_result_t *result)
{
    esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    ESP_LOGI(TAG, "%s", __func__);
    ESP_LOGI(TAG, "url = %s", (char *)arg->data);
    // 
    int ret = raw_stream_write(i2s_h, (char *)arg->data, arg->len);
    printf("raw_stream_write ret = %d\n",ret);
    // int ret = esp_audio_play(handle, AUDIO_CODEC_TYPE_DECODER, (char *)arg->data, 0);
    return 1;
}

esp_err_t player_action_play(void *instance, action_arg_t *arg, action_result_t *result)
{
    esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    ESP_LOGI(TAG, "%s", __func__);
    ESP_LOGI(TAG, "url = %s", (char *)arg->data);
    int ret = esp_audio_play(handle, AUDIO_CODEC_TYPE_DECODER, (char *)arg->data, 0);
    return ret;
}

esp_err_t player_action_pause(void *instance, action_arg_t *arg, action_result_t *result)
{
    ESP_LOGI(TAG, "%s", __func__);
    esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    int ret = esp_audio_pause(handle);
    return ret;
}

esp_err_t player_action_resume(void *instance, action_arg_t *arg, action_result_t *result)
{
    ESP_LOGI(TAG, "%s", __func__);
    esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    int ret = esp_audio_resume(handle);
    return ret;
}


esp_err_t player_action_next(void *instance, action_arg_t *arg, action_result_t *result)
{
    ESP_LOGI(TAG, "%s", __func__);

    return ESP_OK;
}

esp_err_t player_action_prev(void *instance, action_arg_t *arg, action_result_t *result)
{

    ESP_LOGI(TAG, "%s", __func__);

    return ESP_OK;
}

esp_err_t player_action_vol_up(void *instance, action_arg_t *arg, action_result_t *result)
{
    int player_volume = 0;
    esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    if(arg->len != NULL)
    {
        if(arg->data >= 0 || arg->data<=100)
        {
            esp_audio_vol_set(handle, arg->data);
            ESP_LOGI(TAG, "%s, vol:[%d]", __func__, player_volume);
        }
        return ESP_OK;
    }
    esp_audio_vol_get(handle, &player_volume);
    player_volume += 10;
    if (player_volume > 100) {
        player_volume = 100;
    }
    esp_audio_vol_set(handle, player_volume);
    ESP_LOGI(TAG, "%s, vol:[%d]", __func__, player_volume);
    // reply_control_vol("report", 1, player_volume, "5263234234");
    return ESP_OK;
}

esp_err_t player_action_vol_down(void *instance, action_arg_t *arg, action_result_t *result)
{
    int player_volume = 0;
    esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    esp_audio_vol_get(handle, &player_volume);
    player_volume -= 10;
    if (player_volume < 0) {
        player_volume = 0;
    }
    esp_audio_vol_set(handle, player_volume);
    ESP_LOGI(TAG, "%s, vol:[%d]", __func__, player_volume);
    // reply_control_vol("report", 1, player_volume, "5263234234");
    return ESP_OK;
}


int g_player_volume = 50;
esp_err_t player_action_mute_on(void *instance, action_arg_t *arg, action_result_t *result)
{
    ESP_LOGI(TAG, "%s", __func__);
    esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    int player_volume = 0;
    esp_audio_vol_get(handle, &player_volume);

    if(player_volume != 0)
    {
        g_player_volume = player_volume;
        esp_audio_vol_set(handle, 0);
        // reply_control_vol("report", 1, 0, "5263234234");
    }
    else
    {
        esp_audio_vol_set(handle, g_player_volume);
        // reply_control_vol("report", 1, g_player_volume, "5263234234");
    }    
    
    return ESP_OK;
}


esp_err_t player_action_mute_off(void *instance, action_arg_t *arg, action_result_t *result)
{
    // ESP_LOGI(TAG, "%s", __func__);
    // esp_audio_handle_t handle = (esp_audio_handle_t)instance;
    // if(vol_mute_flag == 0)
    // {
    //     esp_audio_vol_get(handle, &g_player_volume);
    //     esp_audio_vol_set(handle, 0);
    // }else{
    //     esp_audio_vol_set(handle, g_player_volume);
    // }
    
    return ESP_OK;
}
