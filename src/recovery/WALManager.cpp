#include "WALManager.h"

#include "../../include/macros.h"

namespace minidb {

namespace {
const char* TypeToStr(WALRecordType type) {
    switch (type) {
        case WALRecordType::BEGIN: return "BEGIN";
        case WALRecordType::COMMIT: return "COMMIT";
        case WALRecordType::ABORT: return "ABORT";
        case WALRecordType::INSERT: return "INSERT";
        case WALRecordType::DELETE: return "DELETE";
        case WALRecordType::UPDATE: return "UPDATE";
    }
    return "UNKNOWN";
}
}  // namespace

WALManager::WALManager(const std::string& wal_file) {
    // Открываем в режиме дозаписи (append) — журнал никогда не перезаписывается,
    // только дополняется, что является ключевым свойством WAL
    out_.open(wal_file, std::ios::out | std::ios::app);
    MINIDB_ASSERT(out_.is_open(), "WALManager: не удалось открыть файл журнала");
}

WALManager::~WALManager() {
    Flush();
    if (out_.is_open()) out_.close();
}

std::string WALManager::ToLine(const WALRecord& record) const {
    return std::to_string(record.lsn) + "|" + std::to_string(record.txn_id) + "|" +
           TypeToStr(record.type) + "|" + record.payload;
}

lsn_t WALManager::AppendRecord(txn_id_t txn_id, WALRecordType type, const std::string& payload) {
    std::lock_guard<std::mutex> guard(mutex_);
    lsn_t lsn = next_lsn_.fetch_add(1);

    WALRecord record{lsn, txn_id, type, payload};
    out_ << ToLine(record) << "\n";
    out_.flush();  // WAL-гарантия: запись должна попасть на диск ДО подтверждения операции
    return lsn;
}

void WALManager::Flush() {
    std::lock_guard<std::mutex> guard(mutex_);
    out_.flush();
}

}  // namespace minidb
