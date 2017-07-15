#include "yesworkflow_db.h"
#include "insert_statement.h"
#include "select_statement.h"

using std::string;
using namespace yw::sqlite;

namespace yw {
    namespace db {

		void YesWorkflowDB::createAnnotationTable() {
			SQLiteDB::createTable(db, std::string(R"(

				CREATE TABLE annotation(
					id                  INTEGER         NOT NULL        PRIMARY KEY,
					qualifies           INTEGER         NULL            REFERENCES annotation(id),
					line				INTEGER			NOT NULL		REFERENCES line(id),
					start				INTEGER         NOT NULL,
					end					INTEGER         NOT NULL,
					tag					TEXT			NOT NULL,
					value               TEXT            NOT NULL
				);

			)"));
		}

        long YesWorkflowDB::insert(const AnnotationRow& annotation) {
            string sql = "INSERT INTO annotation(qualifies, line, start, end, tag, value) VALUES (?,?,?,?,?,?);";
            InsertStatement statement(db, sql);
            statement.bindNullableInt64(1, annotation.qualifiesId);
            statement.bindInt64(2, annotation.lineId);
            statement.bindInt64(3, annotation.start);
			statement.bindInt64(4, annotation.end);
			statement.bindText(5, annotation.tag);
			statement.bindText(6, annotation.value);
			statement.execute();
            return statement.getGeneratedId();
        }

        AnnotationRow YesWorkflowDB::selectAnnotationById(long requested_id) {
            string sql = "SELECT id, qualifies, line, start, end, tag, value FROM annotation WHERE id = ?";
            SelectStatement statement(db, sql);
            statement.bindInt64(1, requested_id);
            if (statement.step() != SQLITE_ROW) throw std::runtime_error("No row with that id");
			auto id = statement.getIdField(0);
			auto qualifies = statement.getNullableIdField(1);
			auto lineId = statement.getInt64Field(2);
            auto start = statement.getInt64Field(3);
            auto end = statement.getInt64Field(4);
			auto tag = statement.getTextField(5);
			auto value = statement.getTextField(6);
			return AnnotationRow(id, qualifies, lineId, start, end, tag, value);
        }
    }
}