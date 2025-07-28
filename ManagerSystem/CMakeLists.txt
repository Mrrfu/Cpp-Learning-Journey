cmake_minimum_required(VERSION 3.31)
project(ManagerSystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(ManagerSystem main.cpp
        Logger.h
        Logger.cpp
        Task.cpp
        Task.h
        TaskManager.cpp
        TaskManager.h
        Command.cpp
        Command.h)
