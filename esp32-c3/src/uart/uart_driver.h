
/*
 * Copyright (c) 2023 PureEngineering, LLC
 *
 * UART utility using the ring buffer of Zephyr kernel
 */
#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/ring_buffer.h>

typedef struct
{
	struct ring_buf rb;
	uint8_t *buffer;
	uint16_t size;
} uart_ring_buf;

typedef struct
{
	const struct device *uart_dev;
	/* This semaphore should be initialized from consumer thread */
	struct k_sem *rx_sem;

	uart_ring_buf tx_rb;
	uart_ring_buf rx_rb;

} uart_struct;

void uart_rb_init(uart_ring_buf *urb, uint8_t* buffer, uint16_t size);
int uart_byte_put(uart_struct *uart, uint8_t data);
int uart_byte_get(uart_struct *uart, uint8_t *data_ptr);
int uart_data_put(uart_struct *uart, uint8_t *data, uint16_t size);
int uart_data_get(uart_struct *uart, uint8_t *data, uint16_t size);

void init_uart_driver(uart_struct *uart, const struct device *uart_dev,
						uint8_t *tx_buffer, int tx_size,
						uint8_t *rx_buffer, int rx_size,
						struct k_sem *rx_sem);

void print_uart_state(uart_struct *uart);
void uart_change_buad(uart_struct *uart, int newbaud);
void uart_change_parity(uart_struct *uart, int parity);

void uart_set_power_state(uart_struct *uart,uint32_t new_state);

void reset_uart_queues(uart_struct *uart);

#define uart_putchar(...) uart_byte_put(__VA_ARGS__)
#define uart_getchar(...) uart_byte_get(__VA_ARGS__)

#endif /* __UART_DRIVER_H__ */
