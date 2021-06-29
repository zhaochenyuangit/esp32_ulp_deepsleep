#include "sntpController.h"

static const char *TAG = "SNTP";

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void initializeSntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    //pool.ntp.org
    //ntp1.in.tum.de -> use this server in Garching
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}

void obtainTime(void)
{
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};

    char strftime_buf[64];

    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    setenv("TZ", "CET-1CEST", 1);
    tzset();

    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
}

void vUpdateTimeStamp()
{
    while (1)
    {
        time_t now = 0;
        struct tm timeinfo = {0};
        char strftime_buf[8];
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);
        //timestampDisplayString = strftime_buf;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
