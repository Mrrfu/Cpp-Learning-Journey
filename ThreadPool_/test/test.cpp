#include "../include/ThreadPool.h"
#include <iostream>

int main()
{
    ThreadPool::ThreadPool threadpool(3);
    auto res = threadpool.enqueue([]()
                                  { std::cout << "hello,threadpool!" << std::endl; });
    res.get();
    return 0;
}