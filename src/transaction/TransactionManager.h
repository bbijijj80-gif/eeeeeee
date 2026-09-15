// TransactionManager — управляет жизненным циклом транзакций (Begin/Commit/Abort)
// и обеспечивает свойства ACID совместно с LockManager, UndoLog и WALManager.
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "../../include/types.h"
#include "LockManager.h"
#include "UndoLog.h"

namespace minidb {

class WALManager;  // forward declaration — избегаем циклической зависимости заголовков

enum class TxnState { GROWING, SHRINKING, COMMITTED, ABORTED };

struct Transaction {
    txn_id_t id;
    TxnState state = TxnState::GROWING;
};

class TransactionManager {
public:
    TransactionManager(LockManager* lock_manager, UndoLog* undo_log, WALManager* wal_manager);

    Transaction* Begin();
    void Commit(Transaction* txn);
    void Abort(Transaction* txn);

private:
    std::atomic<txn_id_t> next_txn_id_{0};
    LockManager* lock_manager_;
    UndoLog* undo_log_;
    WALManager* wal_manager_;

    std::mutex mutex_;
    std::unordered_map<txn_id_t, std::unique_ptr<Transaction>> active_txns_;
};

}  // namespace minidb
