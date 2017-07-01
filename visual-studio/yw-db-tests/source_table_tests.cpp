#include "common_includes.h"

using namespace yw::test;
using namespace yw::db;
using std::make_unique;

namespace yw {
    namespace db {

        TEST_CLASS(SourceTableTests)
        {
        public:

            TEST_METHOD(TestInsertSource_OneRow_GeneratedIdIs_1)
            {
                YesWorkflowDB ywdb;
                long userId, modelId, languageId, fileId, sourceId;
                Assert::AreEqual(1, (userId = ywdb.insertUser("user1")));
                Assert::AreEqual(1, (modelId = ywdb.insertModel(userId, "2017-06-22 10:52:00.000")));
                Assert::AreEqual(1, (fileId = ywdb.insertFile("main.c", userId)));
                Assert::AreEqual(1, (languageId = ywdb.insertLanguage("C")));
                Assert::AreEqual(1, (sourceId = ywdb.insertSource(modelId, languageId, fileId)));
            }

            TEST_METHOD(TestInsertSource_TwoRows_SecondGeneratedIdIs_2)
            {
                YesWorkflowDB ywdb;
                long userId, modelId, language1, language2, file1, file2, source1, source2;
                Assert::AreEqual(1, (userId = ywdb.insertUser("user1")));
                Assert::AreEqual(1, (modelId = ywdb.insertModel(userId, "2017-06-22 10:52:00.000")));
                Assert::AreEqual(1, (file1 = ywdb.insertFile("main.c", userId)));
                Assert::AreEqual(2, (file2 = ywdb.insertFile("script.sh", userId)));
                Assert::AreEqual(1, (language1 = ywdb.insertLanguage("C")));
                Assert::AreEqual(2, (language2 = ywdb.insertLanguage("Bash")));
                Assert::AreEqual(1, (source1 = ywdb.insertSource(modelId, language1, file1)));
                Assert::AreEqual(2, (source2 = ywdb.insertSource(modelId, language2, file2)));
            }

            TEST_METHOD(TestSelectSourceById_RowExists) {
                YesWorkflowDB ywdb;
                long userId, modelId, languageId, fileId, sourceId;
                Assert::AreEqual(1, (userId = ywdb.insertUser("user1")));
                Assert::AreEqual(1, (modelId = ywdb.insertModel(userId, "2017-06-22 10:52:00.000")));
                Assert::AreEqual(1, (fileId = ywdb.insertFile("main.c", userId)));
                Assert::AreEqual(1, (languageId = ywdb.insertLanguage("C")));
                Assert::AreEqual(1, (sourceId = ywdb.insertSource(modelId, languageId, fileId)));

                auto source = ywdb.selectSourceById(1L);
                Assert::AreEqual(1, source.id);
                Assert::AreEqual(1, source.model);
                Assert::AreEqual(1, source.file);
                Assert::AreEqual(1, source.language);
            }

            TEST_METHOD(TestSelectSourceById_RowDoesntExist) {
                YesWorkflowDB ywdb;
                try {
                    auto source = ywdb.selectSourceById(1L);
                    Assert::Fail();
                }
                catch (std::runtime_error& e) {
                    Assert::AreEqual("No row with that id", e.what());
                }
            }
        };
    }
}