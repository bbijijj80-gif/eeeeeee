#include "DatabaseEngine.h"

#include "../../include/macros.h"
#include "../query/QueryExecutor.h"
#include "../query/QueryPlanner.h"
#include "../query/SQLParser.h"
#include "../utils/Logger.h"

namespace minidb {

DatabaseEngine::DatabaseEngine(const std::string& db_file, const std::string& wal_file) {
    disk_manager_ = std::make_shared<DiskManager>(db_file);
    bpm_ = std::make_unique<BufferPoolManager>(BUFFER_POOL_SIZE, disk_manager_);
    wal_manager_ = std::make_unique<WALManager>(wal_file);
    checkpoint_ = std::make_unique<Checkpoint>(bpm_.get(), wal_manager_.get());

    lock_manager_ = std::make_unique<LockManager>();
    undo_log_ = std::make_unique<UndoLog>();
    txn_manager_ = std::make_unique<TransactionManager>(lock_manager_.get(), undo_log_.get(), wal_manager_.get());

    LOG_INFO("DatabaseEngine инициализирован: db=" + db_file + " wal=" + wal_file);
}

DatabaseEngine::~DatabaseEngine() {
    if (bpm_) bpm_->FlushAllPages();
}

TableInfo* DatabaseEngine::GetTable(const std::string& name) {
    auto it = catalog_.find(name);
    return it == catalog_.end() ? nullptr : it->second.get();
}

TableInfo* DatabaseEngine::CreateTable(const std::string& name, const Schema& schema) {
    MINIDB_ASSERT(catalog_.find(name) == catalog_.end(), "Таблица '" + name + "' уже существует");

    auto info = std::make_unique<TableInfo>();
    info->name = name;
    info->schema = schema;
    info->heap = std::make_unique<TableHeap>(bpm_.get(), INVALID_PAGE_ID);

    // Если первый столбец целочисленный — заводим по нему B+ дерево (аналог первичного ключа)
    if (!schema.columns.empty() && schema.columns[0].type == ColumnType::INTEGER) {
        info->index = std::make_unique<BPlusTree>(bpm_.get(), INVALID_PAGE_ID);
        info->has_index = true;
    }

    TableInfo* raw = info.get();
    catalog_[name] = std::move(info);
    return raw;
}

QueryResult DatabaseEngine::ExecuteSQL(const std::string& sql) {
    SQLParser parser(sql);
    auto stmt = parser.Parse();

    bool has_index = false;
    std::string first_col_name;
    if (stmt->GetType() == StatementType::SELECT) {
        const auto& select_stmt = static_cast<const SelectStatement&>(*stmt);
        TableInfo* table = GetTable(select_stmt.table_name);
        if (table != nullptr) {
            has_index = table->has_index;
            if (!table->schema.columns.empty()) first_col_name = table->schema.columns[0].name;
        }
    }

    QueryPlanner planner;
    auto plan = planner.CreatePlan(*stmt, has_index, first_col_name);

    QueryExecutor executor(this);
    return executor.Execute(*plan);
}

}  // namespace minidb
