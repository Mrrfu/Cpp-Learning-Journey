#pragma once
#include "Common.h"
#include <mutex>

namespace MemoryPoolV2
{
    class CentralCache
    {
    public:
        static CentralCache &getInstance()
        {
            static CentralCache instance;
            return instance;
        }
        void *fetchRange(size_t index, size_t batchNum);
        void returnRange(void *start, size_t num_to_return, size_t bytes);

    private:
        CentralCache()
        {
            // 将所有原子指针设为nullptr
            for (auto &ptr : centralFreeList_)
            {
                ptr.store(nullptr, std::memory_order_relaxed);
            }
            // 初始化所有锁
            for (auto &lock : locks_)
            {
                lock.clear();
            }
        }
        // 从页缓存获取内存
        void *fetchFromPageCache(size_t size);

    private:
        // 中心缓存的自由链表
        std::array<std::atomic<void *>, FREE_LIST_SIZE> centralFreeList_;
        // 用于同步的自旋锁
        std::array<std::atomic_flag, FREE_LIST_SIZE> locks_;
    };
}