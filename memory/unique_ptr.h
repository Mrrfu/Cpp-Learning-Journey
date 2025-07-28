//
// Created by a2057 on 25-7-28.
//

#ifndef UNIQUE_PTR_H
#define UNIQUE_PTR_H
#include <utility>
#include <algorithm>

/*
 * 简单的unique_ptr的实现，不是线程安全的
 * RAII思想
 */
template<typename T>
class unique_ptr {
public:
    explicit unique_ptr(T* p = nullptr): ptr_(p) {

    }
    ~unique_ptr() {
        delete ptr_;
    }
    unique_ptr(const unique_ptr&other) = delete;
    unique_ptr& operator = (const unique_ptr&other) = delete;
    //移动构造函数
    unique_ptr(unique_ptr&& other) noexcept :ptr_(std::exchange(other.ptr_,nullptr)) {
    }
    //移动赋值运算符
    unique_ptr& operator = (unique_ptr&& other) noexcept {
        if (this!=&other) {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    T* get() {
        return ptr_;
    }

    T& operator*() {
        return *ptr_;
    }
    T* operator->() {
        return ptr_;
    }
    T* release() {
        return std::exchange(ptr_, nullptr);
    }
    void  reset(T * ptr=nullptr) {
        delete ptr_;
        ptr_ = ptr;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }
    void swap(unique_ptr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }
private:
    T * ptr_;
};

#endif //UNIQUE_PTR_H
