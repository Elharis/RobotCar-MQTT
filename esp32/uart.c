#include "uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "UART";

void uart_link_init(void)
{
    uart_config_t cfg = {
        .baud_rate  = MAIN_UART_BAUDRATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,   
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };

    ESP_ERROR_CHECK(uart_driver_install(MAIN_UART_PORT, MAIN_UART_BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(MAIN_UART_PORT, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(MAIN_UART_PORT, MAIN_UART_TX, MAIN_UART_RX,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART ready (port=%d, TX=%d, RX=%d, %d-8N1)",
             MAIN_UART_PORT, MAIN_UART_TX, MAIN_UART_RX, MAIN_UART_BAUDRATE);
}

void uart_link_send_command(const char *cmd)
{
    if (!cmd) return;
    uart_write_bytes(MAIN_UART_PORT, cmd, strlen(cmd));
    const char lf = '\n';
    uart_write_bytes(MAIN_UART_PORT, &lf, 1);
    ESP_LOGI(TAG, "TX: %s", cmd);
}


int uart_link_read_line(char *buffer, size_t max_len)
{
    if (!buffer || max_len < 2) return 0;

    size_t i = 0;
    uint8_t ch;
    const TickType_t per_byte_to = pdMS_TO_TICKS(20);

    while (i < max_len - 1) {
        int got = uart_read_bytes(MAIN_UART_PORT, &ch, 1, per_byte_to);
        if (got <= 0) break;
        if (ch == '\r') continue;
        if (ch == '\n') break;
        buffer[i++] = (char)ch;
    }
    buffer[i] = '\0';
    return (int)i;
}

