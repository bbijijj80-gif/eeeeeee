cmake_minimum_required(VERSION 3.15)
project(MiniDB CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

find_package(Threads REQUIRED)

set(MINIDB_CORE_SOURCES
    src/core/DatabaseEngine.cpp
    src/core/BufferPoolManager.cpp
    src/core/DiskManager.cpp
    src/storage/BPlusTree.cpp
    src/storage/Page.cpp
    src/storage/Tuple.cpp
    src/storage/TableHeap.cpp
    src/storage/Record.cpp
    src/query/SQLLexer.cpp
    src/query/SQLParser.cpp
    src/query/ASTNodes.cpp
    src/query/QueryPlanner.cpp
    src/query/QueryExecutor.cpp
    src/transaction/TransactionManager.cpp
    src/transaction/LockManager.cpp
    src/transaction/UndoLog.cpp
    src/recovery/WALManager.cpp
    src/recovery/Checkpoint.cpp
    src/utils/Logger.cpp
    src/utils/ErrorCodes.cpp
    src/utils/MemoryPool.cpp
    src/utils/ConsoleUI.cpp
)

add_library(minidb_core STATIC ${MINIDB_CORE_SOURCES})
target_include_directories(minidb_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(minidb_core PUBLIC Threads::Threads)

if(WIN32)
    enable_language(RC)
    set(MINIDB_WIN32_RESOURCES src/resources.rc)
endif()

add_executable(minidb src/main.cpp ${MINIDB_WIN32_RESOURCES})
target_link_libraries(minidb PRIVATE minidb_core)

enable_testing()

add_executable(test_bplus_tree tests/test_bplus_tree.cpp)
target_link_libraries(test_bplus_tree PRIVATE minidb_core)
add_test(NAME test_bplus_tree COMMAND test_bplus_tree)

add_executable(test_sql_parser tests/test_sql_parser.cpp)
target_link_libraries(test_sql_parser PRIVATE minidb_core)
add_test(NAME test_sql_parser COMMAND test_sql_parser)

add_executable(test_transactions tests/test_transactions.cpp)
target_link_libraries(test_transactions PRIVATE minidb_core)
add_test(NAME test_transactions COMMAND test_transactions)
