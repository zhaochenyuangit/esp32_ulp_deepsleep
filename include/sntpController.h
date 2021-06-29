#ifndef SNTPCONTROLLER_H
#define SNTPCONTROLLER_H

#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_sntp.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

void initializeSntp(void);
void obtainTime();
void vUpdateTimeStamp();
#endif