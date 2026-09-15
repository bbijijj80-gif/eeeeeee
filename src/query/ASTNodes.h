// ASTNodes — узлы абстрактного синтаксического дерева, которое строит SQLParser
// и на основе которого QueryPlanner/QueryExecutor выполняют запрос.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../include/types.h"

namespace minidb {

enum class StatementType { CREATE_TABLE, INSERT, SELECT };
enum class CompareOp { EQ, LT, GT, LE, GE, NE };

struct WhereClause {
    std::string column;
    CompareOp op;
    Value value;
};

struct Statement {
    virtual ~Statement() = default;
    virtual StatementType GetType() const = 0;
};

struct CreateTableStatement : Statement {
    std::string table_name;
    std::vector<ColumnDef> columns;
    StatementType GetType() const override { return StatementType::CREATE_TABLE; }
};

struct InsertStatement : Statement {
    std::string table_name;
    std::vector<Value> values;
    StatementType GetType() const override { return StatementType::INSERT; }
};

struct SelectStatement : Statement {
    std::string table_name;
    std::vector<std::string> columns;  // пусто или {"*"} означает "все столбцы"
    bool has_where = false;
    WhereClause where;
    StatementType GetType() const override { return StatementType::SELECT; }
};

}  // namespace minidb
