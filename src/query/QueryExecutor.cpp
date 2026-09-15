#include "QueryExecutor.h"

#include "../../include/macros.h"
#include "../storage/Record.h"

namespace minidb {

QueryResult QueryExecutor::Execute(const PlanNode& plan) {
    switch (plan.GetType()) {
        case PlanType::CREATE_TABLE:
            return ExecuteCreateTable(static_cast<const CreateTablePlan&>(plan));
        case PlanType::INSERT:
            return ExecuteInsert(static_cast<const InsertPlan&>(plan));
        case PlanType::SEQ_SCAN:
            return ExecuteSeqScan(static_cast<const SeqScanPlan&>(plan));
        case PlanType::INDEX_SCAN:
            return ExecuteIndexScan(static_cast<const IndexScanPlan&>(plan));
    }
    MINIDB_THROW("QueryExecutor: неизвестный тип плана");
}

QueryResult QueryExecutor::ExecuteCreateTable(const CreateTablePlan& plan) {
    Schema schema;
    schema.columns = plan.columns;
    engine_->CreateTable(plan.table_name, schema);

    QueryResult result;
    result.message = "Таблица '" + plan.table_name + "' успешно создана";
    return result;
}

QueryResult QueryExecutor::ExecuteInsert(const InsertPlan& plan) {
    TableInfo* table = engine_->GetTable(plan.table_name);
    MINIDB_ASSERT(table != nullptr, "Таблица '" + plan.table_name + "' не найдена");
    MINIDB_ASSERT(plan.values.size() == table->schema.columns.size(),
                  "Число значений в INSERT не совпадает с числом столбцов таблицы");

    auto raw = Record::Serialize(plan.values, table->schema);
    rid_t rid = table->heap->InsertTuple(raw);

    if (table->has_index && !plan.values.empty() && plan.values[0].type == ColumnType::INTEGER) {
        table->index->Insert(plan.values[0].int_val, rid);
    }

    QueryResult result;
    result.message = "1 строка добавлена в таблицу '" + plan.table_name + "'";
    return result;
}

std::vector<std::string> QueryExecutor::ResolveOutputColumns(const std::vector<std::string>& requested,
                                                                const Schema& schema) {
    if (requested.empty() || requested[0] == "*") {
        std::vector<std::string> all;
        for (const auto& c : schema.columns) all.push_back(c.name);
        return all;
    }
    return requested;
}

bool QueryExecutor::MatchesFilter(const Tuple& tuple, const Schema& schema, const WhereClause& filter) {
    int idx = schema.IndexOf(filter.column);
    MINIDB_ASSERT(idx >= 0, "WHERE: столбец '" + filter.column + "' не найден в таблице");
    Value actual = tuple.GetValue(idx);

    int cmp;
    if (actual.type == ColumnType::INTEGER) {
        cmp = (actual.int_val < filter.value.int_val) ? -1 : (actual.int_val > filter.value.int_val ? 1 : 0);
    } else {
        cmp = actual.str_val.compare(filter.value.str_val);
        cmp = cmp < 0 ? -1 : (cmp > 0 ? 1 : 0);
    }

    switch (filter.op) {
        case CompareOp::EQ: return cmp == 0;
        case CompareOp::NE: return cmp != 0;
        case CompareOp::LT: return cmp < 0;
        case CompareOp::GT: return cmp > 0;
        case CompareOp::LE: return cmp <= 0;
        case CompareOp::GE: return cmp >= 0;
    }
    return false;
}

QueryResult QueryExecutor::ExecuteSeqScan(const SeqScanPlan& plan) {
    TableInfo* table = engine_->GetTable(plan.table_name);
    MINIDB_ASSERT(table != nullptr, "Таблица '" + plan.table_name + "' не найдена");

    QueryResult result;
    result.column_names = ResolveOutputColumns(plan.output_columns, table->schema);

    table->heap->Scan([&](rid_t rid, const std::vector<char>& raw) {
        auto values = Record::Deserialize(raw.data(), raw.size(), table->schema);
        Tuple tuple(rid, values);

        if (plan.has_filter && !MatchesFilter(tuple, table->schema, plan.filter)) return;

        std::vector<Value> out_row;
        for (const auto& col_name : result.column_names) {
            int idx = table->schema.IndexOf(col_name);
            MINIDB_ASSERT(idx >= 0, "SELECT: столбец '" + col_name + "' не найден");
            out_row.push_back(values[static_cast<size_t>(idx)]);
        }
        result.rows.push_back(std::move(out_row));
    });

    result.message = std::to_string(result.rows.size()) + " строк(и) найдено (полное сканирование)";
    return result;
}

QueryResult QueryExecutor::ExecuteIndexScan(const IndexScanPlan& plan) {
    TableInfo* table = engine_->GetTable(plan.table_name);
    MINIDB_ASSERT(table != nullptr, "Таблица '" + plan.table_name + "' не найдена");
    MINIDB_ASSERT(table->has_index, "Таблица '" + plan.table_name + "' не имеет индекса");

    QueryResult result;
    result.column_names = ResolveOutputColumns(plan.output_columns, table->schema);

    auto found_rid = table->index->Search(plan.lookup_key);
    if (found_rid.has_value()) {
        std::vector<char> raw;
        if (table->heap->GetTuple(found_rid.value(), &raw)) {
            auto values = Record::Deserialize(raw.data(), raw.size(), table->schema);
            std::vector<Value> out_row;
            for (const auto& col_name : result.column_names) {
                int idx = table->schema.IndexOf(col_name);
                out_row.push_back(values[static_cast<size_t>(idx)]);
            }
            result.rows.push_back(std::move(out_row));
        }
    }

    result.message = std::to_string(result.rows.size()) + " строк(и) найдено (поиск по B+ дереву)";
    return result;
}

}  // namespace minidb
