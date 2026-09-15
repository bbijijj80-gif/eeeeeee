// Тесты для SQLLexer/SQLParser
#include <cassert>
#include <iostream>

#include "../src/query/ASTNodes.h"
#include "../src/query/SQLParser.h"

using namespace minidb;

void TestCreateTable() {
    SQLParser parser("CREATE TABLE users (id INT, name VARCHAR(50))");
    auto stmt = parser.Parse();
    assert(stmt->GetType() == StatementType::CREATE_TABLE);

    auto* create = static_cast<CreateTableStatement*>(stmt.get());
    assert(create->table_name == "users");
    assert(create->columns.size() == 2);
    assert(create->columns[0].name == "id");
    assert(create->columns[0].type == ColumnType::INTEGER);
    assert(create->columns[1].type == ColumnType::VARCHAR);
    std::cout << "[OK] TestCreateTable" << std::endl;
}

void TestInsert() {
    SQLParser parser("INSERT INTO users VALUES (1, 'Alice')");
    auto stmt = parser.Parse();
    assert(stmt->GetType() == StatementType::INSERT);

    auto* insert = static_cast<InsertStatement*>(stmt.get());
    assert(insert->table_name == "users");
    assert(insert->values.size() == 2);
    assert(insert->values[0].int_val == 1);
    assert(insert->values[1].str_val == "Alice");
    std::cout << "[OK] TestInsert" << std::endl;
}

void TestSelectWithWhere() {
    SQLParser parser("SELECT id, name FROM users WHERE id = 1");
    auto stmt = parser.Parse();
    assert(stmt->GetType() == StatementType::SELECT);

    auto* select = static_cast<SelectStatement*>(stmt.get());
    assert(select->table_name == "users");
    assert(select->columns.size() == 2);
    assert(select->has_where);
    assert(select->where.column == "id");
    assert(select->where.op == CompareOp::EQ);
    assert(select->where.value.int_val == 1);
    std::cout << "[OK] TestSelectWithWhere" << std::endl;
}

void TestSelectStar() {
    SQLParser parser("SELECT * FROM users");
    auto stmt = parser.Parse();
    auto* select = static_cast<SelectStatement*>(stmt.get());
    assert(select->columns.size() == 1 && select->columns[0] == "*");
    assert(!select->has_where);
    std::cout << "[OK] TestSelectStar" << std::endl;
}

int main() {
    TestCreateTable();
    TestInsert();
    TestSelectWithWhere();
    TestSelectStar();
    std::cout << "Все тесты SQLParser пройдены успешно." << std::endl;
    return 0;
}
