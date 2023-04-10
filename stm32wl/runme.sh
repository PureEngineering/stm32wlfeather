#!/bin/bash
ZEPHYR_DIR=zephyr_v3.3.0
env_description="setups the path for the toolchain"
env()
{
    export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
    export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.13.2
    source ~/$ZEPHYR_DIR/zephyr/zephyr-env.sh
}

build_feather_description="does a full build with STM32WL-Feather board"
build_feather()
{
    env

    west build -b stm32wl_feather --pristine 
}

build_nucleo_description="does a full build with Nucleo-WL55 board"
build_nucleo()
{
    env

    west build -b nucleo_wl55jc --pristine 
}

flash_description="flash to the board"
flash()
{
    env
    west flash
    # --tool-opt="-USB 81480416"
}


flash_manual_description="flash using jlinkexe"
flash_manual()
{
    rebuild
	JLinkExe -device STM32WL55CC -if SWD -speed 4000 -autoconnect 1 -CommanderScript runner.jlink
}


info_report_description="create a report of the board info"
info_report()
{
    evn
    build_humgw
    west build -t ram_report
}

copy_build_to_release_description="create a report of the board info"
copy_build_to_release()
{
    env
    cp build/zephyr/zephyr.bin ../release/stl32wl.bin
}


source ../common/automenu.sh
