// Простой потокобезопасный логгер уровня приложения
#pragma once

#include <mutex>
#include <string>

namespace minidb {

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
    static Logger& Instance();

    void Log(LogLevel level, const std::string& message);

    void SetLevel(LogLevel level) { min_level_ = level; }

private:
    Logger() = default;
    std::mutex mutex_;
    LogLevel min_level_ = LogLevel::INFO;
};

#define LOG_DEBUG(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::DEBUG, msg)
#define LOG_INFO(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::INFO, msg)
#define LOG_WARN(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::WARN, msg)
#define LOG_ERROR(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::ERROR, msg)

}  // namespace minidb
