cmake_minimum_required(VERSION 3.10)
PROJECT(CppStudy_)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_BUILD_TYPE "Debug")

include_directories(include)


add_library(MemoryPool SHARED src/MemoryPool.cpp)

# 添加要编译的可执行文件，编译main.cpp生成可执行文件main
add_executable(main src/main.cpp)

# 添加可执行文件所需要的库
target_link_libraries(main MemoryPool)