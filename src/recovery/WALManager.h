// WALManager — реализация Write-Ahead Logging: любое изменение данных сперва
// фиксируется в журнале на диске (append-only), и только потом применяется к страницам.
// Это гарантирует durability (D в ACID): после сбоя журнал можно "переиграть" (redo).
#pragma once

#include <atomic>
#include <fstream>
#include <mutex>
#include <string>

#include "../../include/types.h"

namespace minidb {

enum class WALRecordType { BEGIN, COMMIT, ABORT, INSERT, DELETE, UPDATE };

struct WALRecord {
    lsn_t lsn;
    txn_id_t txn_id;
    WALRecordType type;
    std::string payload;  // сериализованные данные операции (таблица, rid, значения и т.п.)
};

class WALManager {
public:
    explicit WALManager(const std::string& wal_file);
    ~WALManager();

    // Дописывает запись в конец журнала и возвращает её LSN. Вызывается ДО применения
    // изменения к странице данных — отсюда и название "Write-Ahead"
    lsn_t AppendRecord(txn_id_t txn_id, WALRecordType type, const std::string& payload);

    // Принудительно сбрасывает журнал на диск (fsync-подобная гарантия)
    void Flush();

private:
    std::string ToLine(const WALRecord& record) const;

    std::ofstream out_;
    std::mutex mutex_;
    std::atomic<lsn_t> next_lsn_{0};
};

}  // namespace minidb
