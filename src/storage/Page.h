// Page — представление одной страницы (4KB) в оперативной памяти буферного пула.
#pragma once

#include <cstring>
#include <shared_mutex>

#include "../../include/config.h"
#include "../../include/types.h"

namespace minidb {

class Page {
public:
    Page() { ResetMemory(); }

    char* GetData() { return data_; }
    const char* GetData() const { return data_; }

    page_id_t GetPageId() const { return page_id_; }
    void SetPageId(page_id_t id) { page_id_ = id; }

    bool IsDirty() const { return is_dirty_; }
    void SetDirty(bool dirty) { is_dirty_ = dirty; }

    int PinCount() const { return pin_count_; }
    void IncPin() { ++pin_count_; }
    void DecPin() { if (pin_count_ > 0) --pin_count_; }

    void ResetMemory() {
        std::memset(data_, 0, PAGE_SIZE);
        page_id_ = INVALID_PAGE_ID;
        is_dirty_ = false;
        pin_count_ = 0;
    }

    std::shared_mutex& Latch() { return latch_; }

private:
    char data_[PAGE_SIZE]{};
    page_id_t page_id_ = INVALID_PAGE_ID;
    bool is_dirty_ = false;
    int pin_count_ = 0;
    std::shared_mutex latch_;  // Латч страницы (RAII-блокировка на уровне страницы)
};

}  // namespace minidb
