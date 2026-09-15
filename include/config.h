// Глобальные константы конфигурации ядра
#pragma once

#include <cstddef>

namespace minidb {

constexpr size_t PAGE_SIZE = 4096;             // Размер страницы — 4KB, как в большинстве СУБД
constexpr size_t BUFFER_POOL_SIZE = 64;         // Количество фреймов в буферном пуле
constexpr size_t MAX_VARCHAR_LEN = 255;
constexpr const char* DEFAULT_DB_FILE = "minidb.db";
constexpr const char* DEFAULT_WAL_FILE = "minidb.wal";

}  // namespace minidb
