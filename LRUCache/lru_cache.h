#pragma once

#include <iostream>
#include <unordered_map>
#include <optional>
#include <vector>
#include <list>
#include <string>
#include <cstdint>
#include <functional>
#include <mutex>

// 封装数据
struct ByteView
{
    std::vector<char> data_{};

    ByteView(const std::string &str)
    {
        data_.resize(str.size());
        std::copy(str.begin(), str.end(), data_.begin());
    }
    auto Len() const -> int64_t
    {
        return data_.size();
    }
    auto ToString() const -> std::string
    {
        return std::string(data_.begin(), data_.end());
    }
};
using ByteViewOptional = std::optional<ByteView>;

// 键值存储单元
struct Entry
{
    std::string key_;
    ByteView value_;
    Entry(std::string k, const ByteView &v) : key_(std::move(k)), value_(v)
    {
    }
    // 重载比较运算符
    auto operator==(const Entry &other)
    {
        return key_ == other.key_ && value_.ToString() == other.value_.ToString();
    }
};
/*
 *LRU: 淘汰最近最久未使用的缓存
 * 使用一个双向链表list。当访问或更新缓存时，对应的entry会被移动到链表头部；当缓存已满时，则将最久没有使用的节点删除，即链表表尾的节点。
 * LRU缓存的实现核心是哈希表+双向链表的组合，这里采用STL中的std::unordered_map和std::list实现；std::unordered_map用于保存指向entry在链表位置的迭代器，std::list的节点为一个Entry
 */
class LRUCache
{
    using EvictedFunc = std::function<void(std::string, ByteView)>;
    using ListElementIter = std::list<Entry>::iterator;

public:
    LRUCache(int max_bytes, const EvictedFunc &evicted_func = nullptr) : max_bytes_(max_bytes), evicted_func_(evicted_func) {}
    auto Get(const std::string &key) -> ByteViewOptional;
    void Set(const std::string &key, const ByteView &);
    void Delete(const std::string &key);
    void RemoveOldest();

private:
    int64_t bytes_ = 0;
    int64_t max_bytes_;
    EvictedFunc evicted_func_;
    // key未缓存的键，value指向list_中对应元素的迭代器
    std::unordered_map<std::string, ListElementIter> cache_;
    // 维护缓存访问条目的访问顺序，链表头部存放最近访问的元素，链表尾部存放最久未访问的元素
    std::list<Entry> list_;
    std::mutex mtx_;
};