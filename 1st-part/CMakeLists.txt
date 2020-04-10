cmake_minimum_required(VERSION 3.16)
project(proj.out C)

set(CMAKE_C_STANDARD 99)

add_executable(simpledu simpledu.c pipe.c)
add_executable(proj main.c pipe.c)