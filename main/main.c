#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_pm.h"

#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "soc/soc.h"
#include "driver/rtc_io.h"
#include "driver/rtc_cntl.h"
#include "esp32/ulp.h"
#include "sdkconfig.h"
#include "time.h"
#include <sys/time.h>

#include "deep_sleep.h"
#include "ulp_main.h"
#include "ulp_wake.h"


static int counting(gpio_evt_msg *message);
static void gpio_task();
static void send_task();
static void ulp_isr(void *arg);

static RTC_NOINIT_ATTR int count;

static const char *TAG = "main";
static SemaphoreHandle_t sema = NULL;

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

static void gpio_task()
{
    gpio_evt_msg msg;
    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &msg, portMAX_DELAY))
        {

            printf("event time: %lld pin: %d level %d\n", msg.timestamp_10us, msg.rtc_gpio_num, msg.level);
            counting(&msg);
        }
    }
}

/** following is a sample application 
 * the two pins are connected to two sensor of a light barrier doorway tunstile
 * when the light barrier is triggered it will output high signal
 * we could count the number of people in the room relatively 
*/
static int counting(gpio_evt_msg *message)
{
    
    static int8_t transition_table[][4] = {
        //event: pin1-fall pin1-rise pin2-fall pin2-rise
        {-1, 1, -1, 2},  //state 0: neither high
        {0, -1, -1, 3},  //state 1: pin1 high
        {-1, 3, 0, -1},  //state 2: pin2 high
        {2, -1, 1, -1}}; //state 3: both high
    static int8_t action_table[][2] = {
        //fall rise
        {0, 1},  //pin1
        {2, 3}}; //pin2

    static RTC_DATA_ATTR int state = 0;
    static RTC_DATA_ATTR uint32_t history = 0;
    static int64_t last_time = 0;
    static const float expire_th_s = 1;
    if ((state == 0) && ((message->timestamp_10us - last_time) > (expire_th_s * S_TO_US_FACTOR / 10)))
    {
        printf("old state expires, clear history\n");
        history = 0;
    }
    last_time = message->timestamp_10us;

    int pin = (message->rtc_gpio_num == rtc_io_number_get(RTC_PIN_1)) ? 0 : 1;
    int action = action_table[pin][message->level];
    state = transition_table[state][action];
    if (state < 0)
    {
        printf("impossible action, debouncing has failed\n");
        state = 0;
        return -1;
    }
    history *= 10;
    history += state;
    history %= 1000;
    printf("history: %d\n", history);
    switch (history)
    {
    case 102:
    case 132:
        count++;
        printf("count: %d\n", count);
        break;
    case 201:
    case 231:
        count--;
        printf("count: %d\n", count);
        break;
    default:
        break;
    }
    return 0;
}


static void ulp_isr(void *arg)
{
    portBASE_TYPE pxHigherPriorityTaskWoken = 0;
    xSemaphoreGiveFromISR(sema, &pxHigherPriorityTaskWoken);
}

static void send_task()
{
    while (1)
    {
        if (xSemaphoreTake(sema, portMAX_DELAY))
        {
            restart_sleep_timer();
            send_gpio_evt();
        }
    }
}

void app_main(void)
{
    if (esp_reset_reason() == ESP_RST_POWERON)
    {
        printf("reset count to zero\n");
        count = 0;
        init_ulp_program();
    }
    sema = xSemaphoreCreateBinary();
    assert(sema);

    create_ulp_interface_queue();
    xTaskCreatePinnedToCore(gpio_task, "gpio task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(send_task, "read rtc data", 2048, NULL, 2, NULL, 1);
    send_gpio_evt();
    esp_err_t err = rtc_isr_register(&ulp_isr, NULL, RTC_CNTL_SAR_INT_ST_M);
    ESP_ERROR_CHECK(err);
    REG_SET_BIT(RTC_CNTL_INT_ENA_REG, RTC_CNTL_ULP_CP_INT_ENA_M);

    create_sleep_timer();
}
