
#include <zephyr/drivers/uart.h>
#include "uart_driver.h"

///start queue code
// this queue will allow one interrupt and one thread to work on it without a sem lock.
void uart_rb_init(uart_ring_buf *urb, uint8_t* buffer, uint16_t size)
{
	ring_buf_init(&urb->rb, size, buffer);
	urb->buffer = buffer;
	urb->size = size;
}

int uart_rb_data_put(uart_ring_buf *urb, uint8_t *data, uint16_t size)
{
	return ring_buf_put(&urb->rb, data, size);
}

int uart_rb_data_get(uart_ring_buf *urb, uint8_t *data, uint16_t size)
{
	return ring_buf_get(&urb->rb, data, size);
}

int uart_rb_byte_put(uart_ring_buf *urb, uint8_t b)
{
	return uart_rb_data_put(urb, &b, sizeof(b));
}

int uart_rb_byte_get(uart_ring_buf *urb, uint8_t *b)
{
	return uart_rb_data_get(urb, b, sizeof(uint8_t));
}

int uart_data_put(uart_struct *uart, uint8_t *data, uint16_t size)
{
	if(uart == NULL)
	{
		return -1;
	}
	uint32_t ret = uart_rb_data_put(&uart->tx_rb, data, size);
	if (ret)
	{
		uart_irq_tx_enable(uart->uart_dev);
	}
	return ret;
}

int uart_data_get(uart_struct *uart, uint8_t *data, uint16_t size)
{
	if(uart == NULL)
	{
		return -1;
	}
	return uart_rb_data_get(&uart->rx_rb, data, size);
}

int uart_byte_put(uart_struct *uart, uint8_t data)
{
	if(uart == NULL)
	{
		return -1;
	}
	if(uart_rb_byte_put(&uart->tx_rb, data))
	{
		uart_irq_tx_enable(uart->uart_dev);
		return 1;
	}
	return 0;
}

int uart_byte_get(uart_struct *uart, uint8_t *data_ptr)
{
	if(uart == NULL)
	{
		return -1;
	}
	return uart_rb_byte_get(&uart->rx_rb, data_ptr);
}

void uart_cb(const struct device *dev, void *user_data)
{
	uart_struct *uart = user_data;
	uint8_t rx_buf[32];
	int rx_size = 0;

	uint8_t tx_buf[32];
	int tx_size = 0;

	uart_irq_update(dev);

	if (uart_irq_rx_ready(dev))
	{
		rx_size = uart_fifo_read(dev, rx_buf, sizeof(rx_buf));
		uart_rb_data_put(&uart->rx_rb, rx_buf, rx_size);

		if (uart->rx_sem != NULL) {
			k_sem_give(uart->rx_sem);
		}
	}

	if (uart_irq_tx_ready(dev))
	{
		tx_size = uart_rb_data_get(&uart->tx_rb, tx_buf, sizeof(tx_buf));

		if (tx_size > 0)
		{
			uart_fifo_fill(dev, tx_buf, tx_size);
		}
		else
		{
			uart_irq_tx_disable(dev); // Output buffer empty, disable irq;
		}
	}
}

void init_uart_driver(uart_struct *uart, const struct device *uart_dev,
						uint8_t *tx_buffer, int tx_size,
						uint8_t *rx_buffer, int rx_size,
						struct k_sem *rx_sem)
{
	uart->uart_dev = uart_dev;
	uart->rx_sem = rx_sem;

	uart_rb_init(&uart->tx_rb, tx_buffer, tx_size);
	uart_rb_init(&uart->rx_rb, rx_buffer, rx_size);

	uart_irq_callback_user_data_set(uart_dev, uart_cb, uart);

	uart_irq_rx_enable(uart_dev);
}

void reset_uart_queues(uart_struct *uart)
{
	ring_buf_reset(&uart->rx_rb.rb);
	uart_rb_init(&uart->rx_rb, uart->rx_rb.buffer, uart->rx_rb.size);
	ring_buf_reset(&uart->tx_rb.rb);
	uart_rb_init(&uart->tx_rb, uart->tx_rb.buffer, uart->tx_rb.size);
}

void uart_set_power_state(uart_struct *uart, uint32_t new_state)
{
	const struct device *dev = uart->uart_dev;

	if(uart->uart_dev)
	{
		if(new_state == 1)
		{
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
		printk("uart_config.baudrate=%d\n", uart_cfg.baudrate);
		printk("uart_config.parity=%d\n", uart_cfg.parity);
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


