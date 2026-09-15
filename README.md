# MiniDB

MiniDB — учебное ядро реляционной СУБД на C++17 с настоящим постраничным движком хранения,
буферным пулом с LRU-вытеснением, B+ деревом, простым SQL-парсером, планировщиком/исполнителем
запросов, менеджером транзакций с блокировками и Undo-журналом, а также Write-Ahead логированием.

## Архитектура

```
SQLLexer → SQLParser → AST → QueryPlanner → QueryExecutor
                                                  │
                                                  ▼
                                       TableHeap / BPlusTree
                                                  │
                                                  ▼
                                       BufferPoolManager (LRU)
                                          │              │
                                          ▼              ▼
                                    DiskManager      WALManager
                                          │
                                          ▼
                                     minidb.db
```

* **DiskManager** — единственный компонент, читающий/пишущий страницы по 4KB напрямую на диск.
* **BufferPoolManager** — кэширует страницы в RAM, вытесняет "холодные" страницы по алгоритму LRU,
  сбрасывая "грязные" страницы на диск.
* **BPlusTree / TableHeap** — структуры хранения, работающие исключительно через буферный пул
  (никогда не обращаются к DiskManager напрямую).
* **SQLLexer / SQLParser** — превращают текст SQL в AST (поддерживаются `CREATE TABLE`,
  `INSERT INTO ... VALUES`, `SELECT ... FROM ... [WHERE]`).
* **QueryPlanner** — строит физический план, выбирая `IndexScan` (через B+ дерево) вместо
  полного перебора `SeqScan`, когда это возможно.
* **TransactionManager / LockManager / UndoLog** — обеспечивают ACID: блокировки на уровне
  таблицы (Shared/Exclusive) и откат через журнал компенсирующих действий.
* **WALManager / Checkpoint** — журналирование "вперёд записи" (Write-Ahead Logging) для
  восстановления после сбоев.

## Структура проекта

```
MiniDB/
├── src/
│   ├── main.cpp
│   ├── core/               # DatabaseEngine, BufferPoolManager, DiskManager, LRUReplacer
│   ├── storage/             # BPlusTree, Page, Tuple, TableHeap, Record
│   ├── query/                # SQLLexer, SQLParser, ASTNodes, QueryPlanner, QueryExecutor
│   ├── transaction/          # TransactionManager, LockManager, UndoLog
│   ├── recovery/             # WALManager, Checkpoint
│   └── utils/                 # Logger, ErrorCodes, MemoryPool
├── include/                   # types.h, config.h, macros.h
├── tests/                      # test_bplus_tree, test_sql_parser, test_transactions
├── CMakeLists.txt
├── build.bat
└── README.md
```

## Сборка (Linux/macOS)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
ctest
```

## Сборка (Windows)

```bat
build.bat
```

## Пример использования

```
minidb> CREATE TABLE users (id INT, name VARCHAR(50))
Таблица 'users' успешно создана
minidb> INSERT INTO users VALUES (1, 'Alice')
1 строка добавлена в таблицу 'users'
minidb> INSERT INTO users VALUES (2, 'Bob')
1 строка добавлена в таблицу 'users'
minidb> SELECT * FROM users WHERE id = 2
id      name
2       Bob
1 строк(и) найдено (поиск по B+ дереву)
```

## Ограничения (осознанные упрощения учебного проекта)

- Блокировки — на уровне таблицы, а не строки.
- B+ дерево не выполняет слияние узлов при удалении (только у листьев/внутренних узлов
  происходит split при переполнении).
- WAL используется для журналирования операций, но полный ARIES-style redo/undo при
  запуске после сбоя не реализован — это следующий шаг развития движка.
- Поддерживается один индекс на таблицу — по первому столбцу, если он `INTEGER`.
