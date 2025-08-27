#ifndef MAIN_UART_H_
#define MAIN_UART_H_

#include "driver/uart.h"
#include "driver/gpio.h"

#define MAIN_UART_PORT        UART_NUM_1
#define MAIN_UART_TX          GPIO_NUM_17   // ESP32 TX -> STM32 RX (PD6)
#define MAIN_UART_RX          GPIO_NUM_16   // ESP32 RX <- STM32 TX (PD5)
#define MAIN_UART_BUF_SIZE    1024
#define MAIN_UART_BAUDRATE    115200

void uart_link_init(void);
void uart_link_send_command(const char *cmd);
int  uart_link_read_line(char *buffer, size_t max_len);

#endif /* MAIN_UART_H_ */
