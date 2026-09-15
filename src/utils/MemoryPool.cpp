#include "MemoryPool.h"

namespace minidb {

MemoryPool::MemoryPool(size_t block_size, size_t block_count)
    : block_size_(block_size), block_count_(block_count), used_(block_count, false) {
    storage_ = new char[block_size_ * block_count_];
}

MemoryPool::~MemoryPool() {
    delete[] storage_;
}

void* MemoryPool::Allocate() {
    for (size_t i = 0; i < block_count_; ++i) {
        if (!used_[i]) {
            used_[i] = true;
            return storage_ + i * block_size_;
        }
    }
    MINIDB_THROW("MemoryPool: нет свободных блоков");
}

void MemoryPool::Deallocate(void* ptr) {
    auto offset = static_cast<char*>(ptr) - storage_;
    size_t index = static_cast<size_t>(offset) / block_size_;
    MINIDB_ASSERT(index < block_count_, "MemoryPool: указатель не принадлежит пулу");
    used_[index] = false;
}

}  // namespace minidb
