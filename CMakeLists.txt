cmake_minimum_required(VERSION 2.8.4)
project(C_p2p)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES ServerClient.c)

find_package( Threads )
add_executable(dbsg ${SOURCE_FILES})
target_link_libraries( dbsg ${CMAKE_THREAD_LIBS_INIT} )
