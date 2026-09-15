#include "Record.h"

#include <cstring>

#include "../../include/macros.h"

namespace minidb {

// Формат записи: для каждой колонки — либо 8 байт int64, либо [4 байта длины][байты строки]
std::vector<char> Record::Serialize(const std::vector<Value>& values, const Schema& schema) {
    MINIDB_ASSERT(values.size() == schema.columns.size(), "Record: число значений не совпадает со схемой");

    std::vector<char> buffer;
    for (size_t i = 0; i < values.size(); ++i) {
        const Value& v = values[i];
        if (schema.columns[i].type == ColumnType::INTEGER) {
            char tmp[sizeof(int64_t)];
            std::memcpy(tmp, &v.int_val, sizeof(int64_t));
            buffer.insert(buffer.end(), tmp, tmp + sizeof(int64_t));
        } else {
            uint32_t len = static_cast<uint32_t>(v.str_val.size());
            char len_bytes[sizeof(uint32_t)];
            std::memcpy(len_bytes, &len, sizeof(uint32_t));
            buffer.insert(buffer.end(), len_bytes, len_bytes + sizeof(uint32_t));
            buffer.insert(buffer.end(), v.str_val.begin(), v.str_val.end());
        }
    }
    return buffer;
}

std::vector<Value> Record::Deserialize(const char* data, size_t size, const Schema& schema) {
    std::vector<Value> values;
    size_t offset = 0;
    for (const auto& col : schema.columns) {
        if (col.type == ColumnType::INTEGER) {
            MINIDB_ASSERT(offset + sizeof(int64_t) <= size, "Record: выход за границы буфера (int)");
            int64_t val;
            std::memcpy(&val, data + offset, sizeof(int64_t));
            offset += sizeof(int64_t);
            values.push_back(Value::MakeInt(val));
        } else {
            MINIDB_ASSERT(offset + sizeof(uint32_t) <= size, "Record: выход за границы буфера (len)");
            uint32_t len;
            std::memcpy(&len, data + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            MINIDB_ASSERT(offset + len <= size, "Record: выход за границы буфера (str)");
            std::string s(data + offset, len);
            offset += len;
            values.push_back(Value::MakeStr(std::move(s)));
        }
    }
    return values;
}

}  // namespace minidb
