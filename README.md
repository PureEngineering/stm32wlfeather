# STM32WL Feather


# setup 

Install Zephyr 3.3.0 https://github.com/zephyrproject-rtos/zephyr/releases/tag/v3.3.0
Install West and compilers for all (or arm+risv V)

https://docs.zephyrproject.org/latest/develop/getting_started/index.html


For ubuntu 22.04 users
There is a ./runme.sh script one can use to build and flash the code more easily. 



# stm32wl Code
takes whatever data is received on the uart and sends it over lora to another device that send sends it out the uart.  Effectively becoming a wireless uart bridge. 
Note that higher datarates are chosen for faster data rates at a reduction of range. For maximum range, adjustment are required at the tradeoff of lower speeds. 

# esp32-cs code
takes the USB uart data and relays it to the uart connected to the stm32wl. Effectively becoming a usb<->uart bridge.   Additional functionally can be added