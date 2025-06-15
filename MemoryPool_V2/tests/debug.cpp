#include "../include/MemoryPoolV2.h"
#include <iostream>
#include <vector>

using namespace MemoryPoolV2;
int main()
{

    std::vector<void *> ptrs;
    constexpr size_t size = 30;
    for (size_t i = 0; i < 100; ++i)
    {
        ptrs.push_back(MemoryPool::allocate(size));
    }
    for (size_t i = 0; i < 80; ++i)
    {
        void *ptr = ptrs[i];
        MemoryPool::deallocate(ptr, size);
    }
    return 0;
}