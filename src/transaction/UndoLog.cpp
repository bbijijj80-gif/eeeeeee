#include "UndoLog.h"

namespace minidb {

void UndoLog::RecordUndo(txn_id_t txn_id, std::function<void()> undo_action) {
    log_[txn_id].push_back(std::move(undo_action));
}

void UndoLog::Rollback(txn_id_t txn_id) {
    auto it = log_.find(txn_id);
    if (it == log_.end()) return;

    // Применяем действия в обратном порядке (LIFO) — последняя операция отменяется первой
    for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
        (*rit)();
    }
    log_.erase(it);
}

void UndoLog::Clear(txn_id_t txn_id) {
    log_.erase(txn_id);
}

}  // namespace minidb
