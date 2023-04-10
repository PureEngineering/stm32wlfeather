/*
 * Copyright (c) 2023 PureEngineering, LLC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/gpio.h>


#include "uart_driver.h"

#define MAX_LORA_DATA_LEN (255)
#define lora_thread_STACK_SIZE 2048
#define lora_thread_PRIORITY 7

#define LORA_RSSI_CLEAR_THRESHOLD -90
#define LORA_MAX_CTS_CHECK_MS 1000
#define LORA_MAX_WAIT_TX_DONE_MS 10000
#define LORA_MAX_POLL_TIME_MS 1000

#define LORA_FREQ 915100000


const struct gpio_dt_spec system_led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(system_led), gpios, {0});
const struct device *uart_dev = DEVICE_DT_GET(DT_ALIAS(uart));
const struct device *lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
static uart_struct uart_ctx;

static uint8_t uart_rx_buf[1024];
static uint8_t uart_tx_buf[1024];

// Simple queue to buffer messages that need to send
typedef struct
{
	uint8_t data[MAX_LORA_DATA_LEN];
	int length;
} lora_data_t;

typedef struct
{
	unsigned char *q;		  /* pointer to queue buffer*/
	volatile int read_index;  /* index used for reading */
	volatile int write_index; /* index used for writing */
	int p_size;				  /* size of index in queue */
	int size;				  /* max number of indexes */
} lora_queue_t;

void lora_queue_init(lora_queue_t *q, int queue_field_size, unsigned char queue_size)
{
	q->read_index = 0;
	q->write_index = 0;
	q->size = queue_size;
	q->p_size = queue_field_size;
	q->q = malloc(queue_field_size * queue_size);
}

int lora_queue_put(lora_queue_t *q, void *p)
{
	int nextindex = (q->write_index + 1) % q->size;
	if (nextindex != q->read_index)
	{
		int offset = nextindex * q->p_size;
		memcpy(q->q + offset, p, q->p_size);
		q->write_index = nextindex;
		return 1;
	}
	else
	{
		return 0; // queue is full
	}
}

int lora_queue_get(lora_queue_t *q, void *p)
{
	if (q->read_index == q->write_index)
	{
		return 0; // queue is empty
	}
	int nextindex = (q->read_index + 1) % q->size;
	int offset = nextindex * q->p_size;
	memcpy(p, q->q + offset, q->p_size);
	q->read_index = nextindex;
	return 1;
}

lora_queue_t lora_queue;

///

K_THREAD_STACK_DEFINE(lora_thread_stack_area, lora_thread_STACK_SIZE);
struct k_thread lora_thread_data;

int16_t lora_rssi = 0;

int lora_configure(uint32_t frequency)
{
	struct lora_modem_config config;
	int ret;
	/* Configure LoRa device */
	lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	if (!device_is_ready(lora_dev))
	{
		printk("%s Device not found\n", lora_dev->name);
		return -EIO;
	}
	else
	{
		config.frequency = frequency;
		config.bandwidth = BW_500_KHZ;
		config.datarate = SF_10;
		config.preamble_len = 32;
		config.coding_rate = CR_4_5;
		config.tx_power = 16;
		config.tx = true;
		ret = lora_config(lora_dev, &config);
		if (ret < 0)
		{
			printk("LoRa config failed %d\n", ret);
			return -EIO;
		}
		else
		{
			printk("LoRa config success\n");
		}
	}
	
	return 0;
}

int lora_rx_state = 0;

void lora_thread(void *unused1, void *unused2, void *unused3)
{
	printk("Started lora_thread\n");


	lora_data_t lora_data;
	int16_t rssi;
	int8_t snr;

	lora_configure(LORA_FREQ);

	lora_send(lora_dev, "hello lora world\n", 16);

	while (1)
	{
		k_yield();

		int length = 0;
		lora_rx_state = 1;
		if ((length = lora_recv(lora_dev, lora_data.data, MAX_LORA_DATA_LEN, K_MSEC(LORA_MAX_POLL_TIME_MS), &rssi, &snr)) > 0)
		{
			lora_data.length = length;
			gpio_pin_set(system_led.port, system_led.pin, 1);
			for (int i = 0; i < length; i++)
			{
				uart_poll_out(uart_dev, lora_data.data[i]);
			}
			gpio_pin_set(system_led.port, system_led.pin, 0);
		}

		lora_rx_state = 0;

		while (lora_queue_get(&lora_queue, &lora_data))
		{
			lora_send(lora_dev, lora_data.data, lora_data.length);
		}
	}
}


    


void main(void)
{
	uint8_t byte;

	printk("*********************************\n");
	printk("**       STM32WL LoRa demo     **\n");
	printk("*********************************\n");
	printk("COMPILE DATE/TIME %s %s\n", __DATE__, __TIME__);

	lora_data_t lora_data;

	lora_queue_init(&lora_queue, sizeof(lora_data_t), 10);

	/* Configure system-led */
	gpio_pin_configure_dt(&system_led, GPIO_OUTPUT_INACTIVE);


	/* Test LED blink */
	for (int i = 0; i < 3; i++)
	{
		gpio_pin_set(system_led.port, system_led.pin, 1);
		k_msleep(100);
		gpio_pin_set(system_led.port, system_led.pin, 0);
		
		k_msleep(10);
	}

	/* Get UART device */
	if (!device_is_ready(uart_dev))
	{
		printk("Could not find %s device\n", uart_dev->name);
	}

	init_uart_driver(&uart_ctx, uart_dev, uart_tx_buf, sizeof(uart_tx_buf), uart_rx_buf, sizeof(uart_rx_buf));

	/* Configure LoRa device */
	if (!device_is_ready(lora_dev))
	{
		printk("%s Device not found\n", lora_dev->name);
	}
	else
	{
		k_thread_create(&lora_thread_data, lora_thread_stack_area, K_THREAD_STACK_SIZEOF(lora_thread_stack_area), lora_thread, NULL, NULL, NULL, lora_thread_PRIORITY, 0, K_NO_WAIT);
	}

	while (1)
	{

		lora_data.length = 0;
		while (uart_getchar(&uart_ctx, &byte))
		{
			uart_poll_out(uart_dev, byte); // echo back to uart

			if ((lora_data.length + 1) < MAX_LORA_DATA_LEN)
			{
				lora_data.data[lora_data.length] = byte;
				lora_data.length++;
			}
			else
			{
				break;
			}
		}

		if (lora_data.length > 0)
		{
			lora_queue_put(&lora_queue, &lora_data);
		}

		k_msleep(1);
	}
}
