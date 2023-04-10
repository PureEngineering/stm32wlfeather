// #include <zephyr.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include "uart_driver.h"


///start queue code
// this queue will allow one interrupt and one thread to work on it without a sem lock. 
void init_uart_queue(uart_queue *q,unsigned char* buffer, int size)
{
	q->read_index = 0;
	q->write_index = 0;
	q->size = size-1;
	q->q = buffer;
}

int put_uart_queue(uart_queue *q, unsigned char x)
{
	int nextindex = (q->write_index + 1) % q->size;
	if(nextindex != q->read_index)
	{
		q->q[q->write_index] = x;
		q->write_index = nextindex;

		return 1;
	}
	else
	{
		return 0; //queue is full
	}
}

int get_uart_queue(uart_queue *q,unsigned char *x)
{
	if(q->read_index == q->write_index)
	{
		return 0; //queue is empty
	}
	
	int nextindex = (q->read_index + 1) % q->size;
	*x = q->q[q->read_index];
	q->read_index = nextindex;

	return 1;
}

//note: not threadsafe
int put_uart_ringbuffer(uart_queue *q, unsigned char x)
{
	int nextindex = (q->write_index + 1) % q->size;
	while(nextindex == q->read_index)
	{
			//make room for the new data
			int nextreadindex = (q->read_index + 1) % q->size;
			q->read_index = nextreadindex;
	}

	q->q[q->write_index] = x;
	q->write_index = nextindex;

	return 1;
}
///end queue code

//start uart code

int uart_putchar(uart_struct *uart, uint8_t data)
{
	if(put_uart_queue(&uart->tx_queue,data))
	{
		uart_irq_tx_enable(uart->uart_dev);
		return 1;
	}
	return 0;
}

int uart_getchar(uart_struct *uart, uint8_t *data_ptr)
{
	return get_uart_queue(&uart->rx_queue,data_ptr);
}

void uart_cb(const struct device *dev, void *user_data)
{
	uart_struct *uart = user_data;
	uint8_t uartrx_buf[32];
	int uartrx_index = 0;

	uint8_t uarttx_buf[32];
	int uarttx_index =0;

	uart_irq_update(dev);

	if (uart_irq_rx_ready(dev)) 
	{
		uartrx_index = uart_fifo_read(dev, uartrx_buf, sizeof(uartrx_buf));

		if(uartrx_index)
		{
			uartrx_buf[uartrx_index] = 0;
			for(int i=0; i<uartrx_index;i++)
			{
				put_uart_queue(&uart->rx_queue,uartrx_buf[i]);
			}
		}
	}

	if (uart_irq_tx_ready(dev)) 
	{
		if(get_uart_queue(&uart->tx_queue,&uarttx_buf[0]))
		{
			uarttx_index = 1;

			//to reduce isr overhead, pull out more than 1 char to send out. 
			while(get_uart_queue(&uart->tx_queue,&uarttx_buf[uarttx_index]))
			{
				uarttx_index++;
				if(uarttx_index >=sizeof(uarttx_buf)) {  break; }
			}
		}

		if (uarttx_index >0)
		{
			uart_fifo_fill(dev, uarttx_buf, uarttx_index);
		} 
		else 
		{
			
			uart_irq_tx_disable(dev); // Output buffer empty, disable irq;
		}
	}
}
void init_uart_driver(uart_struct * uart, const struct device *uart_dev,
						uint8_t *tx_buffer, int tx_size,
						uint8_t *rx_buffer, int rx_size )
{
	uart->uart_dev = uart_dev;

	init_uart_queue(&uart->tx_queue,tx_buffer,tx_size);
	init_uart_queue(&uart->rx_queue,rx_buffer,rx_size);

	uart_irq_callback_user_data_set(uart_dev, uart_cb, uart);

	uart_irq_rx_enable(uart_dev);
}

void reset_uart_queues(uart_struct * uart )
{
	init_uart_queue(&uart->tx_queue,uart->tx_queue.q,uart->tx_queue.size);
	init_uart_queue(&uart->rx_queue,uart->rx_queue.q,uart->rx_queue.size);
}

static inline const struct uarte_nrfx_config *internal_get_dev_config(const struct device *dev)
{
	return dev->config;
}

void uart_set_power_state(uart_struct * uart,uint32_t new_state)
{
	const struct device *dev = uart->uart_dev;

	if(uart->uart_dev)
	{
		if(new_state == 1)
		{
			//uart_irq_tx_enable(dev);
			uart_irq_rx_enable(dev);
		}
		if(new_state == 0)
		{
			uart_irq_tx_disable(dev);
			uart_irq_rx_disable(dev);
		}
	}
}

void print_uart_state(uart_struct * uart)
{

	struct uart_config uart_cfg;
	int ret;

	const struct device *dev = uart->uart_dev;

	ret = uart_config_get(dev, &uart_cfg);
	if (ret == 0) 
	{
		printk("uart_config.baudrate=%d\n",  uart_cfg.baudrate);
		printk("uart_config.parity=%d\n",    uart_cfg.parity);
		printk("uart_config.stop_bits=%d\n", uart_cfg.stop_bits);
		printk("uart_config.data_bits=%d\n", uart_cfg.data_bits);
		printk("uart_config.flow_ctrl=%d\n", uart_cfg.flow_ctrl);
	} 
	else 
	{
		printk("uart_config_get() error\n");
	}
}

void uart_change_buad(uart_struct * uart,int newbaud)
{

	struct uart_config uart_cfg;
	int ret;

	const struct device *dev = uart->uart_dev;

	ret = uart_config_get(dev, &uart_cfg);

	uart_cfg.baudrate = newbaud;

	uart_configure(dev, &uart_cfg);

}

void uart_change_parity(uart_struct * uart, int parity)
{

	struct uart_config uart_cfg;
	int ret;

	const struct device *dev = uart->uart_dev;

	ret = uart_config_get(dev, &uart_cfg);

	uart_cfg.parity = parity;

	uart_configure(dev, &uart_cfg);
}


