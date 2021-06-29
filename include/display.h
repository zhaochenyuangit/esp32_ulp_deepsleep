#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "ssd1306.h"

char* groupDisplayString;
char* timestampDisplayString;
char displayCount[8];
char displayCountPrediction[8];

void initDisplay();
void refreshCount();
void displayText(char* text);
void displayTextTime(char* text, int time);
void showRoomState();
#endif /* DISPLAYCONTROLLER_H */