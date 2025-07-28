//
// Created by a2057 on 25-7-27.
//

#include "Logger.h"


Logger &Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    logFile_.open("log.txt", std::ios::app);
    if (!logFile_.is_open()) {
        throw std::runtime_error("Could not open log file");
    }
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

void Logger::log(std::string const &message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        //获取时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
        logFile_ << std::string(buffer) << ": " << message << std::endl;
    }
}




