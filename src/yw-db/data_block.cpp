#include "ywdb.h"
#include "insert_statement.h"
#include "select_statement.h"

using std::string;
using namespace yw::sqlite;

namespace yw {
    namespace db {

        void YesWorkflowDB::createDataBlockTable() {
            SQLiteDB::createTable(db, std::string(R"(

                CREATE TABLE data_block(
                    id              INTEGER         NOT NULL        PRIMARY KEY,
                    model           INTEGER         NOT NULL        REFERENCES model(id),
                    structure       INTEGER         NULL            REFERENCES data_block(id),
                    name            TEXT            NOT NULL
                );

            )"));
        }

        row_id YesWorkflowDB::insert(const DataBlock& dataBlock) {
            string sql = "INSERT INTO data_block(id, model, structure, name) VALUES (?,?,?,?);";
            InsertStatement statement(db, sql);
            statement.bindNullableId(1, dataBlock.id);
            statement.bindId(2, dataBlock.modelId);
            statement.bindNullableId(3, dataBlock.structureId);
            statement.bindText(4, dataBlock.name);
            statement.execute();
            return statement.getGeneratedId();
        }

        DataBlock YesWorkflowDB::selectDataBlockById(const row_id& requested_id) {
            string sql = "SELECT id, model, structure, name FROM data_block WHERE id = ?";
            SelectStatement statement(db, sql);
            statement.bindId(1, requested_id);
            if (statement.step() != SQLITE_ROW) throw std::runtime_error("No data block with that id");
            auto id = statement.getNullableIdField(0);
            auto modelId = statement.getIdField(1);
            auto structureId = statement.getNullableIdField(2);
            auto name = statement.getTextField(3);
            return DataBlock(id, modelId, structureId, name);
        }
    }
}