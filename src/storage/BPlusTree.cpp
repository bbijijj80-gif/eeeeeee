#include "BPlusTree.h"

#include <algorithm>
#include <cstring>

#include "../../include/macros.h"

namespace minidb {

namespace {
int32_t ReadI32(const char* buf, size_t off) { int32_t v; std::memcpy(&v, buf + off, 4); return v; }
void WriteI32(char* buf, size_t off, int32_t v) { std::memcpy(buf + off, &v, 4); }
int64_t ReadI64(const char* buf, size_t off) { int64_t v; std::memcpy(&v, buf + off, 8); return v; }
void WriteI64(char* buf, size_t off, int64_t v) { std::memcpy(buf + off, &v, 8); }
}  // namespace

BPlusTree::BPlusTree(BufferPoolManager* bpm, page_id_t root_page_id)
    : bpm_(bpm), root_page_id_(root_page_id) {
    if (root_page_id_ == INVALID_PAGE_ID) {
        root_page_id_ = CreateLeaf(INVALID_PAGE_ID);
    }
}

page_id_t BPlusTree::CreateLeaf(page_id_t parent) {
    page_id_t id;
    Page* page = bpm_->NewPage(&id);
    char* data = page->GetData();
    WriteI32(data, OFF_TYPE, static_cast<int32_t>(NodeType::LEAF));
    WriteI32(data, OFF_NUM_KEYS, 0);
    WriteI32(data, OFF_PARENT, parent);
    WriteI32(data, OFF_NEXT_LEAF, INVALID_PAGE_ID);
    bpm_->UnpinPage(id, true);
    return id;
}

page_id_t BPlusTree::CreateInternal(page_id_t parent) {
    page_id_t id;
    Page* page = bpm_->NewPage(&id);
    char* data = page->GetData();
    WriteI32(data, OFF_TYPE, static_cast<int32_t>(NodeType::INTERNAL));
    WriteI32(data, OFF_NUM_KEYS, 0);
    WriteI32(data, OFF_PARENT, parent);
    bpm_->UnpinPage(id, true);
    return id;
}

page_id_t BPlusTree::FindLeaf(int64_t key) {
    page_id_t current = root_page_id_;
    while (true) {
        Page* page = bpm_->FetchPage(current);
        char* data = page->GetData();
        auto type = static_cast<NodeType>(ReadI32(data, OFF_TYPE));
        if (type == NodeType::LEAF) {
            bpm_->UnpinPage(current, false);
            return current;
        }

        int32_t num_keys = ReadI32(data, OFF_NUM_KEYS);
        int i = 0;
        while (i < num_keys && key >= ReadI64(data, OFF_KEYS + i * sizeof(int64_t))) ++i;
        page_id_t child = ReadI32(data, OFF_CHILDREN + i * sizeof(int32_t));
        bpm_->UnpinPage(current, false);
        current = child;
    }
}

std::optional<rid_t> BPlusTree::Search(int64_t key) {
    page_id_t leaf_id = FindLeaf(key);
    Page* page = bpm_->FetchPage(leaf_id);
    char* data = page->GetData();
    int32_t num_keys = ReadI32(data, OFF_NUM_KEYS);

    std::optional<rid_t> result;
    for (int i = 0; i < num_keys; ++i) {
        if (ReadI64(data, OFF_KEYS + i * sizeof(int64_t)) == key) {
            result = ReadI64(data, OFF_VALUES_LEAF + i * sizeof(rid_t));
            break;
        }
    }
    bpm_->UnpinPage(leaf_id, false);
    return result;
}

void BPlusTree::Insert(int64_t key, rid_t value) {
    page_id_t leaf_id = FindLeaf(key);
    InsertIntoLeaf(leaf_id, key, value);

    Page* page = bpm_->FetchPage(leaf_id);
    int32_t num_keys = ReadI32(page->GetData(), OFF_NUM_KEYS);
    bpm_->UnpinPage(leaf_id, false);

    if (num_keys > MAX_KEYS) {
        SplitLeaf(leaf_id);
    }
}

void BPlusTree::InsertIntoLeaf(page_id_t leaf_id, int64_t key, rid_t value) {
    Page* page = bpm_->FetchPage(leaf_id);
    char* data = page->GetData();
    int32_t num_keys = ReadI32(data, OFF_NUM_KEYS);

    int pos = 0;
    while (pos < num_keys && ReadI64(data, OFF_KEYS + pos * sizeof(int64_t)) < key) ++pos;

    // Сдвигаем ключи/значения вправо, освобождая место для вставки
    for (int i = num_keys; i > pos; --i) {
        WriteI64(data, OFF_KEYS + i * sizeof(int64_t), ReadI64(data, OFF_KEYS + (i - 1) * sizeof(int64_t)));
        WriteI64(data, OFF_VALUES_LEAF + i * sizeof(rid_t), ReadI64(data, OFF_VALUES_LEAF + (i - 1) * sizeof(rid_t)));
    }
    WriteI64(data, OFF_KEYS + pos * sizeof(int64_t), key);
    WriteI64(data, OFF_VALUES_LEAF + pos * sizeof(rid_t), value);
    WriteI32(data, OFF_NUM_KEYS, num_keys + 1);

    bpm_->UnpinPage(leaf_id, true);
}

void BPlusTree::SplitLeaf(page_id_t leaf_id) {
    Page* left_page = bpm_->FetchPage(leaf_id);
    char* left = left_page->GetData();
    int32_t num_keys = ReadI32(left, OFF_NUM_KEYS);
    int32_t parent = ReadI32(left, OFF_PARENT);
    int32_t old_next = ReadI32(left, OFF_NEXT_LEAF);

    int mid = num_keys / 2;
    page_id_t right_id = CreateLeaf(parent);
    Page* right_page = bpm_->FetchPage(right_id);
    char* right = right_page->GetData();

    int right_count = num_keys - mid;
    for (int i = 0; i < right_count; ++i) {
        WriteI64(right, OFF_KEYS + i * sizeof(int64_t), ReadI64(left, OFF_KEYS + (mid + i) * sizeof(int64_t)));
        WriteI64(right, OFF_VALUES_LEAF + i * sizeof(rid_t), ReadI64(left, OFF_VALUES_LEAF + (mid + i) * sizeof(rid_t)));
    }
    WriteI32(right, OFF_NUM_KEYS, right_count);
    WriteI32(right, OFF_NEXT_LEAF, old_next);

    WriteI32(left, OFF_NUM_KEYS, mid);
    WriteI32(left, OFF_NEXT_LEAF, right_id);

    int64_t middle_key = ReadI64(right, OFF_KEYS);

    bpm_->UnpinPage(leaf_id, true);
    bpm_->UnpinPage(right_id, true);

    InsertIntoParent(leaf_id, middle_key, right_id);
}

void BPlusTree::InsertIntoParent(page_id_t left_id, int64_t middle_key, page_id_t right_id) {
    Page* left_page = bpm_->FetchPage(left_id);
    int32_t parent_id = ReadI32(left_page->GetData(), OFF_PARENT);
    bpm_->UnpinPage(left_id, false);

    if (parent_id == INVALID_PAGE_ID) {
        // Левый узел был корнем — создаём новый корень
        page_id_t new_root = CreateInternal(INVALID_PAGE_ID);
        Page* root_page = bpm_->FetchPage(new_root);
        char* data = root_page->GetData();
        WriteI64(data, OFF_KEYS, middle_key);
        WriteI32(data, OFF_CHILDREN, left_id);
        WriteI32(data, OFF_CHILDREN + sizeof(int32_t), right_id);
        WriteI32(data, OFF_NUM_KEYS, 1);
        bpm_->UnpinPage(new_root, true);

        root_page_id_ = new_root;

        Page* lp = bpm_->FetchPage(left_id);
        WriteI32(lp->GetData(), OFF_PARENT, new_root);
        bpm_->UnpinPage(left_id, true);
        Page* rp = bpm_->FetchPage(right_id);
        WriteI32(rp->GetData(), OFF_PARENT, new_root);
        bpm_->UnpinPage(right_id, true);
        return;
    }

    Page* parent_page = bpm_->FetchPage(parent_id);
    char* data = parent_page->GetData();
    int32_t num_keys = ReadI32(data, OFF_NUM_KEYS);

    int pos = 0;
    while (pos < num_keys && ReadI32(data, OFF_CHILDREN + pos * sizeof(int32_t)) != left_id) ++pos;

    for (int i = num_keys; i > pos; --i) {
        WriteI64(data, OFF_KEYS + i * sizeof(int64_t), ReadI64(data, OFF_KEYS + (i - 1) * sizeof(int64_t)));
    }
    for (int i = num_keys + 1; i > pos + 1; --i) {
        WriteI32(data, OFF_CHILDREN + i * sizeof(int32_t), ReadI32(data, OFF_CHILDREN + (i - 1) * sizeof(int32_t)));
    }
    WriteI64(data, OFF_KEYS + pos * sizeof(int64_t), middle_key);
    WriteI32(data, OFF_CHILDREN + (pos + 1) * sizeof(int32_t), right_id);
    WriteI32(data, OFF_NUM_KEYS, num_keys + 1);

    bpm_->UnpinPage(parent_id, true);

    Page* rp = bpm_->FetchPage(right_id);
    WriteI32(rp->GetData(), OFF_PARENT, parent_id);
    bpm_->UnpinPage(right_id, true);

    if (num_keys + 1 > MAX_KEYS) {
        SplitInternal(parent_id);
    }
}

void BPlusTree::SplitInternal(page_id_t node_id) {
    Page* left_page = bpm_->FetchPage(node_id);
    char* left = left_page->GetData();
    int32_t num_keys = ReadI32(left, OFF_NUM_KEYS);
    int32_t parent = ReadI32(left, OFF_PARENT);

    int mid = num_keys / 2;
    int64_t middle_key = ReadI64(left, OFF_KEYS + mid * sizeof(int64_t));

    page_id_t right_id = CreateInternal(parent);
    Page* right_page = bpm_->FetchPage(right_id);
    char* right = right_page->GetData();

    int right_count = num_keys - mid - 1;
    for (int i = 0; i < right_count; ++i) {
        WriteI64(right, OFF_KEYS + i * sizeof(int64_t), ReadI64(left, OFF_KEYS + (mid + 1 + i) * sizeof(int64_t)));
    }
    for (int i = 0; i <= right_count; ++i) {
        int32_t child = ReadI32(left, OFF_CHILDREN + (mid + 1 + i) * sizeof(int32_t));
        WriteI32(right, OFF_CHILDREN + i * sizeof(int32_t), child);
        Page* cp = bpm_->FetchPage(child);
        WriteI32(cp->GetData(), OFF_PARENT, right_id);
        bpm_->UnpinPage(child, true);
    }
    WriteI32(right, OFF_NUM_KEYS, right_count);
    WriteI32(left, OFF_NUM_KEYS, mid);

    bpm_->UnpinPage(node_id, true);
    bpm_->UnpinPage(right_id, true);

    InsertIntoParent(node_id, middle_key, right_id);
}

bool BPlusTree::Remove(int64_t key) {
    page_id_t leaf_id = FindLeaf(key);
    Page* page = bpm_->FetchPage(leaf_id);
    char* data = page->GetData();
    int32_t num_keys = ReadI32(data, OFF_NUM_KEYS);

    int pos = -1;
    for (int i = 0; i < num_keys; ++i) {
        if (ReadI64(data, OFF_KEYS + i * sizeof(int64_t)) == key) { pos = i; break; }
    }
    if (pos == -1) {
        bpm_->UnpinPage(leaf_id, false);
        return false;
    }

    for (int i = pos; i < num_keys - 1; ++i) {
        WriteI64(data, OFF_KEYS + i * sizeof(int64_t), ReadI64(data, OFF_KEYS + (i + 1) * sizeof(int64_t)));
        WriteI64(data, OFF_VALUES_LEAF + i * sizeof(rid_t), ReadI64(data, OFF_VALUES_LEAF + (i + 1) * sizeof(rid_t)));
    }
    WriteI32(data, OFF_NUM_KEYS, num_keys - 1);
    bpm_->UnpinPage(leaf_id, true);
    return true;
}

std::vector<std::pair<int64_t, rid_t>> BPlusTree::TraverseAll() {
    std::vector<std::pair<int64_t, rid_t>> result;

    // Спускаемся по самой левой ветке, чтобы найти первый (самый левый) лист
    page_id_t current = root_page_id_;
    while (true) {
        Page* page = bpm_->FetchPage(current);
        char* data = page->GetData();
        auto type = static_cast<NodeType>(ReadI32(data, OFF_TYPE));
        if (type == NodeType::LEAF) {
            bpm_->UnpinPage(current, false);
            break;
        }
        page_id_t child = ReadI32(data, OFF_CHILDREN);
        bpm_->UnpinPage(current, false);
        current = child;
    }

    // Идём по связному списку листьев вправо
    while (current != INVALID_PAGE_ID) {
        Page* page = bpm_->FetchPage(current);
        char* data = page->GetData();
        int32_t num_keys = ReadI32(data, OFF_NUM_KEYS);
        for (int i = 0; i < num_keys; ++i) {
            result.emplace_back(ReadI64(data, OFF_KEYS + i * sizeof(int64_t)),
                                 ReadI64(data, OFF_VALUES_LEAF + i * sizeof(rid_t)));
        }
        page_id_t next = ReadI32(data, OFF_NEXT_LEAF);
        bpm_->UnpinPage(current, false);
        current = next;
    }
    return result;
}

}  // namespace minidb
