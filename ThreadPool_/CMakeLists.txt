cmake_minimum_required(VERSION 3.10)
project(thread_pool)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
add_compile_options(-Wall -O2)

# 查找pthread库
find_package(Threads REQUIRED)

# 设置目录
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

# 源文件
file(GLOB SOURCES "${SRC_DIR}/*.cpp")

# 添加头文件目录
include_directories(${INC_DIR})

# 创建单元测试可执行文件
add_executable(test_run 
    ${SOURCES}
    ${TEST_DIR}/test.cpp
)

# 链接pthread库
target_link_libraries(test_run PRIVATE Threads::Threads)
# target_link_libraries(perf_test PRIVATE Threads::Threads)

# 添加测试命令
add_custom_target(test
    COMMAND ./test_run
    DEPENDS test_run
)
