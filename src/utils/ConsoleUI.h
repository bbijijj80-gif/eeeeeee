// ConsoleUI — оформление текстового интерфейса REPL: цвета ANSI, баннер и таблицы
// для вывода результатов SELECT. Инкапсулирует все "красивости" консоли в одном месте,
// чтобы main.cpp оставался простым REPL-циклом.
#pragma once

#include <string>

#include "../core/DatabaseEngine.h"

namespace minidb::ui {

// Включает поддержку ANSI-последовательностей в консоли Windows (ENABLE_VIRTUAL_TERMINAL_PROCESSING).
// На Linux/macOS терминалы поддерживают ANSI по умолчанию, функция там не делает ничего.
void EnableAnsiSupport();

void PrintBanner();
void PrintPrompt();

void PrintSuccess(const std::string& message);
void PrintError(const std::string& message);
void PrintInfo(const std::string& message);

// Печатает результат SELECT как аккуратную таблицу с рамкой из псевдографики
void PrintTable(const QueryResult& result);

void PrintGoodbye();

}  // namespace minidb::ui
