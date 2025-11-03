cmake_minimum_required(VERSION 4.0)
project(fat16)

set(CMAKE_CXX_STANDARD 20)

add_executable(fat16 main.cpp
        DiskManager/OSManager.cpp
        DiskManager/OSManager.h
        DiskManager/FAT16.cpp
        DiskManager/FAT16.h)
