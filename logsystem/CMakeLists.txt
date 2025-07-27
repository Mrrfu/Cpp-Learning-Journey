cmake_minimum_required(VERSION 3.31)
project(logsystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(logsystem main.cpp
        Logger.h
        Logger.cpp
        Singleton.h)
