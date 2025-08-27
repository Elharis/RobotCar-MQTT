#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "wifi_connect.h"
#include "app_mqtt.h"      
#include "uart.h"          

// ---- MQTT ----
#define MQTT_BROKER_URI     "mqtt://io.adafruit.com"
#define MQTT_USERNAME       "Judjol"
#define MQTT_PASSWORD       "aio_JYmb70ysYupOwi23DZ2mR15J2kmc"
#define MQTT_FEED_COMMAND   "Judjol/feeds/robotcar-dot-commands"

#define ESP32_LED_GPIO GPIO_NUM_2

static const char *TAG = "APP";

static void stm32_send_line(const char *msg)
{
    if (!msg || !msg[0]) return;
    ESP_LOGI(TAG, "MQTT->UART: %s", msg);
    uart_link_send_command(msg);  
}

static void uart_rx_task(void *arg)
{
    (void)arg;
    char line[256];

    ESP_LOGI(TAG, "UART RX task started (115200-8N1)");
    for (;;) {
        int n = uart_link_read_line(line, sizeof(line));
        if (n > 0) {
            ESP_LOGI(TAG, "UART<-STM32: %s", line);   
            
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

static void on_mqtt_cmd(const char *topic, const char *data, int len)
{
    (void)topic;

    char msg[128];
    int n = (len < (int)sizeof(msg) - 1) ? len : ((int)sizeof(msg) - 1);
    memcpy(msg, data, n);
    msg[n] = '\0';

    if      (!strcmp(msg, "LED:ON"))  gpio_set_level(ESP32_LED_GPIO, 1);
    else if (!strcmp(msg, "LED:OFF")) gpio_set_level(ESP32_LED_GPIO, 0);

    stm32_send_line(msg);
}

void app_main(void)
{
    uart_link_init();

    gpio_set_direction(ESP32_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ESP32_LED_GPIO, 0);

    wifi_connect_init("Redmi 13", "1234554321");

    mqtt_set_command_callback(on_mqtt_cmd);
    mqtt_app_start(MQTT_BROKER_URI, MQTT_USERNAME, MQTT_PASSWORD, MQTT_FEED_COMMAND);

    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 10, NULL);

   
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
