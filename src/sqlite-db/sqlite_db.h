#pragma once

#include "sqlite3.h"
#include "sqlite_log.h"
#include "nullable.h"
#include "table_row.h"

#include <memory>
#include <string>
#include <sstream>

namespace yw {
    namespace sqlite {

        class SQLiteDB {

        public:

            SQLiteDB();
            ~SQLiteDB();

            void execute(const std::string& sql);

            std::string getLastErrorMessage();

            static std::string textToString(const unsigned char* text) {
                return std::string((const char*)text);
            }

            sqlite3* getConnection() { return connection; }

            static void createTable(std::shared_ptr<SQLiteDB> connection, const std::string& sql);
            static size_t getRowCount(std::shared_ptr<SQLiteDB> connection, const std::string& tableName);

        private:

            SQLiteLog log;
            sqlite3* connection;
        };

    }
}

