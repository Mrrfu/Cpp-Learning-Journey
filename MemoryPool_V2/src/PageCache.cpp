#include "../include/PageCache.h"
#include <sys/mman.h>
#include <cstring>

namespace MemoryPoolV2
{
    void *PageCache::allocateSpan(size_t numPages)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // 查找合适的空闲span
        // lower_bound函数返回第一个大于等于numPages的元素的迭代器
        auto it = freeSpans_.lower_bound(numPages);
        if (it != freeSpans_.end())
        {
            // 取出span链表
            Span *span = it->second;

            // 将取出的span从原有的空闲链表freeSpans_[it->first]中移除
            if (span->next)
            {
                // 如果不止一个span
                freeSpans_[it->first] = span->next;
            }
            else
            {
                // 如果只有一个，直接清除列表
                freeSpans_.erase(it);
            }

            // 如果span大于需要的numPages则进行分割
            if (span->numPages > numPages)
            {
                Span *newSpan = new Span;
                newSpan->pageAddr = static_cast<char *>(span->pageAddr) +
                                    numPages * PAGE_SIZE;
                newSpan->numPages = span->numPages - numPages;
                newSpan->next = nullptr;

                // 将超出部分放回空闲Span*列表头部
                auto &list = freeSpans_[newSpan->numPages];
                newSpan->next = list;
                list = newSpan;

                // 将分配的numPages修改为对应大小
                span->numPages = numPages;
            }
            // 记录span信息用于回收
            spanMap_[span->pageAddr] = span;
            return span->pageAddr;
        }
        // 没有合适的span，向系统申请
        void *memory = systemAlloc(numPages);
        // 如果申请失败，返回nullptr
        if (!memory)
            return nullptr;
        Span *span = new Span;
        span->pageAddr = memory;
        span->numPages = numPages;
        span->next = nullptr;
        // 记录span信息用于回收
        spanMap_[memory] = span;
        return memory;
    }
    void PageCache::deallocateSpan(void *ptr, size_t numPage)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // 查找对应的span，如果没有则代表不是PageCache分配的内存
        auto it = spanMap_.find(ptr);
        if (it == spanMap_.end())
            return;

        Span *span = it->second;

        // 尝试合并相邻的span，减少内存锁片化
        void *nextAddr = static_cast<char *>(ptr) + numPage * PAGE_SIZE;
        auto nextIt = spanMap_.find(nextAddr);

        if (nextIt != spanMap_.end())
        {
            Span *nextSpan = nextIt->second;

            // 1.检查nextSpan是否在空闲链表中
            bool found = false;
            auto &nextList = freeSpans_[nextSpan->numPages];
            if (nextList == nextSpan)
            {
                // 头结点匹配，直接移除
                nextList = nextSpan->next;
                found = true;
            }
            else if (nextList)
            {
                // 遍历链表查找并移除
                Span *prev = nextList;
                while (prev->next)
                {
                    if (prev->next == nextSpan)
                    {
                        prev->next = nextSpan->next;
                        found = true;
                        break;
                    }
                    prev = prev->next;
                }
            }

            // 2.只有在找到nextSpan的情况下才进行合并
            if (found)
            {
                span->numPages += nextSpan->numPages;
                spanMap_.erase(nextAddr);
                delete nextSpan;
            }
        }

        // 将合并后的span通过头插法插入空闲链表
        auto &list = freeSpans_[span->numPages];
        span->next = list;
        list = span;
    }

    // 从堆上申请一段内存
    void *PageCache::systemAlloc(size_t numPages)
    {
        // size为需要分配的总字节数
        size_t size = numPages * PAGE_SIZE;
        // 使用mmap进行匿名内存映射
        /*
        nullptr：内核自动选择映射地址
        size：要映射的内存大小（单位：字节）
        PROT_READ | PROT_WRITE：可读可写权限
        MAP_PRIVATE | MAP_ANONYMOUS：创建私有匿名映射（不映射到文件）
        MAP_ANONYMOUS：表示直接从系统申请的匿名内存
        MAP_PRIVATE：表示对这段内存的修改不会影响到其他进程
        */
        void *ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // mmap失败，返回空指针
        if (ptr == MAP_FAILED)
            return nullptr;
        // 将分配的内存全部初始化为0
        memset(ptr, 0, size);
        return ptr;
    }
} // namespace MemoryPoolV2
