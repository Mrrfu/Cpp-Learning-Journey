#include "MemoryPool.h"

namespace MyMemoryPool
{
    MemoryPool::MemoryPool(size_t BlockSize)
        : BlockSize_(BlockSize),
          SlotSize_(0),
          firstBlock_(nullptr),
          curSlot_(nullptr),
          freeList_(nullptr),
          lastSlot_(nullptr)
    {
    }
    MemoryPool::~MemoryPool()
    {
        Slot *cur = firstBlock_;
        while (cur)
        {
            Slot *next = cur->next;
            // 转为void 指针，因为void类型不需要调用析构函数，只释放空间
            operator delete(reinterpret_cast<void *>(cur));
            cur = next;
        }
    }
    // 分配内存槽大小
    void MemoryPool::init(size_t size)
    {
        assert(size > 0);
        SlotSize_ = size;
        firstBlock_ = nullptr;
        curSlot_ = nullptr;
        freeList_ = nullptr;
        lastSlot_ = curSlot_;
    }
    void *MemoryPool::allocate()
    {
        // 第一步，检查空闲链表有无内存槽
        std::cout << "申请内存槽..." << std::endl;
        Slot *slot = popFreeList();
        if (slot != nullptr)
            return slot;
        Slot *temp;
        {
            std::lock_guard<std::mutex> lock(mutexForBlock_);
            // 第二步，如果空闲链表为空，向当前内存块申请内存槽
            if (curSlot_ >= lastSlot_)
            {
                // 如果内存块没有空间了，就向操作系统申请新的内存块
                allocateNewBlock();
            }
            temp = curSlot_;
            std::cout << "申请成功..." << std::endl;
            // SlotSize_是当前内存块槽大小（Slotsize_字节大小），sizeof(Slot)是当前管理头大小
            // std::cout << sizeof(Slot) << std::endl;
            // 等价于：curSlot_ += SlotSize_ / sizeof(Slot);
            curSlot_ = reinterpret_cast<Slot *>(reinterpret_cast<char *>(curSlot_) + SlotSize_);
        }
        return temp;
    }
    void MemoryPool::deallocate(void *ptr)
    {
        if (!ptr)
            return;
        // 转换为Slot *
        Slot *slot = reinterpret_cast<Slot *>(ptr);
        // 放入空闲链表中
        pushFreeList(slot);
    }
    void MemoryPool::allocateNewBlock()
    {
        // 1.申请原始内存块并构建“块链表”
        // 头插法插入新的内存块
        void *newBlock = operator new(BlockSize_);
        reinterpret_cast<Slot *>(newBlock)->next = firstBlock_;
        firstBlock_ = reinterpret_cast<Slot *>(newBlock);

        // 2.计算可分配区域的起始点
        //  这里首先要留出指针空间
        char *body = reinterpret_cast<char *>(newBlock) + sizeof(Slot *);
        // 计算对齐需要填充内存的大小
        size_t paddingSzie = padPointer(body, SlotSize_);
        curSlot_ = reinterpret_cast<Slot *>(body + paddingSzie);

        // 3.计算可分配区域的结束点
        lastSlot_ = reinterpret_cast<Slot *>(reinterpret_cast<size_t>(newBlock) + BlockSize_ - SlotSize_ + 1);
        // 这里或许可能会有疑问：为什么置空？
        // 单线程情况下，因为只有当空闲链表为空时才会申请新的内存块
        // 但是多线程可能会导致内存泄漏，因此这行代码存在问题
        freeList_ = nullptr;
    }
    size_t MemoryPool::padPointer(char *p, size_t algin)
    {
        // algin是槽大小
        return algin - (reinterpret_cast<size_t>(p) % algin);
    }
    // 实现无锁入队操作
    bool MemoryPool::pushFreeList(Slot *slot)
    {
        while (true)
        {
            // 获取当前头节点
            Slot *oldHead = freeList_.load(std::memory_order_relaxed);
            // 将新节点的 next 指向当前头节点
            slot->next.store(oldHead, std::memory_order_relaxed);

            // 尝试将新节点设置为头节点
            if (freeList_.compare_exchange_weak(oldHead, slot,
                                                std::memory_order_release, std::memory_order_relaxed))
            {
                return true;
            }
            // 失败：说明另一个线程可能已经修改了 freeList_
            // CAS 失败则重试
        }
    }

    // 实现无锁出队操作
    Slot *MemoryPool::popFreeList()
    {
        while (true)
        {
            Slot *oldHead = freeList_.load(std::memory_order_acquire);
            if (oldHead == nullptr)
                return nullptr; // 队列为空

            // 在访问 newHead 之前再次验证 oldHead 的有效性
            Slot *newHead = nullptr;
            try
            {
                newHead = oldHead->next.load(std::memory_order_relaxed);
            }
            catch (...)
            {
                // 如果返回失败，则continue重新尝试申请内存
                continue;
            }

            // 尝试更新头结点
            // 原子性地尝试将 freeList_ 从 oldHead 更新为 newHead
            if (freeList_.compare_exchange_weak(oldHead, newHead,
                                                std::memory_order_acquire, std::memory_order_relaxed))
            {
                return oldHead;
            }
            // 失败：说明另一个线程可能已经修改了 freeList_
            // CAS 失败则重试
        }
    }
    void HashBucket::initMemoryPool()
    {
        std::cout << "初始化内存池..." << std::endl;
        for (int i = 0; i < MEMORY_POOL_NUM; ++i)
        {
            getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
        }
    }
    MemoryPool &HashBucket::getMemoryPool(int index)
    {
        static MemoryPool memoryPool[MEMORY_POOL_NUM];
        return memoryPool[index];
    }
}
