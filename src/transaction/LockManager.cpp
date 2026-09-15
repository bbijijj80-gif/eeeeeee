#include "LockManager.h"

namespace minidb {

bool LockManager::LockShared(txn_id_t txn_id, const std::string& resource) {
    std::unique_lock<std::mutex> guard(mutex_);
    auto& state = table_[resource];

    // Ждём, пока ресурс не освободится от несовместимого X-лока другой транзакции
    cv_.wait(guard, [&] {
        return state.exclusive_holder == INVALID_TXN_ID || state.exclusive_holder == txn_id;
    });

    state.shared_holders.insert(txn_id);
    held_by_txn_[txn_id].insert(resource);
    return true;
}

bool LockManager::LockExclusive(txn_id_t txn_id, const std::string& resource) {
    std::unique_lock<std::mutex> guard(mutex_);
    auto& state = table_[resource];

    // X-лок совместим только с блокировками этой же транзакции
    cv_.wait(guard, [&] {
        bool only_self_shared = state.shared_holders.empty() ||
                                 (state.shared_holders.size() == 1 && state.shared_holders.count(txn_id));
        bool no_foreign_exclusive = state.exclusive_holder == INVALID_TXN_ID || state.exclusive_holder == txn_id;
        return only_self_shared && no_foreign_exclusive;
    });

    state.exclusive_holder = txn_id;
    held_by_txn_[txn_id].insert(resource);
    return true;
}

void LockManager::ReleaseAll(txn_id_t txn_id) {
    std::unique_lock<std::mutex> guard(mutex_);
    auto it = held_by_txn_.find(txn_id);
    if (it == held_by_txn_.end()) return;

    for (const auto& resource : it->second) {
        auto& state = table_[resource];
        state.shared_holders.erase(txn_id);
        if (state.exclusive_holder == txn_id) {
            state.exclusive_holder = INVALID_TXN_ID;
        }
    }
    held_by_txn_.erase(it);
    cv_.notify_all();
}

}  // namespace minidb
