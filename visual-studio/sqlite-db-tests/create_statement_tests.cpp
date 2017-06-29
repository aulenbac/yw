#include "stdafx.h"
#include "CppUnitTest.h"
#include "sqlite_db.h"
#include "create_statement.h"
#include "insert_statement.h"
#include "preparation_exception.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace yw {
    namespace sqlite {

        TEST_CLASS(CreateStatementTests)
        {
        public:

            TEST_METHOD(TestCreateStatement_Constructor_ValidStatement)
            {
                SQLiteDB db;
                CreateStatement statement(db, R"(
                    CREATE TABLE user(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        name                TEXT            NULL
                    );
                )");
            }


            TEST_METHOD(TestCreateStatement_Constructor_InvalidStatement_CommaAfterLastColumn)
            {
                SQLiteDB db;
                try {
                    CreateStatement statement(db, R"(
                        CREATE TABLE user(
                            id                  INTEGER         NOT NULL        PRIMARY KEY,
                            name                TEXT            NULL,
                        );
                    )");
                    Assert::Fail();
                }
                catch (PreparationException& e) {
                    Assert::AreEqual(R"<>(Error preparing statement: near ")": syntax error)<>", e.what().c_str());
                }
            }

            TEST_METHOD(TestCreateStatement_Constructor_InvalidStatement_MissingCommaBetweenColumns)
            {
                SQLiteDB db;
                try {
                    CreateStatement statement(db, R"(
                        CREATE TABLE user(
                            id                  INTEGER         NOT NULL        PRIMARY KEY
                            name                TEXT            NULL
                        );
                    )");
                    Assert::Fail();
                }
                catch (PreparationException& e) {
                    Assert::AreEqual(R"<>(Error preparing statement: near "name": syntax error)<>", e.what().c_str());
                }
            }

            TEST_METHOD(TestCreateStatement_Execute_SingleTable)
            {
                SQLiteDB db;
                CreateStatement createUser(db, R"(
                    CREATE TABLE user(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        name                TEXT            NULL
                    );
                )");
                createUser.execute();
            }

            TEST_METHOD(TestCreateStatement_Execute_TwoTables)
            {
                SQLiteDB db;
                CreateStatement createUser(db, R"(
                    CREATE TABLE user(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        name                TEXT            NULL
                    );
                )");
                CreateStatement createFile(db, R"(
                    CREATE TABLE file(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        filename            TEXT            NULL
                    );
                )");
                createUser.execute();
                createFile.execute();
            }

            TEST_METHOD(TestCreateStatement_Execute_TwoTables_ValidReference)
            {
                SQLiteDB db;
                CreateStatement createUser(db, R"(
                    CREATE TABLE user(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        name                TEXT            NULL
                    );
                )");
                CreateStatement createFile(db, R"(
                    CREATE TABLE file(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        filename            TEXT            NULL,
                        owner               INTEGER         NOT NULL        REFERENCES user(id)
                    );
                )");
                createUser.execute();
                createFile.execute();
            }

            TEST_METHOD(TestCreateStatement_Execute_TwoTables_InvalidReference)
            {
                SQLiteDB db;
                CreateStatement createUser(db, R"(
                    CREATE TABLE user(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        name                TEXT            NULL
                    );
                )");
                createUser.execute();
                //Assert::AreEqual("", yw::sqlite::getLastLogMessage().c_str());

                CreateStatement createFile(db, R"(
                  CREATE TABLE file(
                        id                  INTEGER         NOT NULL        PRIMARY KEY,
                        filename            TEXT            NOT NULL,
                        user                INTEGER         NOT NULL        REFERENCES user(id)
                  );
                )");
                createFile.execute();
                Assert::AreEqual("", yw::sqlite::getLastLogMessage().c_str());

                try {
                    InsertStatement statement(db, "INSERT INTO file(filename, user) VALUES (?,?);");
                    statement.bindText(1, "script.sh");
                    statement.bindInt64(2, 1);
                    statement.execute();
                }
                catch (PreparationException e) {
                    std::string m = e.getMessage();
                }
                Assert::AreEqual(
                    "abort at 14 in [INSERT INTO file(filename, user) VALUES (?,?);]: FOREIGN KEY constraint failed", 
                    yw::sqlite::getLastLogMessage().c_str());
            }
        };
    }
}