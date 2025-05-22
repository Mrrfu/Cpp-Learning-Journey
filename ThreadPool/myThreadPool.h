#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <memory>
#include <future>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <typeinfo>
class ThreadPool
{
public:
    ThreadPool(int threadNumber) : isStop(false)
    {
        for (int i = 0; i < threadNumber; ++i)
        {
            workers.emplace_back([this]()
                                 { this->worker(); });
        }
    }
    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            isStop = true;
        }
        cv.notify_all(); // 通知所线程池中的线程停止工作
        for (std::thread &th : workers)
        {
            th.join();
        }
    }
    template <typename F, typename... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<decltype(f(args...))>; // 向任务队列提交任务，返回一个结果future

private:
    bool isStop; // 线程池停止工作标志
    std::mutex mtx;
    std::condition_variable cv;              // 条件变量，用于等待任务或提交任务
    std::vector<std::thread> workers;        // 存放线程
    std::queue<std::function<void()>> tasks; // 任务队列，存放任务
    void worker();                           // 工作函数
};

void ThreadPool::worker()
{
    // 执行函数
    while (true)
    {
        // 这里函数为什么是void，因为在提交任务至队列时，使用lambda[task]{(*task)();},这个lambda是一个void函数
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]
                    { return this->isStop || !this->tasks.empty(); });  //||，当线程池停止标志为true或任务队列有任务时唤醒线程进行工作
            if (this->isStop && this->tasks.empty()) //线程池标记为停止以及没有任务时，线程停止；如果被标记为停止，但是有任务未完成，那么所有任务被完成后再退出线程（优雅关闭）
                return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

/*
enqueue函数整体的流程：
1.使用std::bind()将传入的任务函数和参数绑定；
2.使用std::packaged_task封装为一个异步任务
3.使用智能指针指向这个异步任务
4.创建一个future对象，以获取执行结果；
5.使用Lambda函数，在Lambda里面执行这个异步任务函数（此时为一个智能指针）；
6.将这个Lambda函数加入任务队列，随后通知某个线程有任务；
7.返回future对象
*/
template <typename F, typename... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
{
    // 第一步，函数封装
    auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)); // 这里已经把所有的参数绑定了
    // 第二步，获取函数执行结果
    auto res = task->get_future();
    // 加入任务队列
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (isStop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task]
                      { (*task)(); });
    }
    cv.notify_one();
    return res;
}
