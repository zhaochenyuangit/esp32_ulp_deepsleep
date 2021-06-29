#include "network_common.h"
#include "time.h"

static const char *TAG = "my MQTT";
static EventGroupHandle_t mqtt_event_group;

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    //esp_mqtt_client_handle_t client = event->client;
    //int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group,MQTT_CONNECTED_BIT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "published, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    /**
     * this intermidiate function is needed because the esp32 mqtt library
     * is somehow dirty: the event handler, which uses the esp_event_handler_t macro,
     * requires the 4th param to be void*, but mqtt callback function is defined to receive
     * a esp_mqtt_event_handler_t. So this function is needed to go around the compiler
    */
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

esp_err_t start_mqtt(esp_mqtt_client_handle_t *client_ptr)
{
    /*generate some random client name to avoid collision on MQTT broker
    esp_ramdom() returns true random number when wifi is enabled*/
    char client_name[20];
    sprintf(client_name,"ESP32:%d",esp_random());

    mqtt_event_group = xEventGroupCreate();
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MYMQTT,
        .port = 1883,
        .client_id = client_name,
    };
    *client_ptr = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(*client_ptr, ESP_EVENT_ANY_ID, mqtt_event_handler, *client_ptr);
    esp_mqtt_client_start(*client_ptr);
    ESP_LOGI(TAG,"wait for flag");
    xEventGroupWaitBits(mqtt_event_group,MQTT_CONNECTED_BIT,pdFALSE,pdFALSE,portMAX_DELAY);
    
    return (ESP_OK);
}

void mqtt_send(esp_mqtt_client_handle_t client, const char *topic,const char *data){
    int msg_id;
    msg_id = esp_mqtt_client_publish(client,topic,data,0,1,0);
    ESP_LOGD(TAG,"mqtt published, msg id=%d",msg_id);
}