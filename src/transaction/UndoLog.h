// UndoLog — журнал отмены (компенсирующих действий) для реализации ROLLBACK.
// Хранит для каждой транзакции список обратных операций в порядке LIFO.
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "../../include/types.h"

namespace minidb {

class UndoLog {
public:
    // Регистрирует компенсирующее действие (замыкание, которое откатывает одну операцию)
    void RecordUndo(txn_id_t txn_id, std::function<void()> undo_action);

    // Применяет все компенсирующие действия транзакции в обратном порядке и очищает журнал
    void Rollback(txn_id_t txn_id);

    // Очищает журнал транзакции без применения (используется при успешном commit)
    void Clear(txn_id_t txn_id);

private:
    std::unordered_map<txn_id_t, std::vector<std::function<void()>>> log_;
};

}  // namespace minidb
