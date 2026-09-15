#include "TableHeap.h"

#include <cstring>

#include "../../include/config.h"
#include "../../include/macros.h"

namespace minidb {

namespace {
// RID кодируется как (page_id << 32) | slot_id, чтобы не заводить отдельную структуру
rid_t MakeRid(page_id_t page_id, int32_t slot_id) {
    return (static_cast<rid_t>(page_id) << 32) | static_cast<uint32_t>(slot_id);
}
page_id_t RidPageId(rid_t rid) { return static_cast<page_id_t>(rid >> 32); }
int32_t RidSlot(rid_t rid) { return static_cast<int32_t>(rid & 0xFFFFFFFF); }

int32_t ReadI32(const char* buf, size_t offset) {
    int32_t v;
    std::memcpy(&v, buf + offset, sizeof(int32_t));
    return v;
}
void WriteI32(char* buf, size_t offset, int32_t v) {
    std::memcpy(buf + offset, &v, sizeof(int32_t));
}
}  // namespace

TableHeap::TableHeap(BufferPoolManager* bpm, page_id_t first_page_id)
    : bpm_(bpm), first_page_id_(first_page_id), last_page_id_(first_page_id) {
    if (first_page_id_ == INVALID_PAGE_ID) {
        first_page_id_ = CreateNewPage(INVALID_PAGE_ID);
        last_page_id_ = first_page_id_;
    }
}

page_id_t TableHeap::CreateNewPage(page_id_t prev_page_id) {
    page_id_t new_id;
    Page* page = bpm_->NewPage(&new_id);
    char* data = page->GetData();
    WriteI32(data, 0, INVALID_PAGE_ID);              // next_page_id
    WriteI32(data, 4, 0);                             // num_slots
    WriteI32(data, 8, static_cast<int32_t>(PAGE_SIZE));  // free_space_ptr (растёт вниз от конца)
    bpm_->UnpinPage(new_id, true);

    if (prev_page_id != INVALID_PAGE_ID) {
        Page* prev = bpm_->FetchPage(prev_page_id);
        WriteI32(prev->GetData(), 0, new_id);
        bpm_->UnpinPage(prev_page_id, true);
    }
    return new_id;
}

rid_t TableHeap::InsertTuple(const std::vector<char>& serialized_data) {
    page_id_t page_id = last_page_id_;

    while (true) {
        Page* page = bpm_->FetchPage(page_id);
        char* data = page->GetData();
        int32_t num_slots = ReadI32(data, 4);
        int32_t free_ptr = ReadI32(data, 8);

        size_t needed = serialized_data.size() + SLOT_SIZE;
        size_t slot_dir_end = HEADER_SIZE + static_cast<size_t>(num_slots) * SLOT_SIZE;

        if (slot_dir_end + needed <= static_cast<size_t>(free_ptr)) {
            int32_t new_free_ptr = free_ptr - static_cast<int32_t>(serialized_data.size());
            std::memcpy(data + new_free_ptr, serialized_data.data(), serialized_data.size());

            size_t slot_offset = HEADER_SIZE + static_cast<size_t>(num_slots) * SLOT_SIZE;
            WriteI32(data, slot_offset, new_free_ptr);
            WriteI32(data, slot_offset + 4, static_cast<int32_t>(serialized_data.size()));

            WriteI32(data, 4, num_slots + 1);
            WriteI32(data, 8, new_free_ptr);

            bpm_->UnpinPage(page_id, true);
            return MakeRid(page_id, num_slots);
        }

        int32_t next_page_id = ReadI32(data, 0);
        bpm_->UnpinPage(page_id, false);

        if (next_page_id == INVALID_PAGE_ID) {
            next_page_id = CreateNewPage(page_id);
            last_page_id_ = next_page_id;
        }
        page_id = next_page_id;
    }
}

bool TableHeap::DeleteTuple(rid_t rid) {
    page_id_t page_id = RidPageId(rid);
    int32_t slot_id = RidSlot(rid);

    Page* page = bpm_->FetchPage(page_id);
    char* data = page->GetData();
    int32_t num_slots = ReadI32(data, 4);
    if (slot_id < 0 || slot_id >= num_slots) {
        bpm_->UnpinPage(page_id, false);
        return false;
    }
    size_t slot_offset = HEADER_SIZE + static_cast<size_t>(slot_id) * SLOT_SIZE;
    WriteI32(data, slot_offset + 4, -1);  // size = -1 помечает слот как удалённый
    bpm_->UnpinPage(page_id, true);
    return true;
}

bool TableHeap::GetTuple(rid_t rid, std::vector<char>* out_data) {
    page_id_t page_id = RidPageId(rid);
    int32_t slot_id = RidSlot(rid);

    Page* page = bpm_->FetchPage(page_id);
    char* data = page->GetData();
    int32_t num_slots = ReadI32(data, 4);
    if (slot_id < 0 || slot_id >= num_slots) {
        bpm_->UnpinPage(page_id, false);
        return false;
    }
    size_t slot_offset = HEADER_SIZE + static_cast<size_t>(slot_id) * SLOT_SIZE;
    int32_t tuple_offset = ReadI32(data, slot_offset);
    int32_t tuple_size = ReadI32(data, slot_offset + 4);
    if (tuple_size < 0) {
        bpm_->UnpinPage(page_id, false);
        return false;  // удалён
    }
    out_data->assign(data + tuple_offset, data + tuple_offset + tuple_size);
    bpm_->UnpinPage(page_id, false);
    return true;
}

void TableHeap::Scan(const std::function<void(rid_t, const std::vector<char>&)>& callback) {
    page_id_t page_id = first_page_id_;
    while (page_id != INVALID_PAGE_ID) {
        Page* page = bpm_->FetchPage(page_id);
        char* data = page->GetData();
        int32_t num_slots = ReadI32(data, 4);
        int32_t next_page_id = ReadI32(data, 0);

        for (int32_t slot = 0; slot < num_slots; ++slot) {
            size_t slot_offset = HEADER_SIZE + static_cast<size_t>(slot) * SLOT_SIZE;
            int32_t tuple_offset = ReadI32(data, slot_offset);
            int32_t tuple_size = ReadI32(data, slot_offset + 4);
            if (tuple_size < 0) continue;  // удалённая строка
            std::vector<char> raw(data + tuple_offset, data + tuple_offset + tuple_size);
            callback(MakeRid(page_id, slot), raw);
        }

        bpm_->UnpinPage(page_id, false);
        page_id = next_page_id;
    }
}

}  // namespace minidb
