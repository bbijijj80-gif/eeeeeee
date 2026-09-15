// Tuple — логическое представление одной строки таблицы в памяти.
#pragma once

#include <vector>

#include "../../include/types.h"

namespace minidb {

class Tuple {
public:
    Tuple() = default;
    Tuple(rid_t rid, std::vector<Value> values) : rid_(rid), values_(std::move(values)) {}

    rid_t GetRid() const { return rid_; }
    void SetRid(rid_t rid) { rid_ = rid; }

    const std::vector<Value>& GetValues() const { return values_; }
    Value GetValue(int column_idx) const { return values_.at(static_cast<size_t>(column_idx)); }

private:
    rid_t rid_ = 0;
    std::vector<Value> values_;
};

}  // namespace minidb
