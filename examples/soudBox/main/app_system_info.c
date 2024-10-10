#include "app_system_info.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "audio_thread.h"
#include "app_task.h"
#include "player_action.h"
#include "esp_action_exe_type.h"
#include <sys/time.h>
#include "esp_netif_sntp.h"
#include "driver/uart.h"
#include "esp_dispatcher_dueros_app.h"
#include "audio_def.h"
static const char               *TAG                = "SYSTEMTASK";


extern esp_dispatcher_handle_t pp_dispatcher;

/*****************************clock******************************************/
#include<math.h>
 QueueHandle_t clock_queue = NULL;

int system_clock_use(int id, clock_list_t clockList)
{
    clock_msg_t event = { 0 };
    event.clockList = clockList;

    if(0)
    {
        if (xQueueSendFromISR(clock_queue, &event, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    } else {
        if (xQueueSend(clock_queue, &event, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "PLAYER event send failed");
            return ESP_FAIL;
        }
    }
   return  0;
}


time_t set_timeT(int year, int mon, int day, int hour, int min, int sec)
{
    struct tm timeinfo;
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = mon - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = min;
    timeinfo.tm_sec = sec;
    time_t now = mktime(&timeinfo); //tm转time_t
    return now;
}

// time_t mktime(struct tm *tm); //将struct tm 结构的时间转换为从1970年至今的秒数  
void get_current_time(void)
{
    time_t now;
    struct tm timeinfo;
    struct timeval tv_now;
    char timeBuf[30];
    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now); //格林尼治时间1970年1月1日00:00:00到当前时刻的时长，时长单位是秒。
    localtime_r(&now, &timeinfo);
    // printf("get_current_time = %lld\n", now);
    // printf("year = %d %d %d\n", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    // printf("time = %d %d %d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    snprintf(timeBuf, 30, "%04d/%02d/%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    // printf("day = %s\n", timeBuf);
}

long long int str2int(char *buf)
{
    long long int timeStamp=0;
    int len = strlen(buf);
    printf("len = %d\n", len);
    for(int i=0;i<len;i++)
    {
        timeStamp += (buf[i] - '0') * (long long int)pow(10, (len - i - 1));
        // printf("timeStamp = %lld, %d,i=%f\n",timeStamp, (buf[i] - '0'), pow(10, (len - i)));
    }
    printf("timeStamp = %lld\n", timeStamp);
    return timeStamp;
}

void http_setTime_init(char *time,int len) 
{
    char *pEnd;
    char timeTemp[20]={0};
    memcpy(timeTemp,time,len);
    printf("timeTemp = %s\n", timeTemp);
    long long int li1 = str2int(timeTemp);
    // li1 = li1 / 1000;
    // long int li1 = strtol(timeTemp, &pEnd, 10);
    printf("li1 = %lld\n", li1);
    time_t timeSinceEpoch = li1;//.253; //
    struct timeval now = { .tv_sec = timeSinceEpoch, .tv_usec = 0 };
    struct timezone tz = {
        8, 0
    };
    setenv("TZ", "CST-8", 1);
    tzset();
    settimeofday(&now, &tz);
    printf("set time success\n");
    get_current_time();

}

void pp_setSntp_time(void)
{
    struct timeval tv = {
        .tv_sec = 1509449941,
    };
    struct timezone tz = {
        0, 0
    };
    settimeofday(&tv, &tz);

    /* Start SNTP service */
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    // esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("210.72.145.44");
    esp_netif_sntp_init(&config);
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
        printf("Failed to update system time, continuing");
    }
    esp_netif_sntp_deinit();

    time_t now;
    struct tm timeinfo;
    time(&now);
    printf("get now = %lld\n", now);
    localtime_r(&now, &timeinfo);
    gettimeofday(&tv, &tz);
    printf("year = %d %d %d\n", timeinfo.tm_year + 1970, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    printf("time = %d %d %d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

#define MAX_CLOCK_NUM 3
clock_list_t g_clock_lists[MAX_CLOCK_NUM];
uint8_t g_clock_num = 0;
void clock_system_insertDB(clock_list_t clockInfo)
{
    if(g_clock_num == MAX_CLOCK_NUM)
    {
        return ;
    }
    printf("start\n");
    #if 1
    for(int i=0; i<MAX_CLOCK_NUM; i++)
    {
        if(g_clock_lists[i].nowClock ==  0)
        {
            printf("i = %d,action = %d,len = %d\n",i,clockInfo.clock_action, sizeof(clockInfo));
            g_clock_lists[i] = clockInfo;
            g_clock_num ++;
            break;
        }
    }
    printf("start222\n");
    for(int i=0;i<MAX_CLOCK_NUM;i++) {  
        if(g_clock_lists[i].nowClock == 0)
        {
            break;
        }
        printf("i = %d, nowClock = %lld\n",i, g_clock_lists[i].nowClock);
    }
    
    printf("start333\n");
    if(g_clock_num > MAX_CLOCK_NUM) //闹钟已满
    {
        g_clock_num = MAX_CLOCK_NUM;
        printf("clock full \n");
    }

    #endif
}

void dayTimeParase(char *dayBuffer,int *dayTime)
{
    char *temp = strtok(dayBuffer,"/");
    int count = 0;
    char *endptr;
    while(temp)
    {
        dayTime[count] = atoi(temp) ;
        printf("%s,%d\n",temp , dayTime[count]);
        temp = strtok(NULL, "/");
        count ++ ;
        if(count >= 5)
        {
            break;
        }
    }
}

void clock_system_insert(uint8_t action, char *clock_info, char *name)
{
    clock_list_t clock;
    clock.clock_action = action;
    memcpy(clock.clock_name, name, 16);
    memcpy(clock.clock_time, clock_info, 16);
    time_t clockStamp;
    int daytime[6] ={0};
    dayTimeParase(clock_info, daytime);
    printf("%d,%d,%d,%d,%d,%d\n", daytime[0],daytime[1], daytime[2], daytime[3],daytime[4],daytime[5]);
    clockStamp = set_timeT(daytime[0],daytime[1],daytime[2],daytime[3],daytime[4],daytime[5]);
    printf("insert clock:%lld\n", clockStamp);
    clock.nowClock = clockStamp;
    system_clock_use(SYSTEM_CLOCK_INSTER,clock);
}



void clock_system_show()
{
    for(int i=0;i<MAX_CLOCK_NUM;i++)
    {
        printf("i=%d, %s\n", i, g_clock_lists[i].clock_time);
    }
}

void clock_system_deleteById(int id)
{
    memset(&g_clock_lists[id], 0, sizeof(g_clock_lists[id]));
}


uint32_t g_rec_count = 0;
int spk_count = 0;




int out_flag = 0;
static void clock_timer_cb(xTimerHandle tmr)
{
    // ESP_LOGE(TAG, "Func:%s", __func__);
    // get_current_time();
    time_t now;
    struct tm timeinfo;
    struct tm saveTimeinfo;
    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now); //格林尼治时间1970年1月1日00:00:00到当前时刻的时长，时长单位是秒。
    localtime_r(&now, &timeinfo);

    for(int i = 0; i<g_clock_num; i++)
    {
        if(g_clock_lists[i].nowClock == 0)
        {
            continue;
        }
        memset(&saveTimeinfo, 0, sizeof(saveTimeinfo));
        localtime_r(&g_clock_lists[i].nowClock, &saveTimeinfo);
        switch (g_clock_lists[i].clock_action)
        {
            case 1:
                printf("single now = %lld, setClock = %lld\n", now, g_clock_lists[i].nowClock);
                if(((now - g_clock_lists[i].nowClock) == 0) /*&& ((now - g_clock_lists[i].nowClock) >=0)*/)
                {
                    if(out_flag == 0){
                        // esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
                        printf("111single clock is comming............\n");
                        pp_music_play("file://spiffs/clock/1.MP3", 5, 1);
                        clock_system_deleteById(i);
                        out_flag = 1;
                    }
                }
                else
                {
                    out_flag = 0;   
                }
            break;
            case 2:
                printf("daily clock\n");
                printf("time = %d %d %d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec); // 对比这个时间
                if((timeinfo.tm_hour == saveTimeinfo.tm_hour) && (timeinfo.tm_min==saveTimeinfo.tm_min) && (timeinfo.tm_sec==saveTimeinfo.tm_sec))
                {
                    printf("222daily clock is comming............\n");
                    pp_music_play("file://spiffs/clock/1.MP3", 5, 1);
                }
            break;
            case 3:
                printf("weekly clock\n");
                printf("week = %d\n", timeinfo.tm_wday); // 对比这个星期几
                if((timeinfo.tm_wday == saveTimeinfo.tm_wday))
                {
                    if((timeinfo.tm_hour==saveTimeinfo.tm_hour) && (timeinfo.tm_min==saveTimeinfo.tm_min) && (timeinfo.tm_sec==saveTimeinfo.tm_sec))
                    {
                        printf("333daily clock is comming............\n");
                        pp_music_play("file://spiffs/clock/1.MP3", 5, 1);
                    }
                }
            break;
            case 4:
                printf("weekend clock\n");
                printf("week = %d\n", timeinfo.tm_wday); // 对比是否为星球六和星期日
                if((timeinfo.tm_wday == 5)||(timeinfo.tm_wday == 6))
                {
                    if((timeinfo.tm_hour==saveTimeinfo.tm_hour) && (timeinfo.tm_min==saveTimeinfo.tm_min) && (timeinfo.tm_sec==saveTimeinfo.tm_sec))
                    {
                        printf("444weekend clock is comming............\n");
                        pp_music_play("file://spiffs/clock/1.MP3", 5, 1);
                    }
                }
            break;
            case 5:
                printf("workday clock\n");
                printf("week = %d\n", timeinfo.tm_wday); // 对比是否为星球1-5

                if((timeinfo.tm_wday >= 0) || (timeinfo.tm_wday <= 4))
                {
                    if((timeinfo.tm_hour == saveTimeinfo.tm_hour) && (timeinfo.tm_min == saveTimeinfo.tm_min) && (timeinfo.tm_sec==saveTimeinfo.tm_sec))
                    {
                        printf("555workday clock is comming............\n");
                        pp_music_play("file://spiffs/clock/1.MP3", 5, 1);
                    }
                }
            break;
        }
    }
    
    g_rec_count ++;
    spk_count ++;
    // printf("[clock_timer_cb]g_rec_count=%ld\n", g_rec_count);
}
void system_clock_task(void *pvParameters) 
{
    clock_msg_t event;

    while(1) {
        if (xQueueReceive(clock_queue, &event, portMAX_DELAY)) {
            if(event.id == SYSTEM_CLOCK_INSTER)
            {
                clock_system_insertDB(event.clockList);
            }
        }
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}

void pp_clock_init(void)
{
    xTimerHandle retry_login_timer;
    retry_login_timer = xTimerCreate("tm_duer_login", 100 / portTICK_PERIOD_MS, 
                                    pdTRUE, NULL, clock_timer_cb);
    if(retry_login_timer == NULL) {
        //error
        printf("Error xTimerCreate\n");
    } else {
        xTimerStart(retry_login_timer,0); 
        printf("clock init success!!!\n");
    }

    clock_queue = xQueueCreate(2, sizeof(clock_msg_t));
    xTaskCreate(system_clock_task, "clock_timer_cb", 5*1024, NULL, 6, NULL);
}


/*********************************************************************************************/




/***********************************uart*******************************************/
extern esp_dispatcher_dueros_speaker_t *dueros_speaker;

extern esp_audio_status_t  g_playFlag;
extern int g_rec_status;
extern int g_music_playlist;
void wakeup_rec_start(int n,int inter)
{
    esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_AUDIO_STOP, NULL, NULL);
    // pp_file_cmd(1,inter);
    pp_stop_musicList_play();
    pp_stop_networkmusicList_play();
    if(n == 1)
    {
        wakeup_tone(inter);
    }else{

    }
    // while(g_playFlag != AUDIO_STATUS_FINISHED);
    vTaskDelay(pdMS_TO_TICKS(1500));
    led_blink_onoff(1);
    g_rec_count = 0;
    spk_count = 0;
    
    esp_dispatcher_execute(pp_dispatcher, ACTION_EXE_TYPE_REC_WAV_TURN_ON, NULL, NULL);
}


void wakeup_parse(uint8_t *data,int len)
{
   printf("data = ");
    for(int i=0;i<len;i++)
    {
        printf("%x ",data[i]);
    }
    printf("\n");

    // 唤醒词：    7E 06 FF 06 01 01 0D EF
    if(data[5] == 0x01)
    {
        printf("wakeup.....\n");
        g_rec_status = 0;
        g_music_playlist = -1;
        wakeup_rec_start(1,0);
    }
    else if(data[5] == 0x1D) //陪我聊天 7E 06 FF 06 01 1D 29 EF
    {
        printf("陪我聊天。。。。\n");
        g_rec_status = 2;
    }
    else if(data[5] == 0x0A) //再见 退出 关闭音箱 7E 06 FF 06 01 0A 16 EF
    {
        g_rec_status = 0;
    }
    else
    {
        return ;
    }
    // 7e 6 ff 6 1 ff b ef
    
    // 结束：      7e 6 ff 6 1 ff b ef
    // 学习唤醒词： 7e 6 ff 6 1 6d 79 ef

    //命令词：
}

#define UART_NUM UART_NUM_1
#define BUF_SIZE (128) 
QueueHandle_t spp_uart_queue = NULL;
void uart_task(void *pvParameters) 
{
    // uint8_t *data = (uint8_t *)malloc(BUF_SIZE); 
    uint8_t data[BUF_SIZE+10] = {0};
    uart_event_t event;
    // int ret = uart_write_bytes(UART_NUM,"abcd",6);
    while (1)
    {
        #if 1
        if (xQueueReceive(spp_uart_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            // printf("event.type = %d\n", event.type);
            switch (event.type) {
                //Event of UART receving data
                case UART_DATA:
                        int length = 0;
                        // uart_flush_input(UART_NUM);
                        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t*)&length));
                        memset(data, 0, BUF_SIZE);
                        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, 1000/portMAX_DELAY);
                        wakeup_parse(data,len);
                        // free(data);
                        break;
                case UART_BUFFER_FULL:break;
                case UART_FIFO_OVF:break;
                case UART_FRAME_ERR:break;
                case UART_PARITY_ERR:break;
                case UART_DATA_BREAK:break;
                case UART_PATTERN_DET:break;
                case UART_EVENT_MAX:break;
                case UART_WAKEUP:break;
                case UART_BREAK:break;
            }
        }
        #endif
        vTaskDelay(10);
    } 
    free(data); 
    vTaskDelete(NULL); 
} 


#include "driver/gpio.h"
void rec_wakeup() 
{
    gpio_set_direction(7, GPIO_MODE_INPUT);
    gpio_set_direction(15, GPIO_MODE_OUTPUT);
     uart_config_t uart_config = {
        .baud_rate = 9600, 
        .data_bits = UART_DATA_8_BITS, 
        .parity = UART_PARITY_DISABLE, 
        .stop_bits = UART_STOP_BITS_1, 
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    }; 
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, 15, 7, NULL, NULL));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 5,  &spp_uart_queue, ESP_INTR_FLAG_IRAM)); 
    
    xTaskCreate(uart_task, "uart_task", 3*1024, NULL, 0, NULL); 
}

/***********************************************************************************/



/*iic*/

#include "driver/i2c.h"
#define I2C_MASTER_NUM              1                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000
#define MPU9250_SENSOR_ADDR         0x21

/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
static esp_err_t mpu9250_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    int ret = i2c_master_write_read_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    printf("ret = %d, data = %x\n",ret, data[0]);
    return 0;
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t mpu9250_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}


void set_breathing_light(uint8_t B,uint8_t G,uint8_t R,uint8_t times)
{
//	led_close();
	//1.white led
	mpu9250_register_write_byte(0x88,B); //B
	mpu9250_register_write_byte(0x89,G); //G
	mpu9250_register_write_byte(0x8A,R); //R

	//2.set brigthness
	mpu9250_register_write_byte(0x86,0xff);
	mpu9250_register_write_byte(0x87,0x00);

	//3.set group GEn=1/PATEN=1/GSLDIS=0
	mpu9250_register_write_byte(0x8b,0x0f);

	//4.set pattern time T1/T2/T3/T4
	mpu9250_register_write_byte(0x82,0x30|0x03);
	mpu9250_register_write_byte(0x83,0x30|0x03);

	//5.set pattern0_breath start/end phase
	mpu9250_register_write_byte(0x84,0x00);

	//6.set pattern0_breath loop times -> forever
	mpu9250_register_write_byte(0x85,times);

	//7.set auto_breath mode 
	mpu9250_register_write_byte(0x80,0x03);

	//8.start breath
	mpu9250_register_write_byte(0x81,0x01);
}

void aw210xx_update_brightness_to_display()
{
	mpu9250_register_write_byte(0x45,0x00);

}

void aw210xx_mode_set(uint8_t B,uint8_t G,uint8_t R)
{
	mpu9250_register_write_byte(0x88,B); //B
	mpu9250_register_write_byte(0x89,G); //G
	mpu9250_register_write_byte(0x8A,R); //R
	
	mpu9250_register_write_byte(0x8B,0x0f);

    // mpu9250_register_write_byte(0x80,0x00);
	mpu9250_register_write_byte(0x87,0xff); //max brightness
}

void set_all_led(uint8_t B,uint8_t G,uint8_t R)
{
	aw210xx_mode_set(B,G,R);
	aw210xx_update_brightness_to_display();
}



/**
 * @brief i2c led initialization
 */
esp_err_t i2c_led_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    gpio_set_direction(3, GPIO_MODE_OUTPUT);
    gpio_set_direction(17, GPIO_MODE_OUTPUT);
    gpio_set_direction(18, GPIO_MODE_OUTPUT);
    gpio_set_level(3, 1);

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 17,
        .scl_io_num = 18,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);
    esp_err_t err = i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, ESP_INTR_FLAG_IRAM);
    printf("err = %d\n", err);

    mpu9250_register_write_byte(0x70,00);
    vTaskDelay(pdMS_TO_TICKS(30));    
	uint8_t chipid = 0xff;
	uint8_t status = 0;
	status = mpu9250_register_read(0x70, &chipid,1);
		
	printf("++++++++++++++++++aw210xx chipid = %x, status = %d+++++++++++++++++++++\n",chipid, status);

    mpu9250_register_write_byte(0x20,0x01); //en chipen   

    
    int br = 0x0;
    
    mpu9250_register_write_byte(0x58,br); //µçÁ÷

    status = mpu9250_register_read(0x58, &chipid,1);
    printf("++++++++++++++++++aw210xx chipid = %x, status = %d+++++++++++++++++++++\n",chipid, status);
    vTaskDelay(pdMS_TO_TICKS(1000));
    set_all_led(br, br, br);
          
    

    return ESP_OK;
}








