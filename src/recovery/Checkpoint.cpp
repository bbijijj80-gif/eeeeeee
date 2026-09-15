#include "Checkpoint.h"

#include "../utils/Logger.h"

namespace minidb {

Checkpoint::Checkpoint(BufferPoolManager* bpm, WALManager* wal) : bpm_(bpm), wal_(wal) {}

void Checkpoint::DoCheckpoint() {
    // Особая псевдо-транзакция (-1) используется только как маркер в журнале
    wal_->AppendRecord(-1, WALRecordType::COMMIT, "CHECKPOINT_BEGIN");
    bpm_->FlushAllPages();
    wal_->AppendRecord(-1, WALRecordType::COMMIT, "CHECKPOINT_END");
    LOG_INFO("Checkpoint выполнен: все страницы сброшены на диск");
}

}  // namespace minidb
