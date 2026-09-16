// Простой потокобезопасный логгер уровня приложения
#pragma once

#include <mutex>
#include <string>

namespace minidb {

// Значения названы с суффиксом _LEVEL, а не просто DEBUG/INFO/WARN/ERROR, потому что
// на Windows <windows.h> определяет одноимённые макросы (например ERROR), которые
// текстово подменяют такие идентификаторы препроцессором и ломают компиляцию.
enum class LogLevel { DEBUG_LEVEL, INFO_LEVEL, WARN_LEVEL, ERROR_LEVEL };

class Logger {
public:
    static Logger& Instance();

    void Log(LogLevel level, const std::string& message);

    void SetLevel(LogLevel level) { min_level_ = level; }

private:
    Logger() = default;
    std::mutex mutex_;
    LogLevel min_level_ = LogLevel::INFO_LEVEL;
};

#define LOG_DEBUG(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::DEBUG_LEVEL, msg)
#define LOG_INFO(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::INFO_LEVEL, msg)
#define LOG_WARN(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::WARN_LEVEL, msg)
#define LOG_ERROR(msg) ::minidb::Logger::Instance().Log(::minidb::LogLevel::ERROR_LEVEL, msg)

}  // namespace minidb
