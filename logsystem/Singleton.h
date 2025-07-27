//
// Created by a2057 on 25-7-27.
//

#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>
#include <mutex>

template<typename T>
class Singleton {
public:
    ~Singleton() = default;

    Singleton(Singleton const &) = delete;

    Singleton &operator=(Singleton const &) = delete;

    template<typename... Args>
    static T &getInstance(Args &&... args) {
        static T instance(std::forward<Args>(args)...);
        return instance;
    }

protected:
    Singleton() = default;
};

#endif //SINGLETON_H
