#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "driver/rtc_io.h"
#include "driver/rtc_cntl.h"
#include "esp32/ulp.h"
#include "soc/soc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_sleep.h"

#include "ulp_main.h"

#define RTC_PIN_1 GPIO_NUM_34
#define RTC_PIN_2 GPIO_NUM_35
#define ULP_RESTART_TIMER_MS 20

typedef struct gpio_evt_msg
{
    int64_t timestamp_10us;
    int16_t rtc_gpio_num;
    int16_t level;
} gpio_evt_msg;

extern QueueHandle_t gpio_evt_queue;

void init_ulp_program();
void create_ulp_interface_queue();
void send_gpio_evt();