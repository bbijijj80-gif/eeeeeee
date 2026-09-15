#include "ErrorCodes.h"

namespace minidb {

const char* ErrorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::OK: return "OK";
        case ErrorCode::PAGE_NOT_FOUND: return "Страница не найдена";
        case ErrorCode::BUFFER_POOL_FULL: return "Буферный пул переполнен";
        case ErrorCode::TABLE_NOT_FOUND: return "Таблица не найдена";
        case ErrorCode::TABLE_ALREADY_EXISTS: return "Таблица уже существует";
        case ErrorCode::COLUMN_NOT_FOUND: return "Столбец не найден";
        case ErrorCode::SYNTAX_ERROR: return "Синтаксическая ошибка SQL";
        case ErrorCode::TYPE_MISMATCH: return "Несоответствие типов";
        case ErrorCode::LOCK_CONFLICT: return "Конфликт блокировок";
        case ErrorCode::IO_ERROR: return "Ошибка ввода-вывода";
        case ErrorCode::TRANSACTION_ABORTED: return "Транзакция отменена";
    }
    return "Неизвестная ошибка";
}

}  // namespace minidb
