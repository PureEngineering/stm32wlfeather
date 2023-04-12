#!/bin/bash

ZEPHYR_VERSION=v3.3.0

zephyr_sdk_version=0.16.0
ZEPHYR_SDK_DIR=zephyr-sdk-$zephyr_sdk_version

IDF_PATH=~/zephyr_$ZEPHYR_VERSION/modules/hal/espressif/

env_description="setups the path for the toolchain"
env()
{
	export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
	export ZEPHYR_SDK_INSTALL_DIR=~/$ZEPHYR_SDK_DIR
	export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python

	source ~/zephyr_$ZEPHYR_VERSION/zephyr/zephyr-env.sh
}

build_description="does a full build for esp32-c3"
build()
{
	env
	
	rm build -rf
	west build -b esp32c3_devkitm --pristine
}

build_menuconfig_description="menuconfig on current build"
build_menuconfig()
{
	env
	west build -t menuconfig
}

flash_esp32_description="flash to the board"
flash_esp32()
{
	env
	west flash
	# west espressif monitor
	echo "Press CNTL-a then CNTL-x to exit"
	picocom -b115200 /dev/ttyUSB0
}

esp_monitor_description="monitor an esp32"
esp_monitor()
{
	env
	west espressif monitor
}

west_build_description="west build"
west_build()
{
	env
	west build
}

info_report_description="create a report of the board info"
info_report()
{
	env
	west build -t ram_report > ram_report.txt
	west build -t rom_report > rom_report.txt
}

west_update_for_esp_description="run this once before builing for the esp32"
west_update_for_esp()
{
	env
	west update
	west blobs fetch hal_espressif

}

copy_build_to_release_description="copy bin to release"
copy_build_to_release()
{
    env
    cp build/zephyr/zephyr.bin ../release/esp32c3.bin
}


source ../common/automenu.sh
