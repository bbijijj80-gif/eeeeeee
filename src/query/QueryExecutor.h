// QueryExecutor — исполняет физический план (PlanNode), обращаясь к TableHeap/BPlusTree
// через каталог DatabaseEngine, и формирует QueryResult для пользователя.
#pragma once

#include "../core/DatabaseEngine.h"
#include "QueryPlanner.h"

namespace minidb {

class QueryExecutor {
public:
    explicit QueryExecutor(DatabaseEngine* engine) : engine_(engine) {}

    QueryResult Execute(const PlanNode& plan);

private:
    QueryResult ExecuteCreateTable(const CreateTablePlan& plan);
    QueryResult ExecuteInsert(const InsertPlan& plan);
    QueryResult ExecuteSeqScan(const SeqScanPlan& plan);
    QueryResult ExecuteIndexScan(const IndexScanPlan& plan);

    static bool MatchesFilter(const Tuple& tuple, const Schema& schema, const WhereClause& filter);
    static std::vector<std::string> ResolveOutputColumns(const std::vector<std::string>& requested,
                                                           const Schema& schema);

    DatabaseEngine* engine_;
};

}  // namespace minidb
