#include "display.h"

#include "esp_log.h"

void initDisplay()
{
    ssd1306_128x64_i2c_init();
    ssd1306_setFixedFont(ssd1306xled_font6x8);

    timestampDisplayString = "00:00";
    groupDisplayString = "G08";

}

void displayText(char *text)
{
    ssd1306_clearScreen();
    ssd1306_printFixedN(0, 0, text, STYLE_NORMAL, 1);
}

void displayTextTime(char *text, int time)
{
    ssd1306_clearScreen();
    ssd1306_printFixedN(0, 0, text, STYLE_NORMAL, 1);
    delay(time * 1000);
    ssd1306_clearScreen();
}
/*
void showRoomState()
{
    while (1)
    {
        sprintf(displayCount, "%02d",count);
        sprintf(displayCountPrediction, "%02d",internalCount);
        //ssd1306_clearScreen();
        ssd1306_printFixedN(0, 0, groupDisplayString, STYLE_NORMAL, 1);
        ssd1306_printFixedN(60, 0, timestampDisplayString, STYLE_NORMAL, 1);
        ssd1306_printFixedN(5, 30, displayCount, STYLE_NORMAL, 2);
        ssd1306_printFixedN(65, 30, displayCountPrediction, STYLE_NORMAL, 2);

        vTaskDelay(500 / portTICK_PERIOD_MS);
        //ssd1306_clearScreen();
        //vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
*/