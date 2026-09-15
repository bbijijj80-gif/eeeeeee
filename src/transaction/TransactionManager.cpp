#include "TransactionManager.h"

#include "../recovery/WALManager.h"
#include "../utils/Logger.h"

namespace minidb {

TransactionManager::TransactionManager(LockManager* lock_manager, UndoLog* undo_log, WALManager* wal_manager)
    : lock_manager_(lock_manager), undo_log_(undo_log), wal_manager_(wal_manager) {}

Transaction* TransactionManager::Begin() {
    std::lock_guard<std::mutex> guard(mutex_);
    txn_id_t id = next_txn_id_.fetch_add(1);
    auto txn = std::make_unique<Transaction>();
    txn->id = id;
    txn->state = TxnState::GROWING;

    wal_manager_->AppendRecord(id, WALRecordType::BEGIN, "");

    Transaction* raw = txn.get();
    active_txns_[id] = std::move(txn);
    return raw;
}

void TransactionManager::Commit(Transaction* txn) {
    std::lock_guard<std::mutex> guard(mutex_);
    // Изменения уже применены к страницам буферного пула — просто фиксируем факт commit в WAL,
    // очищаем undo-журнал (откат больше не понадобится) и снимаем все блокировки транзакции
    wal_manager_->AppendRecord(txn->id, WALRecordType::COMMIT, "");
    undo_log_->Clear(txn->id);
    lock_manager_->ReleaseAll(txn->id);
    txn->state = TxnState::COMMITTED;
    active_txns_.erase(txn->id);
}

void TransactionManager::Abort(Transaction* txn) {
    std::lock_guard<std::mutex> guard(mutex_);
    // Откатываем изменения через UndoLog (применяем компенсирующие действия в обратном порядке)
    undo_log_->Rollback(txn->id);
    wal_manager_->AppendRecord(txn->id, WALRecordType::ABORT, "");
    lock_manager_->ReleaseAll(txn->id);
    txn->state = TxnState::ABORTED;
    active_txns_.erase(txn->id);
}

}  // namespace minidb
