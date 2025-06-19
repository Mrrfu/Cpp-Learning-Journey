#include <mutex>
#include <vector>
#include <condition_variable>
#include <queue>
#include <memory>
#include <thread>
#include <functional>
#include <atomic>
#include <future>
#include <iostream>

namespace ThreadPool
{
    // 线程安全队列
    template <typename T>
    class Queue
    {
    public:
        void push(T value)
        {
            std::unique_lock<std::mutex> lock(mtx);
            this->q.push(value);
        }
        bool pop(T &value)
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (this->q.empty())
                return false;
            value = this->q.front();
            q.pop();
            return true;
        }
        size_t size()
        {
            std::unique_lock<std::mutex> lock(mtx);
            return q.size();
        }

    private:
        std::queue<T> q;
        std::mutex mtx;
    };

    class ThreadPool
    {

    public:
        ThreadPool(int n) : isStop(false)
        {
            for (int i = 0; i < n; ++i)
            {

                this->threads.emplace_back(
                    std::thread([this]()
                                { this->worker(); }));
            }
        }
        ~ThreadPool()
        {
            this->isStop = true;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.notify_all();
            }
            for (std::thread &th : this->threads)
            {
                if (th.joinable())
                    th.join();
            }
        }
        template <typename F, typename... Args>
        auto enqueue(F &&f, Args &&...args) -> std::future<decltype(f(args...))>;

    private:
        void worker();
        std::vector<std::thread> threads;
        Queue<std::function<void()>> tasks;
        std::condition_variable cv;
        std::mutex mtx;
        std::atomic<bool> isStop;
    };
    template <typename F, typename... Args>
    auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        // 封装
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto res = task->get_future();
        // 提交至队列
        // std::function<void()>
        if (isStop)
        {
            throw std::runtime_error("enqueue on stopped ThreadPool"); // 线程安全队列，可以取消锁
        }
        this->tasks.push(
            [task]()
            { (*task)(); });

        std::unique_lock<std::mutex> lock(mtx);
        cv.notify_one();

        return res;
    }
}