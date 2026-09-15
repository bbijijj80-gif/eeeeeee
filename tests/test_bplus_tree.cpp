// Тесты для BPlusTree: вставка, поиск, удаление, обход
#include <cassert>
#include <iostream>
#include <memory>

#include "../src/core/BufferPoolManager.h"
#include "../src/core/DiskManager.h"
#include "../src/storage/BPlusTree.h"

using namespace minidb;

void TestInsertAndSearch() {
    auto disk = std::make_shared<DiskManager>("test_bplus.db");
    BufferPoolManager bpm(16, disk);
    BPlusTree tree(&bpm, INVALID_PAGE_ID);

    for (int64_t i = 0; i < 50; ++i) {
        tree.Insert(i, i * 100);
    }
    for (int64_t i = 0; i < 50; ++i) {
        auto found = tree.Search(i);
        assert(found.has_value());
        assert(found.value() == i * 100);
    }
    assert(!tree.Search(999).has_value());
    std::cout << "[OK] TestInsertAndSearch" << std::endl;
}

void TestDelete() {
    auto disk = std::make_shared<DiskManager>("test_bplus_delete.db");
    BufferPoolManager bpm(16, disk);
    BPlusTree tree(&bpm, INVALID_PAGE_ID);

    for (int64_t i = 0; i < 20; ++i) tree.Insert(i, i);
    assert(tree.Remove(5));
    assert(!tree.Search(5).has_value());
    assert(tree.Search(6).has_value());
    assert(!tree.Remove(999));
    std::cout << "[OK] TestDelete" << std::endl;
}

void TestTraverseAllIsSorted() {
    auto disk = std::make_shared<DiskManager>("test_bplus_traverse.db");
    BufferPoolManager bpm(16, disk);
    BPlusTree tree(&bpm, INVALID_PAGE_ID);

    for (int64_t i = 30; i >= 0; --i) tree.Insert(i, i);
    auto all = tree.TraverseAll();
    assert(all.size() == 31);
    for (size_t i = 1; i < all.size(); ++i) {
        assert(all[i - 1].first < all[i].first);
    }
    std::cout << "[OK] TestTraverseAllIsSorted" << std::endl;
}

int main() {
    TestInsertAndSearch();
    TestDelete();
    TestTraverseAllIsSorted();
    std::cout << "Все тесты BPlusTree пройдены успешно." << std::endl;
    return 0;
}
