cmake_minimum_required(VERSION 3.9)
project(main)

add_executable(p1_scheduler
        interrupt.c
        interrupt.h
        main.c
        Makefile
        mythread.h
        mythreadlib.c
        queue.c
        queue.h)

add_custom_target(main COMMAND make -C ${main_SOURCE_DIR} CLION_EXE_DIR=${PROJECT_BINARY_DIR})
