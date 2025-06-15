#pragma once
#include <cstddef>
#include <cstdlib>
#include <atomic>
#include <array>

namespace MemoryPoolV2
{
    // 对齐数和定义大小
    constexpr size_t ALIGNMENT = 8;
    constexpr size_t MAX_BYTES = 256 * 1024; // 256KB
    constexpr size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT;

    // 内存块头部信息
    struct BlockHeader
    {
        size_t size;       // 内存块大小
        bool inUse;        // 使用标志
        BlockHeader *next; // 指向下一个内存块
    };
    // 大小类管理
    class SizeClass
    {
    public:
        // 将bytes向上对齐至ALIGNMENT的整数倍
        static size_t roundUp(size_t bytes)
        {
            // ~(ALIGNMEN-1)取非
            return (bytes + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
        }
        // 根据输入的字节数返回对应的内存槽链表下标
        static size_t getIndex(size_t bytes)
        {
            bytes = std::max(bytes, ALIGNMENT);
            return (bytes + ALIGNMENT - 1) / ALIGNMENT - 1;
        }
    };
}