#include "ulp_wake.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

QueueHandle_t gpio_evt_queue;
static const char *TAG = "ulp wake";
static RTC_DATA_ATTR int32_t ulp_data_array_ptr = 0;
static const int ulp_data_array_len = 20; //must be same as EVT_ARRAY_SIZE_WORDS in barrier_main.S
static inline int low_16(int num) { return (num & 0xffff); }

void init_ulp_program()
{
    rtc_gpio_init(RTC_PIN_1);
    rtc_gpio_set_direction(RTC_PIN_1, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_dis(RTC_PIN_1);
    rtc_gpio_pulldown_dis(RTC_PIN_1);
    rtc_gpio_hold_en(RTC_PIN_1);

    rtc_gpio_init(RTC_PIN_2);
    rtc_gpio_set_direction(RTC_PIN_2, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_dis(RTC_PIN_2);
    rtc_gpio_pulldown_dis(RTC_PIN_2);
    rtc_gpio_hold_en(RTC_PIN_2);

    // You MUST load the binary before setting shared variables!
    esp_err_t err = ulp_load_binary(0, ulp_main_bin_start,
                                    (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);
    ulp_set_wakeup_period(0, ULP_RESTART_TIMER_MS * 1000);
    /* Start the ULP program */
    err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "init ulp finish");
}

void create_ulp_interface_queue()
{
    gpio_evt_queue = xQueueCreate(10, sizeof(gpio_evt_msg));
    assert(gpio_evt_queue);
}

void send_gpio_evt()
{
    int end = low_16(ulp_gpio_evt_n);
    int current = ulp_data_array_ptr;
    ulp_data_array_ptr = end;
    if (end == current)
    {
        return;
    }
    int shift = end - current;
    if (shift < 0)
    {
        shift += ulp_data_array_len;
    }

    int32_t *evt_array = (int32_t *)(&ulp_gpio_evt_array);
    int32_t *timestamp_low_array = (int32_t *)(&ulp_timestamp_low_array);
    int32_t *timestamp_mid_array = (int32_t *)(&ulp_timestamp_mid_array);
    int32_t *timestamp_high_array = (int32_t *)(&ulp_timestamp_hi_array);
    for (int i = current; i < (current + shift); i++)
    {
        int index = (i < ulp_data_array_len) ? i : (i - ulp_data_array_len);
        int event = evt_array[index];
        gpio_evt_msg msg;
        msg.rtc_gpio_num = (event >> 4) & (0xfff);
        msg.level = (event) & (0xf);
        int64_t high = (int64_t)(low_16(timestamp_high_array[index])) << 32;
        int64_t mid = low_16(timestamp_mid_array[index]) << 16;
        int64_t low = low_16(timestamp_low_array[index]);
        msg.timestamp_10us = high | mid | low;
        xQueueSend(gpio_evt_queue, &msg, 0);
    }

    return;
}