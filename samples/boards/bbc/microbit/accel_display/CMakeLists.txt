cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(m5stack_joystick_display)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
