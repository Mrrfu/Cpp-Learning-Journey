#include "myThreadPool.h"

int main()
{
    ThreadPool pool(2);
    auto res = pool.enqueue([]()
                            { std::cout<<std::this_thread::get_id()<<std::endl;
                                std::cout << "Hello,ThreadPool!" << std::endl; });

    // auto pool_ptr = std::make_shared<ThreadPool>(std::move(ThreadPool(4)));
    // auto res1 = pool_ptr->enqueue([](int a, int b)
    //                               { return a - b; }, 50, 100);
    res.get();
    // std::cout << res1.get() << std::endl;
    return 0;
}