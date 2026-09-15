#include "BufferPoolManager.h"

namespace minidb {

BufferPoolManager::BufferPoolManager(size_t pool_size, std::shared_ptr<DiskManager> disk_manager)
    : pool_size_(pool_size), disk_manager_(std::move(disk_manager)) {
    pages_.reserve(pool_size_);
    for (size_t i = 0; i < pool_size_; ++i) {
        pages_.push_back(std::make_unique<Page>());
        free_list_.push_back(static_cast<frame_id_t>(i));
    }
    replacer_ = std::make_unique<LRUReplacer>(pool_size_);
}

BufferPoolManager::~BufferPoolManager() {
    FlushAllPages();
}

Page* BufferPoolManager::GetFreeOrVictimFrame(frame_id_t* frame_id) {
    if (!free_list_.empty()) {
        *frame_id = free_list_.front();
        free_list_.pop_front();
        return pages_[*frame_id].get();
    }

    if (!replacer_->Victim(frame_id)) {
        return nullptr;  // Все страницы закреплены (pinned) — пул исчерпан
    }

    Page* victim_page = pages_[*frame_id].get();
    if (victim_page->IsDirty()) {
        disk_manager_->WritePage(victim_page->GetPageId(), victim_page->GetData());
    }
    page_table_.erase(victim_page->GetPageId());
    return victim_page;
}

Page* BufferPoolManager::FetchPage(page_id_t page_id) {
    std::lock_guard<std::mutex> guard(latch_);

    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
        Page* page = pages_[it->second].get();
        page->IncPin();
        replacer_->Pin(it->second);
        return page;
    }

    frame_id_t frame_id;
    Page* page = GetFreeOrVictimFrame(&frame_id);
    if (page == nullptr) {
        MINIDB_THROW("BufferPoolManager: буферный пул переполнен, нет доступных фреймов");
    }

    page->ResetMemory();
    disk_manager_->ReadPage(page_id, page->GetData());
    page->SetPageId(page_id);
    page->IncPin();

    page_table_[page_id] = frame_id;
    return page;
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
    std::lock_guard<std::mutex> guard(latch_);
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) return false;

    Page* page = pages_[it->second].get();
    if (is_dirty) page->SetDirty(true);

    page->DecPin();
    if (page->PinCount() == 0) {
        replacer_->Unpin(it->second);
    }
    return true;
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
    std::lock_guard<std::mutex> guard(latch_);
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) return false;

    Page* page = pages_[it->second].get();
    disk_manager_->WritePage(page_id, page->GetData());
    page->SetDirty(false);
    return true;
}

Page* BufferPoolManager::NewPage(page_id_t* out_page_id) {
    std::lock_guard<std::mutex> guard(latch_);

    frame_id_t frame_id;
    Page* page = GetFreeOrVictimFrame(&frame_id);
    if (page == nullptr) {
        MINIDB_THROW("BufferPoolManager: не удалось выделить страницу — пул переполнен");
    }

    page_id_t new_id = disk_manager_->AllocatePage();
    page->ResetMemory();
    page->SetPageId(new_id);
    page->IncPin();

    page_table_[new_id] = frame_id;
    *out_page_id = new_id;
    return page;
}

void BufferPoolManager::FlushAllPages() {
    std::lock_guard<std::mutex> guard(latch_);
    for (auto& [page_id, frame_id] : page_table_) {
        Page* page = pages_[frame_id].get();
        if (page->IsDirty()) {
            disk_manager_->WritePage(page_id, page->GetData());
            page->SetDirty(false);
        }
    }
}

}  // namespace minidb
