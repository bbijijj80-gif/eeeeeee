// Базовые типы, используемые во всех модулях MiniDB
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace minidb {

using page_id_t = int32_t;      // Идентификатор страницы на диске
using frame_id_t = int32_t;     // Идентификатор фрейма в буферном пуле
using txn_id_t = int64_t;       // Идентификатор транзакции
using lsn_t = int64_t;          // Log Sequence Number (для WAL)
using slot_id_t = int16_t;      // Номер слота внутри страницы
using rid_t = int64_t;          // Row ID (используется как ключ B+ дерева)

constexpr page_id_t INVALID_PAGE_ID = -1;
constexpr txn_id_t INVALID_TXN_ID = -1;
constexpr lsn_t INVALID_LSN = -1;

// Типы значений столбцов, поддерживаемые движком
enum class ColumnType : uint8_t {
    INTEGER,
    VARCHAR
};

// Универсальное значение ячейки таблицы (упрощённый вариант Value/Datum)
struct Value {
    ColumnType type = ColumnType::INTEGER;
    int64_t int_val = 0;
    std::string str_val;

    static Value MakeInt(int64_t v) {
        Value val;
        val.type = ColumnType::INTEGER;
        val.int_val = v;
        return val;
    }

    static Value MakeStr(std::string v) {
        Value val;
        val.type = ColumnType::VARCHAR;
        val.str_val = std::move(v);
        return val;
    }

    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        return type == ColumnType::INTEGER ? int_val == other.int_val
                                            : str_val == other.str_val;
    }

    std::string ToString() const {
        return type == ColumnType::INTEGER ? std::to_string(int_val) : str_val;
    }
};

struct ColumnDef {
    std::string name;
    ColumnType type;
};

struct Schema {
    std::vector<ColumnDef> columns;

    int IndexOf(const std::string& name) const {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == name) return static_cast<int>(i);
        }
        return -1;
    }
};

}  // namespace minidb
