// BPlusTree — дисковое B+ дерево для индексации по целочисленному ключу.
// Каждый узел занимает ровно одну страницу буферного пула. Порядок дерева фиксирован (ORDER),
// что упрощает логику split/redistribute, оставаясь при этом настоящей постраничной структурой.
//
// Упрощение: при удалении ключей не выполняется слияние (merge) недогруженных узлов —
// это стандартное учебное упрощение, не влияющее на корректность поиска и вставки.
#pragma once

#include <optional>
#include <vector>

#include "../core/BufferPoolManager.h"

namespace minidb {

class BPlusTree {
public:
    static constexpr int ORDER = 4;               // максимум детей у внутреннего узла
    static constexpr int MAX_KEYS = ORDER - 1;     // максимум ключей в узле

    BPlusTree(BufferPoolManager* bpm, page_id_t root_page_id);

    void Insert(int64_t key, rid_t value);
    std::optional<rid_t> Search(int64_t key);
    bool Remove(int64_t key);

    // Возвращает все пары (key, rid) в отсортированном порядке — обход всех листьев
    std::vector<std::pair<int64_t, rid_t>> TraverseAll();

    page_id_t GetRootPageId() const { return root_page_id_; }

private:
    enum class NodeType : int32_t { INTERNAL = 0, LEAF = 1 };

    // Смещения полей заголовка узла внутри страницы
    static constexpr size_t OFF_TYPE = 0;
    static constexpr size_t OFF_NUM_KEYS = 4;
    static constexpr size_t OFF_PARENT = 8;
    static constexpr size_t OFF_NEXT_LEAF = 12;
    static constexpr size_t OFF_KEYS = 16;
    static constexpr size_t OFF_VALUES_LEAF = OFF_KEYS + MAX_KEYS * sizeof(int64_t);
    static constexpr size_t OFF_CHILDREN = OFF_KEYS + MAX_KEYS * sizeof(int64_t);

    page_id_t CreateLeaf(page_id_t parent);
    page_id_t CreateInternal(page_id_t parent);

    page_id_t FindLeaf(int64_t key);
    void InsertIntoLeaf(page_id_t leaf_id, int64_t key, rid_t value);
    void SplitLeaf(page_id_t leaf_id);
    void InsertIntoParent(page_id_t left_id, int64_t middle_key, page_id_t right_id);
    void SplitInternal(page_id_t node_id);

    BufferPoolManager* bpm_;
    page_id_t root_page_id_;
};

}  // namespace minidb
