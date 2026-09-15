// LockManager — упрощённый менеджер блокировок с поддержкой Shared (S) и Exclusive (X) локов.
// Гранулярность — таблица целиком (table-level locking), чего достаточно для учебного ядра.
#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../../include/types.h"

namespace minidb {

enum class LockMode { SHARED, EXCLUSIVE };

class LockManager {
public:
    // Блокирует вызывающий поток, пока не удастся выдать блокировку (или бросает исключение
    // при обнаружении конфликта в неблокирующем режиме — см. NoWait ниже)
    bool LockShared(txn_id_t txn_id, const std::string& resource);
    bool LockExclusive(txn_id_t txn_id, const std::string& resource);

    // Освобождает все блокировки, удерживаемые транзакцией (вызывается при commit/abort)
    void ReleaseAll(txn_id_t txn_id);

private:
    struct LockState {
        std::unordered_set<txn_id_t> shared_holders;
        txn_id_t exclusive_holder = INVALID_TXN_ID;
    };

    std::mutex mutex_;
    std::condition_variable cv_;
    std::unordered_map<std::string, LockState> table_;
    std::unordered_map<txn_id_t, std::unordered_set<std::string>> held_by_txn_;
};

}  // namespace minidb
