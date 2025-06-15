#include "../include/ThreadCache.h"
#include "../include/CentralCache.h"
#include <iostream>

namespace MemoryPoolV2
{
    void *ThreadCache::allocate(size_t size)
    {
        // 处理0大小分配请求
        if (size == 0)
        {
            size = ALIGNMENT;
        }
        // 大内存对象请求，直接系统分配
        if (size > MAX_BYTES)
        {
            return malloc(size);
        }
        // 获取对应下标
        size_t index = SizeClass::getIndex(size);
        // 更新自由链表大小
        freeListSize_[index]--;
        // 检查线程本地自由链表
        if (void *ptr = freeList_[index])
        {
            // 从freeList_中取出一个可用内存块，并更新freeList_指针链
            //*reinterpret_cast<void **>(ptr);是将存储的下一个指针的地址取出来，然后赋值给链表头
            freeList_[index] = *reinterpret_cast<void **>(ptr);
            return ptr;
        }
        return fetchFromCentralCache(index);
    }
    void ThreadCache::deallocate(void *ptr, size_t size)
    {
        if (size > MAX_BYTES)
        {
            free(ptr);
            return;
        }
        size_t index = SizeClass::getIndex(size);

        // 将这个ptr里面的值存储为下一个节点,freeList_[index]是一个void* 指针
        *reinterpret_cast<void **>(ptr) = freeList_[index];
        freeList_[index] = ptr;
        // 更新自由链表大小
        freeListSize_[index]++;
        // std::cout << "should return!" << std::endl;

        // 判断是否需要将部分内存回收给中心缓存
        if (shouldReturnToCentralCache(index))
        {
            returnToCentralCache(freeList_[index], size);
        }
    }
    bool ThreadCache::shouldReturnToCentralCache(size_t index)
    {
        // 当自由链表的大小超过一定数量时将内存回收给中心缓存
        size_t threshold = 64;
        return (freeListSize_[index] > threshold);
    }
    void *ThreadCache::fetchFromCentralCache(size_t index)
    {
        // 内存块字节大小
        size_t size = (index + 1) * ALIGNMENT;
        // 获取多少个块
        size_t batchNum = getBatchNum(size);
        // 从中心缓存批量获取内存
        void *start = CentralCache::getInstance().fetchRange(index, batchNum);
        // 获取失败，返回nullptr
        if (!start)
            return nullptr;
        // 更新自由链表大小
        freeListSize_[index] += batchNum;

        void *result = start;
        // 如果batchNum小于1就不用更新，因为已经直接返回给用户
        if (batchNum > 1)
        {
            freeList_[index] = *reinterpret_cast<void **>(start);
        }
        return result;
    }
    void ThreadCache::returnToCentralCache(void *start, size_t size)
    {

        size_t index = SizeClass::getIndex(size);
        // 获取对齐后的实际内存块大小（因为size不一定是8的倍数，可能是12,15之类的）
        // size_t alignedSize = SizeClass::roundUp(size);
        // std::cout << "alignedSize: " << alignedSize << std::endl;
        // 要归还内存块数量
        size_t batchNum = freeListSize_[index];
        // 如果只有一个块，不用归还
        if (batchNum <= 1)
            return;
        // 保留一部分在ThreadCache中，比如1/4
        size_t keepNum = std::max(batchNum / 4, size_t(1));
        size_t returnNum = batchNum - keepNum;

        // 将内存块串成链表
        char *current = static_cast<char *>(start);

        char *splitNode = current;
        for (size_t i = 0; i < keepNum - 1; ++i)
        {
            splitNode = reinterpret_cast<char *>(*reinterpret_cast<void **>(splitNode));
            if (splitNode == nullptr)
            {
                // 如果链表提前结束，更新实际返回的数量
                returnNum = batchNum - (i + 1);
                break;
            }
        }
        if (splitNode != nullptr)
        {
            void *nextNode = *reinterpret_cast<void **>(splitNode);
            *reinterpret_cast<void **>(splitNode) = nullptr; // 断开连接

            // 更新ThreadCache的空闲链表
            freeList_[index] = start;
            // 更新自由链表大小
            freeListSize_[index] = keepNum;
            if (returnNum > 0 && nextNode != nullptr)
            {
                CentralCache::getInstance().returnRange(nextNode, returnNum, index);
            }
        }
    }
    size_t ThreadCache::getBatchNum(size_t size)
    {
        // 基准：每次批量获取不超过4KB内存
        constexpr size_t MAX_BATCH_SIZE = 4 * 1024; // 4KB
        size_t baseNum;
        if (size <= 32)
            baseNum = 64; // 64 * 32 = 2KB
        else if (size <= 64)
            baseNum = 32; // 32 * 64 = 2KB
        else if (size <= 128)
            baseNum = 16; // 16 * 128 = 2KB
        else if (size <= 256)
            baseNum = 8; // 8 * 256 = 2KB
        else if (size <= 512)
            baseNum = 4; // 4 * 512 = 2KB
        else if (size <= 1024)
            baseNum = 2; // 2 * 1024 = 2KB
        else
            baseNum = 1; // 大于1024的对象每次只从中心缓存取1个
        size_t maxNum = std::max(size_t(1), MAX_BATCH_SIZE / size);
        // 确保至少返回1
        return std::max(sizeof(1), std::min(maxNum, baseNum));
    }
}