#include "Logger.h"

#define LOG Logger<2>::getInstance("log.txt")

int main() {
    try {
        // Logger<2> logger("log.txt");
        // auto logger = Logger<2>::getInstance("log.txt");
        LOG.log(LogLevel::INFO, "Starting application...");
        int user_id = 42;
        std::string action = "login";
        double duration = 3.5;
        std::string world = "world";
        LOG.log(LogLevel::INFO, "User {} performed {} in {} seconds", user_id, action, duration);
        LOG.log(LogLevel::DEBUG, "Hello {}", world);
        LOG.log(LogLevel::WARNING, "This is a message without any placeholders");
        LOG.log(LogLevel::ERROR, "Multiple placeholders {}, {}, ", 1, 2, 3);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    std::unique_ptr<int>p(new int(42));
    p.reset();
    return 0;
}
