#include "DiskManager.h"

#include <cstring>

namespace minidb {

DiskManager::DiskManager(const std::string& db_file) : file_name_(db_file) {
    // Открываем файл для чтения/записи; если его нет — создаём
    db_io_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary);
    if (!db_io_.is_open()) {
        db_io_.clear();
        std::ofstream create(file_name_, std::ios::binary);
        create.close();
        db_io_.open(file_name_, std::ios::in | std::ios::out | std::ios::binary);
    }
    MINIDB_ASSERT(db_io_.is_open(), "DiskManager: не удалось открыть файл БД");

    db_io_.seekg(0, std::ios::end);
    auto size = static_cast<size_t>(db_io_.tellg());
    num_pages_ = size / PAGE_SIZE;
}

DiskManager::~DiskManager() {
    if (db_io_.is_open()) {
        db_io_.flush();
        db_io_.close();
    }
}

void DiskManager::ReadPage(page_id_t page_id, char* out_buffer) {
    std::lock_guard<std::mutex> guard(io_mutex_);
    auto offset = static_cast<std::streamoff>(page_id) * static_cast<std::streamoff>(PAGE_SIZE);

    db_io_.seekg(offset);
    if (db_io_.eof() || !db_io_.good()) {
        // Страница ещё не была записана — отдаём нулевой буфер (аналог "новой" страницы)
        std::memset(out_buffer, 0, PAGE_SIZE);
        db_io_.clear();
        return;
    }
    db_io_.read(out_buffer, static_cast<std::streamsize>(PAGE_SIZE));
    auto read_count = db_io_.gcount();
    if (read_count < static_cast<std::streamsize>(PAGE_SIZE)) {
        std::memset(out_buffer + read_count, 0, PAGE_SIZE - static_cast<size_t>(read_count));
    }
    db_io_.clear();
}

void DiskManager::WritePage(page_id_t page_id, const char* in_buffer) {
    std::lock_guard<std::mutex> guard(io_mutex_);
    auto offset = static_cast<std::streamoff>(page_id) * static_cast<std::streamoff>(PAGE_SIZE);

    db_io_.seekp(offset);
    db_io_.write(in_buffer, static_cast<std::streamsize>(PAGE_SIZE));
    if (db_io_.fail()) {
        MINIDB_THROW("DiskManager: ошибка записи страницы на диск");
    }
    db_io_.flush();
}

page_id_t DiskManager::AllocatePage() {
    std::lock_guard<std::mutex> guard(io_mutex_);
    return static_cast<page_id_t>(num_pages_++);
}

}  // namespace minidb
