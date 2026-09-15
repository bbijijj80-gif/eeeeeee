// DiskManager отвечает исключительно за физический ввод-вывод страниц на диск.
// Он не знает ничего о содержимом страниц — только читает/пишет блоки по PAGE_SIZE байт.
#pragma once

#include <fstream>
#include <mutex>
#include <string>

#include "../../include/config.h"
#include "../../include/macros.h"
#include "../../include/types.h"

namespace minidb {

class DiskManager {
public:
    explicit DiskManager(const std::string& db_file);
    ~DiskManager();

    DISALLOW_COPY(DiskManager)

    // Читает страницу page_id в буфер размером PAGE_SIZE
    void ReadPage(page_id_t page_id, char* out_buffer);

    // Записывает страницу page_id из буфера на диск
    void WritePage(page_id_t page_id, const char* in_buffer);

    // Выделяет новый идентификатор страницы (аллокация в конец файла)
    page_id_t AllocatePage();

    size_t GetNumPages() const { return num_pages_; }

private:
    std::string file_name_;
    std::fstream db_io_;
    std::mutex io_mutex_;
    size_t num_pages_ = 0;
};

}  // namespace minidb
