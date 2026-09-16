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
#include "utils/ConsoleUI.h"
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

}  // namespace

int main() {
    SetupConsoleEncoding();
    minidb::ui::EnableAnsiSupport();
    minidb::ui::PrintBanner();

    minidb::DatabaseEngine engine;

    std::string line;
    while (true) {
        minidb::ui::PrintPrompt();
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        try {
            auto result = engine.ExecuteSQL(line);
            minidb::ui::PrintTable(result);
            if (!result.message.empty()) minidb::ui::PrintSuccess(result.message);
        } catch (const minidb::MiniDBException& ex) {
            minidb::ui::PrintError(ex.what());
        } catch (const std::exception& ex) {
            minidb::ui::PrintError(std::string("непредвиденная ошибка: ") + ex.what());
        }
    }

    minidb::ui::PrintGoodbye();
    return 0;
}
