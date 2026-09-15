// QueryPlanner — превращает AST в дерево физического плана выполнения.
// В этой мини-реализации план в основном "прозрачен" (1:1 к AST), но именно здесь
// принимается ключевое решение оптимизации: SeqScan против IndexScan для SELECT ... WHERE.
#pragma once

#include <memory>

#include "ASTNodes.h"

namespace minidb {

enum class PlanType { CREATE_TABLE, INSERT, SEQ_SCAN, INDEX_SCAN };

struct PlanNode {
    virtual ~PlanNode() = default;
    virtual PlanType GetType() const = 0;
};

struct CreateTablePlan : PlanNode {
    std::string table_name;
    std::vector<ColumnDef> columns;
    PlanType GetType() const override { return PlanType::CREATE_TABLE; }
};

struct InsertPlan : PlanNode {
    std::string table_name;
    std::vector<Value> values;
    PlanType GetType() const override { return PlanType::INSERT; }
};

// Полное сканирование всех строк таблицы с последующей фильтрацией (если есть WHERE)
struct SeqScanPlan : PlanNode {
    std::string table_name;
    std::vector<std::string> output_columns;
    bool has_filter = false;
    WhereClause filter;
    PlanType GetType() const override { return PlanType::SEQ_SCAN; }
};

// Точечный поиск по индексному столбцу таблицы (используется, когда WHERE col = value
// и col — индексированный столбец), что избегает полного сканирования кучи страниц
struct IndexScanPlan : PlanNode {
    std::string table_name;
    std::vector<std::string> output_columns;
    int64_t lookup_key = 0;
    PlanType GetType() const override { return PlanType::INDEX_SCAN; }
};

class QueryPlanner {
public:
    // has_index_on_first_col сообщает планировщику, есть ли у таблицы индекс по первому столбцу
    // (в реальной СУБД эта информация берётся из каталога, здесь передаётся явно)
    std::unique_ptr<PlanNode> CreatePlan(const Statement& stmt, bool has_index_on_first_col,
                                          const std::string& first_col_name);
};

}  // namespace minidb
