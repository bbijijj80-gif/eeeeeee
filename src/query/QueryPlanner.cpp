#include "QueryPlanner.h"

#include "../../include/macros.h"

namespace minidb {

std::unique_ptr<PlanNode> QueryPlanner::CreatePlan(const Statement& stmt, bool has_index_on_first_col,
                                                     const std::string& first_col_name) {
    switch (stmt.GetType()) {
        case StatementType::CREATE_TABLE: {
            const auto& s = static_cast<const CreateTableStatement&>(stmt);
            auto plan = std::make_unique<CreateTablePlan>();
            plan->table_name = s.table_name;
            plan->columns = s.columns;
            return plan;
        }
        case StatementType::INSERT: {
            const auto& s = static_cast<const InsertStatement&>(stmt);
            auto plan = std::make_unique<InsertPlan>();
            plan->table_name = s.table_name;
            plan->values = s.values;
            return plan;
        }
        case StatementType::SELECT: {
            const auto& s = static_cast<const SelectStatement&>(stmt);

            // Оптимизация: если условие WHERE — точное равенство по индексированному столбцу,
            // выбираем IndexScan вместо полного перебора страниц таблицы
            if (s.has_where && s.where.op == CompareOp::EQ && has_index_on_first_col &&
                s.where.column == first_col_name && s.where.value.type == ColumnType::INTEGER) {
                auto plan = std::make_unique<IndexScanPlan>();
                plan->table_name = s.table_name;
                plan->output_columns = s.columns;
                plan->lookup_key = s.where.value.int_val;
                return plan;
            }

            auto plan = std::make_unique<SeqScanPlan>();
            plan->table_name = s.table_name;
            plan->output_columns = s.columns;
            plan->has_filter = s.has_where;
            if (s.has_where) plan->filter = s.where;
            return plan;
        }
    }
    MINIDB_THROW("QueryPlanner: неизвестный тип SQL-выражения");
}

}  // namespace minidb
