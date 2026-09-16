#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace minidb {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

namespace {
const char* LevelToStr(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG_LEVEL: return "DEBUG";
        case LogLevel::INFO_LEVEL: return "INFO";
        case LogLevel::WARN_LEVEL: return "WARN";
        case LogLevel::ERROR_LEVEL: return "ERROR";
    }
    return "?";
}
}  // namespace

void Logger::Log(LogLevel level, const std::string& message) {
    if (level < min_level_) return;
    std::lock_guard<std::mutex> guard(mutex_);

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &now_c);
#else
    localtime_r(&now_c, &tm_buf);
#endif

    std::ostream& out = (level == LogLevel::ERROR_LEVEL) ? std::cerr : std::cout;
    out << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "][" << LevelToStr(level) << "] "
        << message << std::endl;
}

}  // namespace minidb
