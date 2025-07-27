//
// Created by a2057 on 25-7-27.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <atomic>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "Singleton.h"


//辅助函数，将单个参数转化为字符串
template<typename T>
std::string to_string_helper(T &&arg) {
    std::ostringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
}

// 线程安全的任务队列
class LogQueue {
public:
    void push(const std::string &msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        cond_var_.notify_one(); //唤醒一个工作线程处理任务
    }

    bool pop(std::string &msg) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this] {
            return !queue_.empty() || is_shutdown_;
        });

        if (queue_.empty()) {
            //这里只有在is_shutdown_为true即队列被关闭的条件下进入
            //如果队列为空且被关闭，直接返回
            //如果队列不为空，则清空队列
            //所谓“优雅关闭”
            return false;
        }

        msg = queue_.front();
        queue_.pop();
        return true;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        is_shutdown_ = true;
        cond_var_.notify_all(); //通知所有消费者
    }

private:
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    bool is_shutdown_ = false;
};

enum class LogLevel {
    INFO = 0,
    DEBUG = 1,
    WARNING = 2,
    ERROR = 3,
};


template<size_t THREAD_NUM>
class Logger : public Singleton<Logger<THREAD_NUM> > {
    friend class Singleton<Logger>;

public:
    ~Logger() {
        exit_flag_ = true;
        log_queue_.shutdown();

        for (auto &worker: workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    template<typename... Args>
    void log(LogLevel level, const std::string &format, Args &&... args) {
        log_queue_.push(std::string(logLevelToString(level)) + formatMessage(format, std::forward<Args>(args)...));
    }

private:
    explicit Logger(const std::string &filename): log_file_(filename, std::ios::out | std::ios::app),
                                                  exit_flag_(false) {
        if (!log_file_.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }

        for (size_t i = 0; i < THREAD_NUM; ++i) {
            workers_.emplace_back([this]() {
                //打印当前工作线程的id
                std::string msg;
                while (log_queue_.pop(msg)) {
                    //写入日志文件
                    {
                        std::lock_guard<std::mutex> lock(file_mutex_);
                        log_file_ << msg << std::endl;
                    }
                    //写入控制台
                    {
                        std::lock_guard<std::mutex> lock(cout_mutex_);
                        // std::cout <<" "<<std::this_thread::get_id()<<" "<< msg << std::endl; //打印到控制台
                        std::cout << msg << std::endl;
                    }
                }
            });
        }
    }

private:
    LogQueue log_queue_;
    // std::thread worker_thread_;
    std::vector<std::thread> workers_;
    std::ofstream log_file_;
    std::atomic<bool> exit_flag_;
    static std::mutex cout_mutex_;
    static std::mutex file_mutex_;

    template<typename... Args>
    std::string formatMessage(const std::string &format, Args &&... args) {
        std::vector<std::string> arg_strings = {to_string_helper(std::forward<Args>(args))...};
        std::ostringstream oss;

        size_t arg_index = 0;
        size_t pos = 0;
        size_t placeholder = format.find("{}", pos);

        while (placeholder != std::string::npos) {
            oss << format.substr(pos, placeholder - pos);
            if (arg_index < arg_strings.size()) {
                oss << arg_strings[arg_index++];
            } else {
                oss << "{}"; //没有匹配
            }
            pos = placeholder + 2;
            placeholder = format.find("{}", pos);
        }
        oss << format.substr(pos);
        while (arg_index < arg_strings.size()) {
            oss << arg_strings[arg_index++];
        }
        return " [" + getCurrentTime() + "] " + oss.str(); //增加时间戳
    }

    static std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
        return buf;
    }

    static const char *logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO: return "[INFO] ";
            case LogLevel::DEBUG: return "[DEBUG] ";
            case LogLevel::WARNING: return "[WARNING] ";
            case LogLevel::ERROR: return "[ERROR] ";
            default: return "[UNKNOWN] ";
        }
    }
};

template<size_t N>
std::mutex Logger<N>::cout_mutex_;

template<size_t N>
std::mutex Logger<N>::file_mutex_;


#endif //LOGGER_H
