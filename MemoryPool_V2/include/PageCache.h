#pragma once
#include "Common.h"
#include <map>
#include <mutex>

namespace MemoryPoolV2
{
    class PageCache
    {
    public:
        static const size_t PAGE_SIZE = 4096; // 页大小为4KB
        // 单例模式
        static PageCache &getInstance()
        {
            static PageCache instance;
            return instance;
        }
        // 分配指定页数的span
        void *allocateSpan(size_t numPages);
        // 释放span
        void deallocateSpan(void *ptr, size_t numPages);

    private:
        // 在private里表示禁止在外部使用构造函数创建类对象
        PageCache() = default;
        // 向系统申请内存
        void *systemAlloc(size_t numPages);

    private:
        struct Span
        {
            void *pageAddr;  // 页起始地址
            size_t numPages; // 页数
            Span *next;      // 链表指针
        };
        // 按页数管理空闲Span，不同页数对应不同的Sapn链表
        std::map<size_t, Span *> freeSpans_;
        // 页号到Span的映射，用于回收
        std::map<void *, Span *> spanMap_;
        std::mutex mutex_;
    };
}