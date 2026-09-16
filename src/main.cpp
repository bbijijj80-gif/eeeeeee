// MiniDB — точка входа: интерактивный REPL для выполнения SQL-команд.
#include <iostream>
#include <string>

#ifdef _WIN32
// Отключаем избыточную часть Win32 API и макросы min/max, которые иначе
// конфликтуют с std::min/std::max и другими идентификаторами проекта.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include "../include/macros.h"
#include "core/DatabaseEngine.h"
#include "utils/Logger.h"

namespace {

// Исходники в UTF-8, но консоль Windows по умолчанию использует кодовую
// страницу OEM (866 для русской локали), из-за чего кириллица превращается
// в нечитаемые символы. Переключаем консоль на UTF-8 при старте программы.
void SetupConsoleEncoding() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void PrintResult(const minidb::QueryResult& result) {
    if (!result.column_names.empty()) {
        for (const auto& name : result.column_names) std::cout << name << "\t";
        std::cout << "\n";
        for (const auto& row : result.rows) {
            for (const auto& value : row) std::cout << value.ToString() << "\t";
            std::cout << "\n";
        }
    }
    if (!result.message.empty()) {
        std::cout << result.message << std::endl;
    }
}

}  // namespace

int main() {
    SetupConsoleEncoding();

    std::cout << "=== MiniDB — учебное ядро реляционной СУБД ===" << std::endl;
    std::cout << "Поддерживаемые команды: CREATE TABLE, INSERT INTO, SELECT ... [WHERE], exit" << std::endl;

    minidb::DatabaseEngine engine;

    std::string line;
    while (true) {
        std::cout << "minidb> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        try {
            auto result = engine.ExecuteSQL(line);
            PrintResult(result);
        } catch (const minidb::MiniDBException& ex) {
            std::cerr << "Ошибка: " << ex.what() << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "Непредвиденная ошибка: " << ex.what() << std::endl;
        }
    }

    std::cout << "Завершение работы MiniDB." << std::endl;
    return 0;
}
