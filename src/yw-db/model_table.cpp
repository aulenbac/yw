#include "yesworkflow_db.h"
#include "insert_statement.h"
#include "select_statement.h"

using std::string;
using namespace yw::sqlite;

namespace yw {
    namespace db {

        long YesWorkflowDB::insertModel(long creator, const std::string& create_date) {
            string sql = "INSERT INTO model(creator, create_date) VALUES (?,?);";
            InsertStatement statement(db, sql);
            statement.bindInt64(1, creator);
            statement.bindText(2, create_date);
            statement.execute();
            return statement.getGeneratedId();
        }

        ModelRow YesWorkflowDB::selectModelById(long requested_id) {
            string sql = "SELECT id, creator, create_date FROM model WHERE id = ?";
            SelectStatement statement(db, sql);
            statement.bindInt64(1, requested_id);
            if (statement.step() != SQLITE_ROW) throw std::runtime_error("No row with that id");
            auto id = statement.getInt64Field(0);
            auto creator = statement.getInt64Field(1);
            auto create_date = statement.getTextField(2);
            return ModelRow(id, creator, create_date);
        }
    }
}