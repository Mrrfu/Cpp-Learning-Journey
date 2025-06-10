#include "../include/MemoryPool.h"
#include <iostream>
#include <vector>
using namespace MyMemoryPool;
int main()
{
    HashBucket::initMemoryPool();
    int *p = newElement<int>(4);
    int *p2 = newElement<int>(8);
    deleteElement(p);
    int *p3 = newElement<int>(10);
    int *p4 = newElement<int>(12);
    // std::cout << "Hello!" << std::endl;
    std::cout << *p3 << " " << *p2 << " " << *p4 << std::endl;
    return 0;
}