// DatabaseEngine — фасад, связывающий воедино все ядра: хранение, транзакции,
// восстановление и обработку запросов. Хранит каталог таблиц (Catalog) и является
// точкой входа для выполнения SQL-команд.
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../../include/types.h"
#include "../recovery/Checkpoint.h"
#include "../recovery/WALManager.h"
#include "../storage/BPlusTree.h"
#include "../storage/TableHeap.h"
#include "../transaction/LockManager.h"
#include "../transaction/TransactionManager.h"
#include "../transaction/UndoLog.h"
#include "BufferPoolManager.h"
#include "DiskManager.h"

namespace minidb {

// Метаданные одной таблицы в каталоге базы данных
struct TableInfo {
    std::string name;
    Schema schema;
    std::unique_ptr<TableHeap> heap;
    std::unique_ptr<BPlusTree> index;  // индекс по первому столбцу, если он INTEGER
    bool has_index = false;
    int64_t next_row_id = 1;
};

// Результат выполнения запроса, который возвращается пользователю (например, в main.cpp REPL)
struct QueryResult {
    std::vector<std::string> column_names;
    std::vector<std::vector<Value>> rows;
    std::string message;  // например, "Таблица создана", "1 строка добавлена"
};

class DatabaseEngine {
public:
    explicit DatabaseEngine(const std::string& db_file = DEFAULT_DB_FILE,
                             const std::string& wal_file = DEFAULT_WAL_FILE);
    ~DatabaseEngine();

    // Выполняет одну SQL-команду от начала до конца (парсинг -> план -> исполнение)
    QueryResult ExecuteSQL(const std::string& sql);

    TableInfo* GetTable(const std::string& name);
    TableInfo* CreateTable(const std::string& name, const Schema& schema);

    BufferPoolManager* GetBufferPoolManager() { return bpm_.get(); }
    TransactionManager* GetTransactionManager() { return txn_manager_.get(); }
    WALManager* GetWALManager() { return wal_manager_.get(); }
    Checkpoint* GetCheckpoint() { return checkpoint_.get(); }

private:
    std::shared_ptr<DiskManager> disk_manager_;
    std::unique_ptr<BufferPoolManager> bpm_;
    std::unique_ptr<WALManager> wal_manager_;
    std::unique_ptr<Checkpoint> checkpoint_;
    std::unique_ptr<LockManager> lock_manager_;
    std::unique_ptr<UndoLog> undo_log_;
    std::unique_ptr<TransactionManager> txn_manager_;

    std::unordered_map<std::string, std::unique_ptr<TableInfo>> catalog_;
};

}  // namespace minidb
