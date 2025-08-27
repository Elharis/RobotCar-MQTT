/*
 * mqtt_client.h
 * Updated: Aug 8, 2025
 */

#ifndef MAIN_MQTT_CLIENT_H_
#define MAIN_MQTT_CLIENT_H_

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mqtt_cmd_cb_t)(const char *topic, const char *payload, int len);

/**
 * @brief MQTT başlat ve (opsiyonel) bir topic'e subscribe ol.
 *
 * @param broker_uri   Örn: "mqtt://io.adafruit.com"
 * @param username     NULL olabilir
 * @param password     NULL olabilir
 * @param sub_topic    NULL değilse bağlanınca subscribe edilir
 */
esp_err_t mqtt_app_start(const char *broker_uri,
                         const char *username,
                         const char *password,
                         const char *sub_topic);

int mqtt_publish(const char *topic, const char *payload, int qos, int retain);

void mqtt_set_command_callback(mqtt_cmd_cb_t cb);

bool mqtt_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_MQTT_CLIENT_H_ */
