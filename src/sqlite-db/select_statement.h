#pragma once

#include "bindable_statement.h"

#include <memory>
#include <string>

namespace yw {
    namespace sqlite {

        class SelectStatement : public BindableStatement {

        public:

            SelectStatement(SQLiteDB& connection, std::string sql);

            int step();

            long getInt64Field(int column);
            std::string getTextField(int column);
        };
    }
}