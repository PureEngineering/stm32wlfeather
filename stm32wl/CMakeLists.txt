#
# Copyright 2021 PureEngineering, LLC
#

cmake_minimum_required(VERSION 3.13.1)
set (CMAKE_C_FLAGS "-fdiagnostics-color=always")

ADD_DEFINITIONS(-DNETWORK_CHANNEL=${NETWORK_CHANNEL})

# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(wl55_demo)

FILE(GLOB_RECURSE app_sources
  src/*.c
)

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/.)


target_sources(app PRIVATE ${app_sources}) 
