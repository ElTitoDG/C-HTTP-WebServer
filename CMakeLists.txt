cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(WebServer)

add_executable(WebServer webserver.c)

set_property(TARGET WebServer PROPERTY C_STANDARD 23)
set(CMAKE_C_FLAGS "-Wall -arch arm64 -g")
