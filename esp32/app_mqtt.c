/*
 * mqtt_client.c
 * Updated: Aug 8, 2025
 */

#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "app_mqtt.h"  
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t s_client = NULL;
static mqtt_cmd_cb_t            s_cmd_cb = NULL;
static bool                     s_connected = false;

static char s_sub_topic[256] = {0};

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        s_connected = true;
        ESP_LOGI(TAG, "MQTT connected");
        if (s_sub_topic[0] != '\0') {
            int mid = esp_mqtt_client_subscribe(s_client, s_sub_topic, 0);
            ESP_LOGI(TAG, "Subscribed to %s (mid=%d)", s_sub_topic, mid);
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        s_connected = false;
        ESP_LOGW(TAG, "MQTT disconnected");
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "DATA: topic=%.*s len=%d",
                 event->topic_len, event->topic, event->data_len);
        if (s_cmd_cb) {
            s_cmd_cb(event->topic, event->data, event->data_len);
        }
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "SUBSCRIBED mid=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "PUBLISHED mid=%d", event->msg_id);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT EVENT ERROR");
        break;

    default:
        break;
    }
}

esp_err_t mqtt_app_start(const char *broker_uri,
                         const char *username,
                         const char *password,
                         const char *sub_topic)
{
    if (s_client) {
        ESP_LOGW(TAG, "MQTT already started");
        return ESP_OK;
    }
    if (!broker_uri || !broker_uri[0]) {
        ESP_LOGE(TAG, "broker_uri required");
        return ESP_ERR_INVALID_ARG;
    }

    if (sub_topic && sub_topic[0]) {
        strncpy(s_sub_topic, sub_topic, sizeof(s_sub_topic)-1);
    } else {
        s_sub_topic[0] = '\0';
    }

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = broker_uri,
    };
    if (username) cfg.credentials.username = username;
    if (password) cfg.credentials.authentication.password = password;

    s_client = esp_mqtt_client_init(&cfg);
    if (!s_client) {
        ESP_LOGE(TAG, "esp_mqtt_client_init failed");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_err_t err = esp_mqtt_client_start(s_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_mqtt_client_start err=%d", err);
        return err;
    }
    return ESP_OK;
}

int mqtt_publish(const char *topic, const char *payload, int qos, int retain)
{
    if (!s_client || !topic || !topic[0]) {
        ESP_LOGE(TAG, "publish failed: invalid client/topic");
        return -1;
    }
    int len = payload ? (int)strlen(payload) : 0;
    return esp_mqtt_client_publish(s_client, topic, payload, len, qos, retain);
}

void mqtt_set_command_callback(mqtt_cmd_cb_t cb)
{
    s_cmd_cb = cb;
}

bool mqtt_is_connected(void)
{
    return s_connected;
}
