// BufferPoolManager — центральный кэш страниц. Хранит N фреймов в RAM,
// сопоставляет page_id -> frame_id и вытесняет "холодные" страницы по LRU,
// сбрасывая их на диск через DiskManager, если они "грязные" (dirty).
#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "../../include/config.h"
#include "../../include/macros.h"
#include "../storage/Page.h"
#include "DiskManager.h"
#include "LRUReplacer.h"

namespace minidb {

class BufferPoolManager {
public:
    BufferPoolManager(size_t pool_size, std::shared_ptr<DiskManager> disk_manager);
    ~BufferPoolManager();

    DISALLOW_COPY(BufferPoolManager)

    // Возвращает страницу из кэша либо подгружает её с диска, увеличивая pin_count
    Page* FetchPage(page_id_t page_id);

    // Уменьшает pin_count страницы; is_dirty говорит, была ли страница изменена вызывающим кодом
    bool UnpinPage(page_id_t page_id, bool is_dirty);

    // Принудительно сбрасывает страницу на диск
    bool FlushPage(page_id_t page_id);

    // Создаёт новую страницу, выделяя место на диске и фрейм в пуле
    Page* NewPage(page_id_t* out_page_id);

    // Сбрасывает все страницы буферного пула на диск (используется при checkpoint)
    void FlushAllPages();

private:
    Page* GetFreeOrVictimFrame(frame_id_t* frame_id);

    size_t pool_size_;
    std::shared_ptr<DiskManager> disk_manager_;
    std::vector<std::unique_ptr<Page>> pages_;
    std::unordered_map<page_id_t, frame_id_t> page_table_;
    std::list<frame_id_t> free_list_;
    std::unique_ptr<LRUReplacer> replacer_;
    std::mutex latch_;
};

}  // namespace minidb
