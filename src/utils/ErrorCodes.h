// Коды ошибок ядра MiniDB
#pragma once

namespace minidb {

enum class ErrorCode {
    OK = 0,
    PAGE_NOT_FOUND,
    BUFFER_POOL_FULL,
    TABLE_NOT_FOUND,
    TABLE_ALREADY_EXISTS,
    COLUMN_NOT_FOUND,
    SYNTAX_ERROR,
    TYPE_MISMATCH,
    LOCK_CONFLICT,
    IO_ERROR,
    TRANSACTION_ABORTED
};

// Человекочитаемое описание кода ошибки
const char* ErrorCodeToString(ErrorCode code);

}  // namespace minidb
