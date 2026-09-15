// TableHeap — куча страниц (slotted pages), в которых физически хранятся строки таблицы.
// Каждая страница имеет заголовок (next_page_id, число слотов, указатель свободного места)
// и слотовый каталог, растущий от начала страницы, тогда как сами данные строк растут с конца.
#pragma once

#include <functional>
#include <memory>

#include "../core/BufferPoolManager.h"
#include "Tuple.h"

namespace minidb {

class TableHeap {
public:
    // first_page_id == INVALID_PAGE_ID означает "создать новую таблицу с нуля"
    TableHeap(BufferPoolManager* bpm, page_id_t first_page_id);

    // Вставляет кортеж, возвращая присвоенный ему RID; при нехватке места создаёт новую страницу
    rid_t InsertTuple(const std::vector<char>& serialized_data);

    // Помечает слот как удалённый (логическое удаление)
    bool DeleteTuple(rid_t rid);

    // Возвращает сырые байты кортежа по RID
    bool GetTuple(rid_t rid, std::vector<char>* out_data);

    // Обходит все живые кортежи таблицы, вызывая callback(rid, raw_bytes)
    void Scan(const std::function<void(rid_t, const std::vector<char>&)>& callback);

    page_id_t GetFirstPageId() const { return first_page_id_; }

private:
    static constexpr size_t HEADER_SIZE = 12;  // next_page_id(4) + num_slots(4) + free_ptr(4)
    static constexpr size_t SLOT_SIZE = 8;      // offset(4) + size(4)

    page_id_t CreateNewPage(page_id_t prev_page_id);

    BufferPoolManager* bpm_;
    page_id_t first_page_id_;
    page_id_t last_page_id_;
};

}  // namespace minidb
