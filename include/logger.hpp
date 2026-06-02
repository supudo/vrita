#ifndef VRITA_LOGGER_INCLUDES
#define VRITA_LOGGER_INCLUDES

#include <iostream>
#include <ctime>
#include <cstdarg>
#include <string>
#include <iomanip>
#include <sstream>

class Logger {
public:
    void logMessage(const std::string& message) {
        std::cout << "[" << currentTime() << "] " << message << std::endl;
    }

    void log(const char* fmt, va_list args) {
        char buf[8192];
        vsnprintf(buf, 8192, fmt, args);
        std::cout << "[" << currentTime() << "] " << std::string(buf) << std::endl;
    }

    void log(const char* fmt, ...)  {
        char buf[8192];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, 8192, fmt, args);
        va_end(args);
        std::cout << "[" << currentTime() << "] " << std::string(buf) << std::endl;
    }

    auto str_format(const char* fmt, va_list args) -> std::string {
        char buf[8192];
        vsnprintf(buf, 8192, fmt, args);
        return std::string(buf);
    }

    auto str_format(const char* fmt, ...) -> std::string {
        char buf[8192];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, 8192, fmt, args);
        va_end(args);
        return std::string(buf);
    }

private:
    std::string currentTime() {
        std::time_t now = std::time(nullptr);
        std::tm tm_struct;
        localtime_s(&tm_struct, &now);
        std::ostringstream oss;
        oss << std::put_time(&tm_struct, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

#endif
