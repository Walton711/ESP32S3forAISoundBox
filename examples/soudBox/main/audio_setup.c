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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <freertos/semphr.h>

#include "esp_log.h"
#include "esp_audio.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_mem.h"
#include "board.h"
#include "audio_common.h"
#include "audio_hal.h"
#include "filter_resample.h"
#include "fatfs_stream.h"
#include "raw_stream.h"
#include "i2s_stream.h"
#include "wav_decoder.h"
#include "wav_encoder.h"
#include "mp3_decoder.h"
#include "aac_decoder.h"
#include "http_stream.h"
#include "audio_idf_version.h"

#include "model_path.h"

#include "audio_recorder.h"
#include "recorder_sr.h"


#include "esp_decoder.h"
#include "amr_decoder.h"
#include "flac_decoder.h"
#include "ogg_decoder.h"
#include "opus_decoder.h"
#include "mp3_decoder.h"
#include "wav_decoder.h"
#include "aac_decoder.h"

static char *TAG = "AUDIO_SETUP";

#ifndef CODEC_ADC_SAMPLE_RATE
#warning "Please define CODEC_ADC_SAMPLE_RATE first, default value is 48kHz may not correctly"
#define CODEC_ADC_SAMPLE_RATE    16000
#endif

#ifndef CODEC_ADC_BITS_PER_SAMPLE
#warning "Please define CODEC_ADC_BITS_PER_SAMPLE first, default value 16 bits may not correctly"
#define CODEC_ADC_BITS_PER_SAMPLE  16
#endif

#ifndef CODEC_ADC_I2S_PORT
#define CODEC_ADC_I2S_PORT  (0)
#endif

static audio_element_handle_t raw_read;

static int _http_stream_event_handle(http_stream_event_msg_t *msg)
{
    if (msg->event_id == HTTP_STREAM_RESOLVE_ALL_TRACKS) {
        return ESP_OK;
    }

    if (msg->event_id == HTTP_STREAM_FINISH_TRACK) {
        return http_stream_next_track(msg->el);
    }
    if (msg->event_id == HTTP_STREAM_FINISH_PLAYLIST) {
        return http_stream_restart(msg->el);
    }
    return ESP_OK;
}
#include "audio_sys.h"
typedef enum {
    AUDIO_DATA_FORMNAT_ONLY_RIGHT1,
    AUDIO_DATA_FORMNAT_ONLY_LEFT1,
    AUDIO_DATA_FORMNAT_RIGHT_LEFT1,
} audio_channel_format_t111;
static esp_err_t audio_data_format_set111(i2s_stream_cfg_t *i2s_cfg, audio_channel_format_t111 fmt)
{
    AUDIO_UNUSED(i2s_cfg);   // remove unused warning
    switch (fmt) {
        case AUDIO_DATA_FORMNAT_ONLY_RIGHT1:
            i2s_cfg->std_cfg.slot_cfg.slot_mode = I2S_SLOT_MODE_MONO;
            i2s_cfg->std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;
            break;
        case AUDIO_DATA_FORMNAT_ONLY_LEFT1:
            i2s_cfg->std_cfg.slot_cfg.slot_mode = I2S_SLOT_MODE_MONO;
            i2s_cfg->std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
            break;
        case AUDIO_DATA_FORMNAT_RIGHT_LEFT1:
            i2s_cfg->std_cfg.slot_cfg.slot_mode = I2S_SLOT_MODE_STEREO;
            i2s_cfg->std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
            break;
    }
    return ESP_OK;
}
audio_element_handle_t el_raw_writer;
audio_element_handle_t i2s_h;
void *setup_player(void *cb, void *ctx)
{
    esp_audio_handle_t handle;
    esp_audio_cfg_t cfg = DEFAULT_ESP_AUDIO_CONFIG();
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    cfg.vol_handle = board_handle->audio_hal;
    cfg.vol_set = (audio_volume_set)audio_hal_set_volume;
    cfg.vol_get = (audio_volume_get)audio_hal_get_volume;
    cfg.prefer_type = ESP_AUDIO_PREFER_MEM;
    cfg.resample_rate = 16000;
    cfg.cb_func = cb;
    handle = esp_audio_create(&cfg);

    // Create readers and add to esp_audio
    fatfs_stream_cfg_t fs_reader = FATFS_STREAM_CFG_DEFAULT();
    fs_reader.type = AUDIO_STREAM_READER;
    // i2s_stream_cfg_t i2s_reader = I2S_STREAM_CFG_DEFAULT_WITH_PARA(I2S_NUM_0, 16000, I2S_DATA_BIT_WIDTH_16BIT, AUDIO_STREAM_READER);

    raw_stream_cfg_t raw_reader = RAW_STREAM_CFG_DEFAULT();
    raw_reader.type = AUDIO_STREAM_READER;

    // esp_audio_input_stream_add(ppplayer, raw_stream_init(&raw_reader));
    esp_audio_input_stream_add(handle, fatfs_stream_init(&fs_reader));
    // esp_audio_input_stream_add(ppplayer, i2s_stream_init(&i2s_reader));
    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    http_cfg.event_handle = _http_stream_event_handle;
    http_cfg.type = AUDIO_STREAM_READER;
    http_cfg.enable_playlist_parser = true;
    audio_element_handle_t http_stream_reader = http_stream_init(&http_cfg);
    esp_audio_input_stream_add(handle, http_stream_reader);

    // Add decoders and encoders to esp_audio
#ifdef ESP_AUDIO_AUTO_PLAY
    audio_decoder_t auto_decode[] = {
        DEFAULT_ESP_AMRNB_DECODER_CONFIG(),
        DEFAULT_ESP_AMRWB_DECODER_CONFIG(),
        DEFAULT_ESP_FLAC_DECODER_CONFIG(),
        DEFAULT_ESP_OGG_DECODER_CONFIG(),
        DEFAULT_ESP_OPUS_DECODER_CONFIG(),
        DEFAULT_ESP_MP3_DECODER_CONFIG(),
        DEFAULT_ESP_WAV_DECODER_CONFIG(),
        DEFAULT_ESP_AAC_DECODER_CONFIG(),
        DEFAULT_ESP_M4A_DECODER_CONFIG(),
        DEFAULT_ESP_TS_DECODER_CONFIG(),
    };
    esp_decoder_cfg_t auto_dec_cfg = DEFAULT_ESP_DECODER_CONFIG();
    esp_audio_codec_lib_add(ppplayer, AUDIO_CODEC_TYPE_DECODER, esp_decoder_init(&auto_dec_cfg, auto_decode, 10));
#else
    // amr_decoder_cfg_t  amr_dec_cfg  = DEFAULT_AMR_DECODER_CONFIG();
    // flac_decoder_cfg_t flac_dec_cfg = DEFAULT_FLAC_DECODER_CONFIG();
    // ogg_decoder_cfg_t  ogg_dec_cfg  = DEFAULT_OGG_DECODER_CONFIG();
    // opus_decoder_cfg_t opus_dec_cfg = DEFAULT_OPUS_DECODER_CONFIG();
    mp3_decoder_cfg_t  mp3_dec_cfg  = DEFAULT_MP3_DECODER_CONFIG();
    wav_decoder_cfg_t  wav_dec_cfg  = DEFAULT_WAV_DECODER_CONFIG();
    // aac_decoder_cfg_t  aac_dec_cfg  = DEFAULT_AAC_DECODER_CONFIG();

    // esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, amr_decoder_init(&amr_dec_cfg));
    // esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, flac_decoder_init(&flac_dec_cfg));
    // esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, ogg_decoder_init(&ogg_dec_cfg));
    // esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, decoder_opus_init(&opus_dec_cfg));
    esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, mp3_decoder_init(&mp3_dec_cfg));
    esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, wav_decoder_init(&wav_dec_cfg));
    // esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, aac_decoder_init(&aac_dec_cfg));
    // audio_element_handle_t m4a_dec_cfg = aac_decoder_init(&aac_dec_cfg);
    // audio_element_set_tag(m4a_dec_cfg, "m4a");
    // esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, m4a_dec_cfg);

    // audio_element_handle_t ts_dec_cfg = aac_decoder_init(&aac_dec_cfg);
    // audio_element_set_tag(ts_dec_cfg, "ts");
    // esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_DECODER, ts_dec_cfg);

    wav_encoder_cfg_t wav_enc_cfg = DEFAULT_WAV_ENCODER_CONFIG();
    esp_audio_codec_lib_add(handle, AUDIO_CODEC_TYPE_ENCODER, wav_encoder_init(&wav_enc_cfg));
#endif
    
    // Create writers and add to esp_audio
    fatfs_stream_cfg_t fs_writer = FATFS_STREAM_CFG_DEFAULT();
    fs_writer.type = AUDIO_STREAM_WRITER;

    i2s_stream_cfg_t i2s_writer = I2S_STREAM_CFG_DEFAULT_WITH_PARA(I2S_NUM_0, 16000, I2S_DATA_BIT_WIDTH_16BIT, AUDIO_STREAM_WRITER);
    
    raw_stream_cfg_t raw_writer = RAW_STREAM_CFG_DEFAULT();
    raw_writer.type = AUDIO_STREAM_WRITER;
    raw_writer.out_rb_size = 16*1024;
    i2s_h =  i2s_stream_init(&i2s_writer);
    esp_audio_output_stream_add(handle, i2s_h);
    esp_audio_output_stream_add(handle, fatfs_stream_init(&fs_writer));

    el_raw_writer = raw_stream_init(&raw_writer);
    ESP_LOGI(TAG, "esp_audio el_raw_writer is:%p\r\n", el_raw_writer);
    esp_audio_output_stream_add(handle, el_raw_writer);
    i2s_stream_set_clk(i2s_h, CODEC_ADC_SAMPLE_RATE, 16, 2);    
    // Set default volume
    esp_audio_vol_set(handle, 50);
    AUDIO_MEM_SHOW(TAG);
    ESP_LOGI(TAG, "esp_audio instance is:%p\r\n", handle);
    return handle;
}

static int input_cb_for_afe(int16_t *buffer, int buf_sz, void *user_ctx, TickType_t ticks)
{
    return raw_stream_read(raw_read, (char *)buffer, buf_sz);
}

void *setup_recorder(rec_event_cb_t cb, void *ctx)
{
    audio_element_handle_t i2s_stream_reader;
    audio_pipeline_handle_t pipeline;
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    if (NULL == pipeline) {
        return NULL;
    }
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_READER;
   
    audio_data_format_set111(&i2s_cfg, AUDIO_DATA_FORMNAT_ONLY_LEFT1); 
    i2s_cfg.std_cfg.clk_cfg.sample_rate_hz = 16000;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);
    i2s_cfg.out_rb_size = 16 * 1024; // Increase buffer to avoid missing data in bad network conditions
    i2s_stream_set_clk(i2s_stream_reader, CODEC_ADC_SAMPLE_RATE, 16, 1);

    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_READER;
    raw_cfg.out_rb_size = 4 * 1024;
    raw_read = raw_stream_init(&raw_cfg);

    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, raw_read, "raw");
    const char *link_tag[3] = {"i2s", "raw"};
    uint8_t linked_num = 2;
    

#if 0//(CODEC_ADC_SAMPLE_RATE != (16000))
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = CODEC_ADC_SAMPLE_RATE;
    rsp_cfg.dest_rate = 16000;
// #ifdef CONFIG_ESP32_S3_KORVO2_V3_BOARD
    rsp_cfg.mode = RESAMPLE_UNCROSS_MODE;
    rsp_cfg.src_ch = 1;
    rsp_cfg.dest_ch = 1;
    rsp_cfg.src_bits = 32;
    rsp_cfg.dest_bits = 16;
    rsp_cfg.max_indata_bytes = 1024*4;
// #endif
    audio_element_handle_t filter = rsp_filter_init(&rsp_cfg);
    audio_pipeline_register(pipeline, filter, "filter");
    link_tag[1] = "filter";
    link_tag[2] = "raw";
    linked_num = 3;
#endif

    audio_pipeline_link(pipeline, &link_tag[0], linked_num);
    audio_pipeline_run(pipeline);
    ESP_LOGI(TAG, "Recorder has been created");

    // recorder_sr_cfg_t recorder_sr_cfg = DEFAULT_RECORDER_SR_CFG();
    // recorder_sr_cfg.afe_cfg.aec_init = false;
    // recorder_sr_cfg.afe_cfg.memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;
    // recorder_sr_cfg.afe_cfg.agc_mode = AFE_MN_PEAK_NO_AGC;

    audio_rec_cfg_t cfg = AUDIO_RECORDER_DEFAULT_CFG();
    cfg.read = (recorder_data_read_t)&input_cb_for_afe;
    // cfg.sr_handle = recorder_sr_create(&recorder_sr_cfg, &cfg.sr_iface);
    cfg.event_cb = cb;
    cfg.vad_off = 1000;
    cfg.user_data = ctx;
    return audio_recorder_create(&cfg);
}
