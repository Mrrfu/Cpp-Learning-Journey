#include "../include/MemoryPool.h"
#include <iostream>
#include <vector>
using namespace MyMemoryPool;
int main()
{
    HashBucket::initMemoryPool();
    int *p = newElement<int>(4);
    int *p2 = newElement<int>(8);
    // std::cout << "Hello!" << std::endl;
    if (p == nullptr)
    {
        std::cerr << "ERROR: Allocation failed!" << std::endl;
        return 1;
    }
    std::cout << *p << " " << *p2 << std::endl;
    return 0;
}