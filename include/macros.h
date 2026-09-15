// Общие вспомогательные макросы
#pragma once

#include <stdexcept>
#include <string>

// Кастомное исключение ядра MiniDB — все ошибки уровня движка должны бросать его
namespace minidb {
class MiniDBException : public std::runtime_error {
public:
    explicit MiniDBException(const std::string& msg) : std::runtime_error(msg) {}
};
}  // namespace minidb

#define MINIDB_THROW(msg) throw ::minidb::MiniDBException(msg)

#define MINIDB_ASSERT(cond, msg)          \
    do {                                  \
        if (!(cond)) {                    \
            MINIDB_THROW(msg);            \
        }                                 \
    } while (0)

#define DISALLOW_COPY(ClassName)               \
    ClassName(const ClassName&) = delete;      \
    ClassName& operator=(const ClassName&) = delete;
