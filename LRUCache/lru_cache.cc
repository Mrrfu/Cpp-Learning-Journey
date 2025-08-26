#include "lru_cache.h"

auto LRUCache::Get(const std::string &key) -> ByteViewOptional
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (cache_.find(key) == cache_.end())
    {
        return std::nullopt;
    }
    auto ele = cache_[key];
    auto [k, value] = *ele;
    list_.erase(ele);
    list_.emplace_front(key, value);
    cache_[key] = list_.begin();
    return value;
}

void LRUCache::Set(const std::string &key, const ByteView &value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (cache_.find(key) != cache_.end())
    {
        // 缓存中有这个元素，直接更新
        auto ele = cache_[key];
        bytes_ += value.Len() - ele->value_.Len();
        list_.erase(ele); // 删除，因为需要添加到头部
    }
    else
    {
        // 缓存中没有这个元素
        bytes_ += key.size() + value.Len();
    }
    list_.emplace_front(key, value);
    cache_[key] = list_.begin();

    // 检查是否超出缓存大小
    while (max_bytes_ > 0 && bytes_ > max_bytes_ && !list_.empty())
    {
        RemoveOldest();
    }
}

void LRUCache::Delete(const std::string &key)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (cache_.find(key) != cache_.end())
    {
        auto ele = cache_[key];
        auto [_, value] = *ele;
        bytes_ = bytes_ - (key.size() + value.Len());
        list_.erase(ele);
        cache_.erase(key);
        if (evicted_func_)
        {
            evicted_func_(key, value);
        }
    }
}

void LRUCache::RemoveOldest()
{
    if (list_.empty())
        return;
    auto [key, value] = list_.back();
    cache_.erase(key);
    list_.pop_back();
    bytes_ -= key.size() + value.Len();
    if (evicted_func_)
    {
        evicted_func_(key, value);
    }
}
