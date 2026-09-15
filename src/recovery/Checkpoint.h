// Checkpoint — периодически сбрасывает все "грязные" страницы буферного пула на диск
// и делает отметку в WAL, что сокращает объём журнала, который нужно переигрывать при restart recovery.
#pragma once

#include "../../include/types.h"
#include "../core/BufferPoolManager.h"
#include "WALManager.h"

namespace minidb {

class Checkpoint {
public:
    Checkpoint(BufferPoolManager* bpm, WALManager* wal);

    // Выполняет checkpoint: flush всех страниц + запись CHECKPOINT-маркера в WAL
    void DoCheckpoint();

private:
    BufferPoolManager* bpm_;
    WALManager* wal_;
};

}  // namespace minidb
