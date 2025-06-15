#pragma once
#include "Common.h"

namespace MemoryPoolV2
{
    class ThreadCache
    {
    public:
        // 静态成员函数，实现线程局部单例，不同线程之间不会互相干扰
        static ThreadCache *getInstance()
        {
            // 通过thread_local，每个线程都有自己独立的一份副本
            static thread_local ThreadCache instance;
            return &instance;
        }
        void *allocate(size_t size);
        void deallocate(void *ptr, size_t size);

    private:
        ThreadCache()
        {
            freeList_.fill(nullptr);
            freeListSize_.fill(0);
        }
        // 从中心缓存获取内存
        void *fetchFromCentralCache(size_t index);
        // 归还内存至中心缓存
        void returnToCentralCache(void *start, size_t size);
        // 计算批量获取内存块的数量
        size_t getBatchNum(size_t size);
        // 判断是否需要归还给中心缓存
        bool shouldReturnToCentralCache(size_t index);

    private:
        // 每个线程的自由链表数组
        std::array<void *, FREE_LIST_SIZE> freeList_;
        // 自由链表大小统计
        std::array<size_t, FREE_LIST_SIZE> freeListSize_;
    };
}