#include "../include/ThreadPool.h"

namespace ThreadPool
{
    void ThreadPool::worker()
    {
        std::function<void()> task;
        bool isPop = this->tasks.pop(task);
        while (true)
        {
            while (isPop)
            {
                // 执行函数
                task();
                isPop = this->tasks.pop(task);
            }

            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this, &isPop, &task]()
                    {
                    isPop = this->tasks.pop(task);
                    return isPop||this->isStop; });
            if (!isPop) // 为什么不能是isStop，因为需要把任务完成再停止
            {
                return;
            }
        }
    }
}