#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

namespace MyMemoryPool
{
#define MEMORY_POOL_NUM 64
#define SLOT_BASE_SIZE 8
#define MAX_SLOT_SIZE 512
    struct Slot
    {
        std::atomic<Slot *> next;
    };
    class MemoryPool
    {
    public:
        MemoryPool(size_t BlockSize = 4096);
        ~MemoryPool();
        void init(size_t);
        void *allocate();        // 向元素分配一个空间
        void deallocate(void *); // 回收内存

    private:
        void allocateNewBlock();                  // 向操作系统申请新的内存块
        size_t padPointer(char *p, size_t align); // 计算对齐内存所需补全字节数
        // 使用CAS操作进行无锁插入和弹出
        bool pushFreeList(Slot *slot);
        Slot *popFreeList();

    private:
        int BlockSize_;                // 内存块大小，一般为4KB
        int SlotSize_;                 // 槽大小
        Slot *firstBlock_;             // 指向内存池管理的首个实际内存块
        Slot *curSlot_;                // 指向当前内存块未被使用的槽
        std::atomic<Slot *> freeList_; // 指向空闲的槽（被使用后又被释放的槽）
        Slot *lastSlot_;               // 当前内存块中最后能够存放元素的位置标识
        std::mutex mutexForBlock_;     // 保证多线程情况下避免不必要的重复开辟内存导致的浪费行为
    };
    class HashBucket
    {
    public:
        static void initMemoryPool();
        static MemoryPool &getMemoryPool(int index);

        static void *useMemory(size_t size)
        {
            if (size <= 0)
                return nullptr;
            if (size > MAX_SLOT_SIZE) // 大于512字节，直接分配不使用内存池
                return operator new(size);
            return getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).allocate();
        }
        // 将空间添加至对应的内存池的空闲链表
        static void freeMemory(void *ptr, size_t size)
        {
            if (!ptr)
                return;
            if (size > MAX_SLOT_SIZE)
            {
                operator delete(ptr);
                return;
            }
            getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).deallocate(ptr);
        }
        template <typename T, typename... Args>
        friend T *newElement(Args &&...args);
        template <typename T>
        friend void deleteElement(T *p);
    };
    template <typename T, typename... Args>
    T *newElement(Args &&...args)
    {
        // reinterpret_cast用于指针类型擦除
        T *p = nullptr;
        if ((p = reinterpret_cast<T *>(HashBucket::useMemory(sizeof(T)))) != nullptr)
            new (p) T(std::forward<Args>(args)...); // 在分配的内存空间上就地构造
        return p;
    }
    template <typename T>
    void deleteElement(T *p)
    {
        if (p)
        {
            p->~T(); // 调用析构函数
            HashBucket::freeMemory(reinterpret_cast<void *>(p), sizeof(T));
        }
    }
}