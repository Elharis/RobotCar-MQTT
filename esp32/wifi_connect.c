/*
 * wifi_connect.c
 *
 *  Created on: Aug 4, 2025
 *      Author: LENOVO
 */


#include "wifi_connect.h"
#include "esp_err.h"
#include "esp_event_base.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi_types_generic.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include <stdint.h>
#include <string.h>

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
#define MAX_RETRY 5


static void wifi_event_handler(void* arg , esp_event_base_t event_base, int32_t event_id, void* event_data){
	if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
		esp_wifi_connect();
	}else if(event_base == WIFI_EVENT && event_id  == WIFI_EVENT_STA_DISCONNECTED){
		if(s_retry_num < MAX_RETRY){
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG , "retrying wifi connection");
		}else{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
	}else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
		ip_event_got_ip_t* event = (ip_event_got_ip_t *)event_data;
		ESP_LOGI(TAG, "Got ip:" IPSTR, IP2STR(&event -> ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}



void wifi_connect_init(const char *ssid, const char *password){
	s_wifi_event_group = xEventGroupCreate();
	
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi init finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to SSID:%s", ssid);
    } else {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", ssid);
    }
    
}






















