cmake_minimum_required(VERSION 4.0)
project(SemestralkaNacovsky C)

set(CMAKE_C_STANDARD 90)

add_executable(SemestralkaNacovsky main.c
        bigint.c
        bigint.h
        stack.c
        stack.h
        parser.c
        parser.h)
