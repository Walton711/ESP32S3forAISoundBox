idf_component_register(SRCS "i2s_recorder_main.c"
                    INCLUDE_DIRS "$ENV{IDF_PATH}/examples/peripherals/i2s/common")

[16:58:51.421]收←◆ets Jun  8 2016 00:22:57

rst:0x1 (POWERON_RESET),boot:0x1f (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1472
load:0x40078000,len:16132
ho 0 tail 12 room 4
load:0x40080400,len:4
load:0x40080404,len:3484
entry 0x4008060c

[16:58:51.785]收←◆[0;32mI (359) cpu_start: Multicore app[0m
[0;32mI (359) quad_psram: This chip is ESP32-D0WD[0m
[0;32mI (359) esp_psram: Found 8MB PSRAM device[0m
[0;32mI (361) esp_psram: Speed: 80MHz[0m
[0;32mI (365) esp_psram: PSRAM initialized, cache is in low/high (2-core) mode.[0m
[0;33mW (372) esp_psram: Virtual address not enough for PSRAM, map as much as we can. 4MB is mapped[0m
[0;32mI (381) cpu_start: Pro cpu up.[0m
[0;32mI (385) cpu_start: Starting app cpu, entry point is 0x40081610[0m
[0;32mI (0) cpu_start: App cpu up.[0m

[16:58:52.280]收←◆[0;32mI (856) esp_psram: SPI SRAM memory test OK[0m
[0;32mI (864) cpu_start: Pro cpu start user code[0m
[0;32mI (864) cpu_start: cpu freq: 240000000 Hz[0m
[0;32mI (864) cpu_start: Application information:[0m
[0;32mI (867) cpu_start: Project name:     esp_dueros[0m
[0;32mI (873) cpu_start: App version:      v2.6-76-gf1cd5b6a-dirty[0m
[0;32mI (879) cpu_start: Compile time:     Mar  6 2024 02:10:54[0m
[0;32mI (885) cpu_start: ELF file SHA256:  21fbe7afdd088b90...[0m
[0;32mI (891) cpu_start: ESP-IDF:          v5.1.3-dirty[0m
[0;32mI (896) cpu_start: Min chip rev:     v0.0[0m
[0;32mI (901) cpu_start: Max chip rev:     v3.99 [0m
[0;32mI (906) cpu_start: Chip rev:         v1.0[0m
[0;32mI (911) heap_init: Initializing. RAM available for dynamic allocation:[0m
[0;32mI (918) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM[0m
[0;32mI (924) heap_init: At 3FFB5F50 len 0002A0B0 (168 KiB): DRAM[0m
[0;32mI
[16:58:52.382]收←◆ (930) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM[0m
[0;32mI (936) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM[0m
[0;32mI (943) heap_init: At 400953E0 len 0000AC20 (43 KiB): IRAM[0m
[0;32mI (949) esp_psram: Adding pool of 4082K of PSRAM memory to heap allocator[0m
[0;32mI (957) spi_flash: detected chip: generic[0m
[0;32mI (961) spi_flash: flash io
[16:58:52.432]收←◆: qio[0m
[0;33mW (965) ADC: legacy driver is deprecated, please migrate to `esp_adc/adc_oneshot.h`[0m
[0;32mI (974) app_start: Starting scheduler on CPU0[0m
[0;32mI (979) app_start: Starting scheduler on CPU1[0m
[0;32mI (979) main_task: Started on CPU0[0m
[0;32mI (989) esp_psram: Reserving pool of 32K of internal memory for DMA/internal allocations[0m
[0;32mI (989
[16:58:52.458]收←◆) main_task: Calling app_main()[0m
[0;32mI (1019) DUEROS: ADF version is v2.6-76-gf1cd5b6a-dirty[0m

[16:58:53.447]收←◆[0;33mW (2019) SPI: MCS ret:0,Status:1[0m
[0;32mI (2019) gpio: GPIO[22]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 [0m
[0;32mI (2019) gpio: GPIO[21]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 [0m
[0;32mI (2029) AUDIO_THREAD: The button_task task allocate stack on internal memory[0m
[0;32mI (2039) AUDIO_THREAD: The esp_periph task allocate stack on internal memory[0m
[0;32mI (2049) SDCARD: Using 1-line SD mode,  base path=/sdcard[0m

[16:58:53.519]收←◆[0;32mI (2099) SDCARD: CID name SD32G!
[0m
[0;32mI (2099) D
[16:58:53.549]收←◆UEROS: 1111Periph Event received: src_type:100003, source:0x3f8043a0 cmd:2, data:0x0, data_len:0[0m

[16:58:53.965]收←◆[0;31mE (2539) DISPATCHER: exe first list: 0x0[0m
[0;32mI (2539) DISPATCHER: dispatcher_event_task is running...[0m
[0;32mI (2539) AUDIO_THREAD: The esp_dispatcher task allocate stack on internal memory[0m
[0;32mI (2549) AUDIO_THREAD: The wifi_serv task allocate stack on external memory[0m
I (2559) wifi:wif
[16:58:54.014]收←◆i driver task: 3ffca8d4, prio:23, stack:6656, core=0
I (2559) wifi:wifi firmware version: 0016c4d
I (2559) wifi:wifi certification version: v7.0
I (2559) wifi:config NVS flash: enabled
I (2569) wifi:config nano formating: disabled
I (2569) wifi:Init data frame dynamic rx buffer num: 32
I (2579) wifi:Init static rx mgmt buffer num: 5
I (2579) wifi:Init management short buffer num: 32
I (2589) wifi:Init static tx buffer num: 16
I (2589) wifi:Init tx cache buffer num: 32
I (2589) wifi:Init static rx
[16:58:54.056]收←◆buffer size: 1600
I (2599) wifi:Init static rx buffer num: 16
I (2599) wifi:Init dynamic rx buffer num: 32
[0;32mI (2609) wifi_init: rx ba win: 16[0m
[0;32mI (2609) wifi_init: tcpip mbox: 32[0m
[0;32mI (2609) wifi_init: udp mbox: 6[0m
[0;32mI (2619) wifi_init: tcp mbox: 6[0m
[0;32mI (2619) wifi_init: tcp tx win: 5744[0m
[0;32mI (2629) wifi_init: tcp rx win: 5744[0m
[0;32mI (2629) wifi_init: tcp mss: 1440[0m
[0;32mI (2629) wifi_init: WiFi/LWIP prefer SPIRAM[0m
[0;32mI (2639) phy_init: phy_version 4791,2c4672b,Dec 20 2023,16:06:06[0m

[16:58:54.136]收←◆I (2709) wifi:mode : sta (c4:4f:33:17:24:05)
I (2709) wifi:enable tsf

[16:59:03.977]收←◆[0;33mW (12549) AUDIO_BOARD: The board has already been initialized![0m
[0;32mI (12549) AUDIO_THREAD: The media_task task allocate stack on external memory[0m
[0;32mI (12549) ESP_AUDIO_TASK: media_ctrl_task running...,0x3f81b014[0m
[0;32mI (12559) WIFI_SERV: Connect to wifi ssid: Walton_, pwd: 1234567890[0m

----------------------------- ESP Audio Platform -----------------------------
|                                                                            |
|                 ESP_AUDIO-v1.7.3-12-g521f7fc-57d65d5-0303fc9               |
|                     Compile date: Jan 22 2024-02:26:14                     |
------------------------------------------------------------------------------
[0;32mI (12599) ESP_AUDIO_CTRL: Func:media_ctrl_create, Line:355, MEM Total:4238132 Bytes, Inter:234399 Bytes, Dram:191163 Bytes
[0m
[0;32mI (12609) AUDIO_HAL: Codec mode is 3, Ctrl:1[0m
[0;32mI (12619) AUDIO_WRAPPER: +++++++++++1111111++++++++++++
[0m
[0;32mI (12619) AUDIO_WRAPPER: +++++++++++22222++++++++++++
[0m
[0;31mE (12629) I2S_STREAM_IDF5.x: Invalid ch: 16[0m
[0;31mE (12629) I2S_STREAM_IDF5.x: i2s_set_clk failed1[0m
[0;32mI (12639) AUDIO_WRAPPER: +++++++++++33333++++++++++++
[0m
[0;32mI (12649) MP3_DECODER: MP3 ini
[16:59:04.108]收←◆t[0m
[0;32mI (12649) AUDIO_WRAPPER: +++++++++++444444++++++++++++
[0m
[0;33mW (12659) I2S_STREAM_IDF5.x: I2S(2) already startup[0m
[0;32mI (12659) AUDIO_WRAPPER: +++++++++++55555555++++++++++++
[0m
[0;32mI (12669) AUDIO_WRAPPER: Func:duer_audio_setup_player, Line:185, MEM Total:4224664 Bytes, Inter:221883 Bytes, Dram:178647 Bytes
[0m
[0;32mI (12679) AUDIO_WRAPPER: esp_audio instance is:0x3f81b014[0m
[0;32mI (12689) AUDIO_PIPELINE: link el->rb, el:0x3f81d640, tag:i2s, rb:0x3f81db08[0m
[16:59:04.149]收←◆
[0;32mI (12689) AUDIO_PIPELINE: link el->rb, el:0x3f81d9c0, tag:filter, rb:0x3f81fb50[0m
[0;32mI (12699) AUDIO_THREAD: The i2s task allocate stack on internal memory[0m
[0;32mI (12709) AUDIO_ELEMENT: [i2s-0x3f81d640] Element task created[0m
[0;32mI (12709) AUDIO_ELEMENT: [raw-0x3f81d824] Element task created[0m
[0;32mI (12719) AUDIO_THREAD: The filter task allocate st
[16:59:04.179]收←◆ack on external memory[0m
[0;32mI (12729) AUDIO_ELEMENT: [filter-0x3f81d9c0] Element task created[0m
[0;32mI (12739) AUDIO_PIPELINE: Func:audio_pipeline_run, Line:359, MEM Total:4198148 Bytes, Inter:211175 Bytes, Dram:167939 Bytes
[0m
[0;32mI (12749) AUDIO_ELEMENT: [i2s] AEL_MSG_CMD_RESUME,state:1[0m
[0;32mI (12749) AUDIO_ELEMENT: [filter] AEL_MSG_CMD_RESUME,state:1[0m

[16:59:04.300]收←◆[0;32mI (12869) RSP_FILTER: sample rate of source data : 44100,
[16:59:04.320]收←◆ channel of source data : 2, sample rate of destination data : 16000, channel of destination data : 2[0m
[0;32mI (12879) AUDIO_PIPELINE: Pipeline started[0m
[0;32mI (12879) AUDIO_WRAPPER: Recorder has been created[0m
[0;32mI (12889) AUDIO_THREAD: The recorder_task task allocate stack on external memory[0m
[0;32mI (12889) AUDIO_THREAD: The voice_read_task task allocate s
[16:59:04.362]收←◆tack on external memory[0m
[0;32mI (12899) main_task: Returned from app_main()[0m

[16:59:05.348]收←◆I (13919) wifi:new:<12,0>, old:<1,0>, ap:<255,255>, sta:<12,0>,
[16:59:05.367]收←◆prof:1

[16:59:05.524]收←◆I (14089) wifi:state: init -> auth (b0)
I (14099) wifi:state: auth -> assoc (0)
I (14099) wifi:state: assoc -> run (10)
I (14139) wifi:connected with Walton_, aid = 1, channel 12, BW20, bssid = b2:10:c9:1a:04:91
I (14139) wifi:security: WPA2-PSK, phy: bgn, rssi: -50
I (14139) wifi:pm start, type: 1

I (14139) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
[0;33mW (14149) WIFI_SERV: WiFi Event cb, Unhandle event_base:WIFI_EVENT, event_id:4[0m

[16:59:05.658]收←◆I (14229) wifi:AP's beacon interval = 102400 us, DTIM period = 1

[16:59:06.574]收←◆[0;32mI (15149) esp_netif_handlers: sta ip: 172.20.10.3, mask: 255.255.255.240, gw: 172.20.10.1[0m
[0;32mI (15149) WIFI_SERV: Got ip:172.20.10.3[0m
[0;33mW (15149) WIFI_SERV: STATE type:2, pdata:0x0, len:0[0m

[16:59:06.660]收←◆[0;32mI (15229) DUEROS: event type:2,source:0x3f80af48, data:0x
[16:59:06.679]收←◆0,len:0,ctx:0x0[0m
[0;32mI (15229) DUEROS: PERIPH_WIFI_CONNECTED [229][0m

[16:59:19.686]收←◆[0;32mI (28259) DUEROS: 1111Periph Event received: src_type:10000c, source:0x3f8042b4 cmd:1, data:0x2, data_len:3[0m
[0;32mI (28259) DUEROS: ++++++++++++++++++++++
[0m
[0;32mI (28259) DUEROS: rec_engine_cb - AUDIO_REC_VAD_START[0m
++++++++rec+++++++++
[0;32mI (28269) AUDIO_RECORDER: RECORDER_CMD_TRIGGER_START[0m
333333333
55555555555
[0;32mI (28269) DUEROS: rec_engine_cb - AUDIO_REC_VAD_START[0m
ret = 2048,start rec2 = 0........
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). E
[16:59:19.746]收←◆xception was unhandled.

Core  1 register dump:
PC      : 0x4008d149  PS      : 0x00060c30  A0      : 0x8016670c  A1      : 0x3f823f50
A2      : 0x00003e80  A3      : 0x00003e7c  A4      : 0x000000ff  A5      : 0x0000ff00
A6      : 0x00ff0000  A7      : 0xff000000  A8      : 0x00000000  A9      : 0x3ffb7e68
A10     : 0x00000000  A11     : 0x00000015  A12     : 0x0000000c  A13     : 0x3ffb7e50
A14     : 0x00001800  A15     : 0x00000000  SAR     : 0x00000013  EXCCAUSE: 0x0000001c
EXCVADDR: 0x00003e80  LBEG    : 0x4008d149  LEND    : 0x4008d159  LCOUNT  :
[16:59:19.794]收←◆ 0xffffffff


Backtrace: 0x4008d146:0x3f823f50 0x40166709:0x3f823f60 0x40166543:0x3f823f80 0x4016486a:0x3f823fb0 0x400d8eda:0x3f823fd0 0x400d904a:0x3f8240a0




ELF file SHA256: 21fbe7afdd088b90

Rebooting...
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x1f (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1472
load:0x40078000,len:16132
ho 0 tail 12 room 4
load:0x40080400,len:4
load:0x40080404,len:3484
entry 0x4008060c

[16:59:20.151]收←◆[0;32mI (358) cpu_start: Multicore app[0m
[0;32mI (358) quad_psram: This chip is ESP32-D0WD[0m
[0;32mI (359) esp_psram: Found 8MB PSRAM device[0m
[0;32mI (360) esp_psram: Speed: 80MHz[0m
[0;32mI (364) esp_psram: PSRAM initialized, cache is in low/high (2-core) mode.[0m
[0;33mW (372) esp_psram: Virtual address not enough for PSRAM, map as much as we can. 4MB is mapped[0m
[0;32mI (381) cpu_start: Pro cpu up.[0m
[0;32mI (385) cpu_start: Starting app cpu, entry point is 0x40081610[0m
[0;32mI (376) cpu_start: App cpu up.[0m

[16:59:20.657]收←◆[0;32mI (855) esp_psram: SPI SRAM memory test OK[0m
[0;32mI (864) cpu_start: Pro cpu start user code[0m
[0;32mI (864) cpu_start: cpu freq: 240000000 Hz[0m
[0;32mI (864) cpu_start: Application information:[0m
[0;32mI (867) cpu_start: Project name:     esp_dueros[0m
[0;32mI (872) cpu_start: App version:      v2.6-76-gf1cd5b6a-dirty[0m
[0;32mI (878) cpu_sta
[16:59:20.710]收←◆rt: Compile time:     Mar  6 2024 02:10:54[0m
[0;32mI (884) cpu_start: ELF file SHA256:  21fbe7afdd088b90...[0m
[0;32mI (890) cpu_start: ESP-IDF:          v5.1.3-dirty[0m
[0;32mI (896) cpu_start: Min chip rev:     v0.0[0m
[0;32mI (900) cpu_start: Max chip rev:     v3.99 [0m
[0;32mI (905) cpu_start: Chip rev:         v1.0[0m
[0;32mI (910) heap_init: Initializing. RAM available for dynamic allocation:[0m
[0;32mI (917) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM[0m
[0;32mI (923) h
[16:59:20.760]收←◆eap_init: At 3FFB5F50 len 0002A0B0 (168 KiB): DRAM[0m
[0;32mI (930) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM[0m
[0;32mI (936) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM[0m
[0;32mI (942) heap_init: At 400953E0 len 0000AC20 (43 KiB): IRAM[0m
[0;32mI (949) esp_psram: Adding pool of 4082K of PSRAM memory to heap allocator[0m
[0;32mI (957) spi_flash
