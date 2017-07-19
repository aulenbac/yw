#include "yw_extract_tests.h"

using namespace yw::extract;
using namespace yw::parse;
using namespace yw::test;
using namespace yw::db;
using namespace yw::sqlite;

YW_TEST_FIXTURE(AnnotationListener)

	YesWorkflowDB ywdb;
	AnnotationListener* listener;
	StderrRecorder stderrRecorder;

	YW_TEST_SETUP(AnnotationListener) {

		long userId, modelId, languageId, sourceId;

		Expect::AreEqual(1, (userId = ywdb.insert(UserRow{ auto_id, "user1" })));
		Expect::AreEqual(1, (modelId = ywdb.insert(ModelRow{ auto_id, userId, "2017-06-22 10:52:00.000" })));
		Expect::AreEqual(1, (languageId = ywdb.insert(LanguageRow{ auto_id, "C" })));
		Expect::AreEqual(1, (sourceId = ywdb.insert(SourceRow{ auto_id, modelId, languageId, null_id })));

		listener = new AnnotationListener(ywdb, sourceId);
	}

YW_TEST_SET

	YW_TEST(AnnotationListener, EnterBeginHandlerInsertsBeginAnnotation)
	{
		YWParserBuilder parser_builder("@begin b");
		antlr4::tree::ParseTreeWalker::DEFAULT.walk(listener, parser_builder.parse()->begin());
		Expect::EmptyString(stderrRecorder.str());

		auto beginAnnotation = ywdb.selectAnnotationById(1L);
		Assert::IsNull(beginAnnotation.qualifiesId);
		Assert::AreEqual(1, beginAnnotation.lineId);
		Assert::AreEqual(0, beginAnnotation.start);
		Assert::AreEqual(7, beginAnnotation.end);
		Assert::AreEqual("@begin", beginAnnotation.tag);
		Assert::AreEqual("b", beginAnnotation.value.getValue());
	}

	YW_TEST(AnnotationListener, WhenEndAnnotationHasArgumentEnterEndHandlerInsertsEndAnnotationWithBlockName)
	{
		YWParserBuilder parser_builder("@end b");
		antlr4::tree::ParseTreeWalker::DEFAULT.walk(listener, parser_builder.parse()->end());
		Expect::EmptyString(stderrRecorder.str());

		auto beginAnnotation = ywdb.selectAnnotationById(1L);
		Assert::IsNull(beginAnnotation.qualifiesId);
		Assert::AreEqual(1, beginAnnotation.lineId);
		Assert::AreEqual(0, beginAnnotation.start);
		Assert::AreEqual(5, beginAnnotation.end);
		Assert::AreEqual("@end", beginAnnotation.tag);
		Assert::AreEqual("b", beginAnnotation.value.getValue());
	}

	YW_TEST(AnnotationListener, WhenEndAnnotationHasNoArgumentEnterEndHandlerInsertsEndAnnotationWithoutBlockName)
	{
		YWParserBuilder parser_builder("@end");
		antlr4::tree::ParseTreeWalker::DEFAULT.walk(listener, parser_builder.parse()->end());
		Expect::EmptyString(stderrRecorder.str());

		auto beginAnnotation = ywdb.selectAnnotationById(1L);
		Assert::IsNull(beginAnnotation.qualifiesId);
		Assert::AreEqual(1, beginAnnotation.lineId);
		Assert::AreEqual(0, beginAnnotation.start);
		Assert::AreEqual(3, beginAnnotation.end);
		Assert::AreEqual("@end", beginAnnotation.tag);
		Assert::IsNull(beginAnnotation.value);
	}

YW_TEST_END
