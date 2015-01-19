cmake_minimum_required(VERSION 2.8.4)
project(PR1_CLION)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES webclient_V2.c webserver_V2.c)

add_executable(PR1_CLION ${SOURCE_FILES})
