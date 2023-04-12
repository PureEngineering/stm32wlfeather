# STM32WL Feather

# Getting Started 

- Order a Board here: https://groupgets.com/campaigns/933

Install Zephyr 3.3.0 https://github.com/zephyrproject-rtos/zephyr/releases/tag/v3.3.0
Install West and compilers for all (or arm+risv V)

https://docs.zephyrproject.org/latest/develop/getting_started/index.html

For ubuntu 22.04 users
There is a ./runme.sh script one can use to build and flash the code more easily. 

if using the uart instead of the SWD jtag the latest version of stm32flash is needed. (PA9 and PA10 pins need to be wired) (115200 8E1)
https://gitlab.com/stm32flash/stm32flash

# Hardware
- Feather sized: 2.0" x 0.9" x 0.3" (51mm x 23mm x 8mm) without headers
- esp32-c3: USB<->uart, WiFi, BLE
- stm32wl55: 64kb ram, 256 kB flash, Low Power Arm Core, Lora, (G)FSK, (G)MSK, BPSK
- On board 3.3V regulator
- USB-C directly to esp32-C3 and onboard Li-ion Charger
- Bootloader and Reset buttons for both microcontrollers
- SMA connector for 915Mhz stm32wl
- On board Chip antenna for esp32-c3
- stm32wl power control of the esp32-c3 for low power battery applications. 
- ADC, DAC, SPI, UART, GPIO accessible on Feather Header. 
- TC2030-CTX-NL JTAG connection
- Optional BME688: temperature pressure humiditity, gas sensor, machine learning
- Optional LIS2DTW12TR: accelerometer 
- Optional Li-ion Battery connector

![alt text](https://raw.githubusercontent.com/PureEngineering/stm32wlfeather/main/release/front.jpg?raw=true)

![alt text](https://raw.githubusercontent.com/PureEngineering/stm32wlfeather/main/release/back.jpg?raw=true)

![alt text](https://raw.githubusercontent.com/PureEngineering/stm32wlfeather/main/release/angle.jpg?raw=true)

# Features
- Uses latest Zephyr Project for both the esp32-c3 as well as the stm32wl
- Ultra low power consumpion with stm32wl 
- Early Adopter utilizing a fully working Zephyr Project for Lora, WiFi, BLE, and sensors.

# STM32WL Code
Zephyr Project Based example. Takes whatever data is received on the uart and sends it over lora to another device that send sends it out the uart.  Effectively becoming a wireless uart bridge. 
Note that higher datarates are chosen for faster data rates at a reduction of range. For maximum range, adjustment are required at the tradeoff of lower speeds. 

# ESP32-C3 code
Zephyr Project Based example. Takes the USB uart data and relays it to the uart connected to the stm32wl. Effectively becoming a usb<->uart bridge.   Additional functionally can be added such as BLE and Wi-Fi. Can use other programming environments such as arduino. 

# Intended Future Use Cases
- Low power sensor reporting temperature, pressure, humidity over lora 
- Custom Lora USB/WiFi basestation
- Amazon sidewalk sensor
- LoRaWAN®, Sigfox, W-MBUS, mioty connected sensor


# Schematic
![alt text](https://raw.githubusercontent.com/PureEngineering/stm32wlfeather/main/release/Schematic.png?raw=true)
