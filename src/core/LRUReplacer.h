// LRU-replacer выбирает "жертву" — фрейм для вытеснения из буферного пула.
// Реализация — классический список последних использований (не K-LRU, но легко расширяется).
#pragma once

#include <list>
#include <mutex>
#include <unordered_map>

#include "../../include/types.h"

namespace minidb {

class LRUReplacer {
public:
    explicit LRUReplacer(size_t num_frames) : capacity_(num_frames) {}

    // Помечает фрейм как доступный для вытеснения (unpin)
    void Unpin(frame_id_t frame_id) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (index_.count(frame_id)) return;
        lru_list_.push_front(frame_id);
        index_[frame_id] = lru_list_.begin();
    }

    // Помечает фрейм как используемый (pin) — убирает из списка кандидатов на вытеснение
    void Pin(frame_id_t frame_id) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it = index_.find(frame_id);
        if (it != index_.end()) {
            lru_list_.erase(it->second);
            index_.erase(it);
        }
    }

    // Возвращает наименее недавно использованный фрейм, если такой есть
    bool Victim(frame_id_t* frame_id) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (lru_list_.empty()) return false;
        *frame_id = lru_list_.back();
        lru_list_.pop_back();
        index_.erase(*frame_id);
        return true;
    }

    size_t Size() const {
        std::lock_guard<std::mutex> guard(mutex_);
        return lru_list_.size();
    }

private:
    size_t capacity_;
    mutable std::mutex mutex_;
    std::list<frame_id_t> lru_list_;
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> index_;
};

}  // namespace minidb
