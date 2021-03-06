#include "insert_statement.h"
#include "insert_exception.h"

using std::string;

namespace yw {
    namespace sqlite {

        void InsertStatement::execute() {
            int rc = sqlite3_step(statement);
            if (rc != SQLITE_DONE) {
                auto errorMessage = db->getLastErrorMessage();
                throw InsertException(errorMessage, sql);
            }
        }

        row_id InsertStatement::getGeneratedId() {
            return static_cast<row_id>(sqlite3_last_insert_rowid(db->getConnection()));
        }
    }
}