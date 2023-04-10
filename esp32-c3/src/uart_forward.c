/*
 * Copyright (c) 2023 PureEngineering LLC
 *
 */

#include <zephyr/kernel.h>
#include "uart_driver.h"

#include <hal/uart_ll.h>
#include <hal/usb_serial_jtag_ll.h>

const struct device *const usb_uart_dev = DEVICE_DT_GET(DT_NODELABEL(usb_serial));
const struct device *const uart0_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

K_SEM_DEFINE(usb_uart_rx_sem, 0, 1);
K_SEM_DEFINE(uart0_rx_sem, 0, 1);

struct k_poll_event events[2] = {
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
									K_POLL_MODE_NOTIFY_ONLY,
									&usb_uart_rx_sem, 0),
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
									K_POLL_MODE_NOTIFY_ONLY,
									&uart0_rx_sem, 0),
};

#define UART_BUFFER_SIZE 1024

uint8_t usb_uart_rx_buf[UART_BUFFER_SIZE], usb_uart_tx_buf[UART_BUFFER_SIZE];
uint8_t uart0_rx_buf[UART_BUFFER_SIZE], uart0_tx_buf[UART_BUFFER_SIZE];

uart_struct usb_uart, uart0;

int uart_forward_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	static uint8_t buf[UART_BUFFER_SIZE];
	int len;
	int flush_needed = 0;

	if (!device_is_ready(usb_uart_dev)) {
		printk("USB_SERIAL device not ready\n");
		return -ENODEV;
	}

	if (!device_is_ready(uart0_dev)) {
		printk("UART0 device not ready\n");
		return -ENODEV;
	}

	init_uart_driver(&usb_uart, usb_uart_dev,
					usb_uart_tx_buf, sizeof(usb_uart_tx_buf),
					usb_uart_rx_buf, sizeof(usb_uart_rx_buf),
					&usb_uart_rx_sem);

	init_uart_driver(&uart0, uart0_dev,
					uart0_tx_buf, sizeof(uart0_tx_buf),
					uart0_rx_buf, sizeof(uart0_rx_buf),
					&uart0_rx_sem);

	while (1)
	{

		len = uart_data_get(&usb_uart, buf, 32);
		if (len) {
			uart_data_put(&uart0, buf, len);
		}


		if(usb_serial_jtag_ll_txfifo_writable())
		{
			if(uart_byte_get(&uart0, buf))
			{
				usb_serial_jtag_ll_write_txfifo( buf, 1);
				flush_needed = 1;
			}
			else if(flush_needed)
			{
				usb_serial_jtag_ll_txfifo_flush();
				flush_needed = 0;
			}
		}

		//something is wrong with the esp usb uart driver and tx irq not triggering. once this is fixed, then the below should work. for now, just using the polling method above.
		// len = uart_data_get(&usb_uart, buf, 32);
		// if (len) {
		// 	uart_data_put(&uart0, buf, len);
		// }

	}
}

K_THREAD_DEFINE(uart_forward, 1024, uart_forward_thread, NULL, NULL, NULL,
	10, 0, 0);
