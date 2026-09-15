// Тесты для TransactionManager / LockManager / UndoLog
#include <cassert>
#include <iostream>

#include "../src/recovery/WALManager.h"
#include "../src/transaction/LockManager.h"
#include "../src/transaction/TransactionManager.h"
#include "../src/transaction/UndoLog.h"

using namespace minidb;

void TestBeginCommit() {
    WALManager wal("test_txn.wal");
    LockManager locks;
    UndoLog undo;
    TransactionManager manager(&locks, &undo, &wal);

    Transaction* txn = manager.Begin();
    assert(txn->state == TxnState::GROWING);
    manager.Commit(txn);
    assert(txn->state == TxnState::COMMITTED);
    std::cout << "[OK] TestBeginCommit" << std::endl;
}

void TestAbortRollsBackUndoLog() {
    WALManager wal("test_txn_abort.wal");
    LockManager locks;
    UndoLog undo;
    TransactionManager manager(&locks, &undo, &wal);

    Transaction* txn = manager.Begin();

    int value = 10;
    undo.RecordUndo(txn->id, [&value]() { value = 0; });  // компенсирующее действие
    value = 20;                                             // имитация "изменения" данных

    manager.Abort(txn);
    assert(txn->state == TxnState::ABORTED);
    assert(value == 0);  // откат должен был вернуть исходное значение
    std::cout << "[OK] TestAbortRollsBackUndoLog" << std::endl;
}

void TestLockManagerSharedAndExclusive() {
    LockManager locks;
    assert(locks.LockShared(1, "table_a"));
    assert(locks.LockShared(2, "table_a"));  // два S-лока совместимы
    locks.ReleaseAll(1);
    locks.ReleaseAll(2);

    assert(locks.LockExclusive(3, "table_b"));
    locks.ReleaseAll(3);
    std::cout << "[OK] TestLockManagerSharedAndExclusive" << std::endl;
}

int main() {
    TestBeginCommit();
    TestAbortRollsBackUndoLog();
    TestLockManagerSharedAndExclusive();
    std::cout << "Все тесты транзакций пройдены успешно." << std::endl;
    return 0;
}
