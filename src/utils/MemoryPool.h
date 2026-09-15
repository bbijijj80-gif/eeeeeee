// Простой пул памяти фиксированного размера блока (для страниц буферного пула)
#pragma once

#include <cstddef>
#include <vector>

#include "../../include/macros.h"

namespace minidb {

class MemoryPool {
public:
    MemoryPool(size_t block_size, size_t block_count);
    ~MemoryPool();

    DISALLOW_COPY(MemoryPool)

    // Выдаёт указатель на свободный блок либо бросает исключение, если пул пуст
    void* Allocate();

    // Возвращает блок обратно в пул
    void Deallocate(void* ptr);

    size_t BlockSize() const { return block_size_; }

private:
    size_t block_size_;
    size_t block_count_;
    char* storage_;
    std::vector<bool> used_;
};

}  // namespace minidb
