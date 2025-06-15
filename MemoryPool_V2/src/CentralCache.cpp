#include "../include/CentralCache.h"
#include "../include/PageCache.h"
#include <cassert>
#include <thread>
#include <iostream>

namespace MemoryPoolV2
{
    // 每次从PageCache获取Span大小（以页为单位）
    static const size_t SPAN_PAGES = 8;
    void *CentralCache::fetchRange(size_t index, size_t batchNum)
    {
        // 索引检查，当索引大于等于FREE_LIST_SIZE时，说明申请内存过大应向系统申请
        if (index >= FREE_LIST_SIZE || batchNum == 0)
        {
            return nullptr;
        }
        // 自旋锁保护
        while (locks_[index].test_and_set(std::memory_order_acquire))
        {
            // 添加线程让步，避免忙等待导致过度消耗CPU
            std::this_thread::yield();
        }
        void *result = nullptr;
        try
        {
            // 检查entralFreeList_[index] 是否有空闲内存块
            result = centralFreeList_[index].load(std::memory_order_relaxed);
            if (!result)
            {
                // 中心缓存为空，从页缓存获取新的内存块
                size_t size = (index + 1) * ALIGNMENT;
                result = fetchFromPageCache(size);
                if (!result)
                {
                    // 获取失败
                    locks_[index].clear(std::memory_order_release);
                    return nullptr;
                }
                // 将从PageCache获得的内存块切分成小块
                char *start = static_cast<char *>(result);
                // 总的块数，size是一个块的字节数量
                size_t totalBlocks = (SPAN_PAGES * PageCache::PAGE_SIZE) / size;
                // 分配块数
                size_t allocBlcoks = std::min(batchNum, totalBlocks);

                // 构建返回给ThreadCache的内存块链表
                if (allocBlcoks > 1)
                {
                    // 至少要有两个才能构成一个链表
                    for (size_t i = 1; i < allocBlcoks; ++i)
                    {
                        void *current = start + (i - 1) * size;
                        void *next = start + i * size;
                        // 下一个块的地址
                        *reinterpret_cast<void **>(current) = next;
                    }
                    // 将最后一个的内存块指向设置为nullptr
                    *reinterpret_cast<void **>(start + (allocBlcoks - 1) * size) = nullptr;
                }
                // 多余的块存储在中心缓存空闲链表
                if (totalBlocks > allocBlcoks)
                {
                    void *remainStart = start + allocBlcoks * size;
                    for (size_t i = allocBlcoks + 1; i < totalBlocks; ++i)
                    {
                        void *current = start + (i - 1) * size;
                        void *next = start + i * size;
                        *reinterpret_cast<void **>(current) = next;
                    }
                    *reinterpret_cast<void **>(start + (totalBlocks - 1) * size) = nullptr;
                    centralFreeList_[index].store(remainStart, std::memory_order_release);
                }
            }
            else // 如果中心缓存有index对应大小的内存块
            {
                void *current = result;
                void *prev = nullptr;
                size_t count = 0;
                while (current && count < batchNum)
                {
                    prev = current;
                    current = *reinterpret_cast<void **>(current);
                    count++;
                }
                if (prev)
                {
                    // 当centralFreeList_[index]链表上的内存块大于batchNum时会用到
                    *reinterpret_cast<void **>(prev) = nullptr;
                }
                centralFreeList_[index].store(current, std::memory_order_release);
            }
        }
        catch (...)
        {
            locks_[index].clear(std::memory_order_release);
            throw;
        }
        locks_[index].clear(std::memory_order_release);
        return result;
    }
    void CentralCache::returnRange(void *start, size_t num_to_return, size_t index)
    {
        // 当索引大于等于FREE_LISE_SIZE时，说明内存过大应该直接向系统归还
        if (!start || index >= FREE_LIST_SIZE)
            return;
        while (locks_[index].test_and_set(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
        try
        {
            void *end = start;
            for (size_t i = 0; i < num_to_return - 1; ++i)
            {
                void *nextNode = *reinterpret_cast<void **>(end);
                if (nextNode == nullptr)
                {
                    break;
                }
                end = nextNode;
            }
            // size_t count = 1;
            /*
            size设置为归还内存块数量*内存块字节数，这个设计虽然有一些逻辑问题，
            但是并不影响整体，因为*reinterpret_cast<void **>(end) != nullptr会先触发
             */
            // while (*reinterpret_cast<void **>(end) != nullptr && count < size)
            // {
            //     end = *reinterpret_cast<void **>(end);
            //     count++;
            // }
            // 将归还的链表连接到中心缓存的链表头部
            void *current = centralFreeList_[index].load(std::memory_order_relaxed);
            // 将原链表头接到归还链表的尾部
            *reinterpret_cast<void **>(end) = current;
            // 将归还的链表头设置为新的链表头
            centralFreeList_[index].store(start, std::memory_order_release);
        }
        catch (...)
        {
            locks_[index].clear(std::memory_order_release);
            throw;
        }
        locks_[index].clear(std::memory_order_release);
    }

    void *CentralCache::fetchFromPageCache(size_t size)
    {
        // 1. 计算实际需要的页数
        size_t numPages = (size + PageCache::PAGE_SIZE - 1) / PageCache::PAGE_SIZE;
        // 2.根据大小决定分配策略
        if (size <= SPAN_PAGES * PageCache::PAGE_SIZE)
        {
            // 小于等于32KB的请求，使用固定8页
            return PageCache::getInstance().allocateSpan(SPAN_PAGES);
        }
        else
        {
            // 大于32KB的请求，按实际需求分配
            return PageCache::getInstance().allocateSpan(numPages);
        }
    }
}