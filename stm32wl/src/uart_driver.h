
#ifndef UART_DRIVER_H
#define UART_DRIVER_H

typedef struct
{
	unsigned char *q;		/* pointer to queue buffer*/
	int read_index;         /* index used for reading */
	int write_index;        /* index used for writing */
	int size;			    /* size of queue */
} uart_queue;


void init_uart_queue(uart_queue *q,unsigned char* buffer, int size);
int put_uart_queue(uart_queue *q, unsigned char x);
int get_uart_queue(uart_queue *q,unsigned char *x);

typedef struct
{
	const struct device *uart_dev;

	uart_queue tx_queue;
	uart_queue rx_queue;

} uart_struct;


int uart_putchar(uart_struct *uart, uint8_t data);
int uart_getchar(uart_struct *uart, uint8_t *data_ptr);

void init_uart_driver(uart_struct * uart, const struct device *uart_dev,
						uint8_t *tx_buffer, int tx_size,
						uint8_t *rx_buffer, int rx_size );

void print_uart_state(uart_struct * uart);
void uart_change_buad(uart_struct * uart, int newbaud);
void uart_change_parity(uart_struct * uart, int parity);

void uart_set_power_state(uart_struct * uart,uint32_t new_state);

void reset_uart_queues(uart_struct * uart );



#endif
