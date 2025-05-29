cmake_minimum_required(VERSION 3.20.0) 
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(display_tests)

include_directories(../../include)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/display.c
               ${CMAKE_CURRENT_SOURCE_DIR}/display_test.c)
