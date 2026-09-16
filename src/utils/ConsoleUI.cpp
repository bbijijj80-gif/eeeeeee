#include "ConsoleUI.h"

#include <algorithm>
#include <iostream>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace minidb::ui {

namespace {

// ANSI-коды цвета. Используем "яркие" варианты — они лучше читаются на тёмном фоне терминала.
constexpr const char* RESET = "\033[0m";
constexpr const char* BOLD = "\033[1m";
constexpr const char* DIM = "\033[2m";
constexpr const char* CYAN = "\033[96m";
constexpr const char* GREEN = "\033[92m";
constexpr const char* YELLOW = "\033[93m";
constexpr const char* RED = "\033[91m";
constexpr const char* BLUE = "\033[94m";
constexpr const char* MAGENTA = "\033[95m";

// Считает длину строки в отображаемых символах (code points), а не в байтах —
// иначе кириллица (2 байта на символ в UTF-8) сломала бы выравнивание таблицы.
size_t DisplayWidth(const std::string& s) {
    size_t width = 0;
    for (unsigned char c : s) {
        // Байты-продолжения UTF-8 (10xxxxxx) не считаются отдельными символами
        if ((c & 0xC0) != 0x80) ++width;
    }
    return width;
}

std::string PadRight(const std::string& s, size_t target_width) {
    size_t width = DisplayWidth(s);
    if (width >= target_width) return s;
    return s + std::string(target_width - width, ' ');
}

}  // namespace

void EnableAnsiSupport() {
#ifdef _WIN32
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) return;

    DWORD mode = 0;
    if (!GetConsoleMode(handle, &mode)) return;
    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void PrintBanner() {
    const std::string rule(60, '=');
    std::cout << CYAN << BOLD << rule << RESET << "\n";
    std::cout << "   " << MAGENTA << BOLD << "MiniDB" << RESET << DIM
               << "  —  учебное ядро реляционной СУБД" << RESET << "\n";
    std::cout << CYAN << BOLD << rule << RESET << "\n";
    std::cout << DIM << "Команды: " << RESET << GREEN << "CREATE TABLE" << RESET << ", " << GREEN << "INSERT INTO" << RESET
               << ", " << GREEN << "SELECT ... [WHERE]" << RESET << ", " << YELLOW << "exit" << RESET << "\n\n";
}

void PrintPrompt() {
    std::cout << BOLD << BLUE << "minidb" << RESET << DIM << ">" << RESET << " ";
}

void PrintSuccess(const std::string& message) {
    std::cout << GREEN << "✓ " << RESET << message << std::endl;
}

void PrintError(const std::string& message) {
    std::cerr << RED << BOLD << "✗ Ошибка: " << RESET << RED << message << RESET << std::endl;
}

void PrintInfo(const std::string& message) {
    std::cout << DIM << message << RESET << std::endl;
}

void PrintTable(const QueryResult& result) {
    if (result.column_names.empty()) return;

    size_t cols = result.column_names.size();
    std::vector<size_t> widths(cols);
    for (size_t i = 0; i < cols; ++i) widths[i] = DisplayWidth(result.column_names[i]);
    for (const auto& row : result.rows) {
        for (size_t i = 0; i < cols; ++i) {
            widths[i] = std::max(widths[i], DisplayWidth(row[i].ToString()));
        }
    }

    auto print_border = [&](const char* left, const char* mid, const char* right, const char* fill) {
        std::cout << DIM << left;
        for (size_t i = 0; i < cols; ++i) {
            for (size_t j = 0; j < widths[i] + 2; ++j) std::cout << fill;
            std::cout << (i + 1 < cols ? mid : right);
        }
        std::cout << RESET << "\n";
    };

    print_border("┌", "┬", "┐", "─");

    std::cout << DIM << "│" << RESET;
    for (size_t i = 0; i < cols; ++i) {
        std::cout << " " << BOLD << CYAN << PadRight(result.column_names[i], widths[i]) << RESET << " " << DIM << "│" << RESET;
    }
    std::cout << "\n";

    print_border("├", "┼", "┤", "─");

    for (const auto& row : result.rows) {
        std::cout << DIM << "│" << RESET;
        for (size_t i = 0; i < cols; ++i) {
            std::cout << " " << PadRight(row[i].ToString(), widths[i]) << " " << DIM << "│" << RESET;
        }
        std::cout << "\n";
    }

    print_border("└", "┴", "┘", "─");
}

void PrintGoodbye() {
    std::cout << CYAN << "Завершение работы MiniDB. До встречи!" << RESET << std::endl;
}

}  // namespace minidb::ui
