// Record — необработанное байтовое представление строки на диске (сериализация Tuple).
#pragma once

#include <cstdint>
#include <vector>

#include "../../include/types.h"

namespace minidb {

class Record {
public:
    // Сериализует значения полей в плоский байтовый буфер согласно схеме
    static std::vector<char> Serialize(const std::vector<Value>& values, const Schema& schema);

    // Десериализует байтовый буфер обратно в значения полей согласно схеме
    static std::vector<Value> Deserialize(const char* data, size_t size, const Schema& schema);
};

}  // namespace minidb
