#include <stdio.h>
#include "stdlib.h"
#include <string.h>
#include "time.h"     //for timeval
#include <sys/time.h> //for POSIX function gettimeofday()
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_sleep.h"

#define SLEEP_TIMER_MS 2000
#define WAKE_UP_INTERVAL_S 600
#define S_TO_US_FACTOR 1000000

#define DEEP_SLEEP

void print_slept_time(void);
void create_sleep_timer(void);
void restart_sleep_timer(void);