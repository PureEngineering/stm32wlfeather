
#include <zephyr/kernel.h>

void main(void)
{
	k_msleep(2000);

	while (1) 
	{
		printk("Hello World! From %s\n", CONFIG_BOARD);
		k_msleep(1000);
	}
}
