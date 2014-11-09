cmake_minimum_required(VERSION 2.8.4)
project(C_p2p)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES main.c)
add_executable(dbsg ${SOURCE_FILES})