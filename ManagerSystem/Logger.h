//
// Created by a2057 on 25-7-27.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>


class Logger {
public:
    static Logger &getInstance();

    Logger(Logger const &) = delete;

    Logger &operator=(const Logger &) = delete;

    ~Logger();

    void log(std::string const &message);

private:
    Logger();

private:
    std::mutex mutex_;
    std::ofstream logFile_;
};
#endif //LOGGER_H
