#include "stdafx.h"
#include "CppUnitTest.h"

#include "YWLexer.h"
#include "YWListener.h"
#include "YWParser.h"
#include "YWBaseListener.h"
#include "ParserBuilder.h"
#include "test_helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using std::stringstream;
using std::endl;

namespace yw_parse_tests
{
    TEST_CLASS(BeginTagContextTests)
    {
    public:

        TEST_METHOD(TestBeginTagContext_NoName)
        {
            ParserBuilder parserBuilder("@begin");
            YWParser::BeginTagContext* context = parserBuilder.parser()->beginTag();
            Assert::IsNull(context->blockName());
        }

        TEST_METHOD(TestBeginTagContext_NameOnNextLineNotFound)
        {
            ParserBuilder parserBuilder(
                "@begin "   "\n"
                "block"     "\n" );
            YWParser::BeginTagContext* context = parserBuilder.parser()->beginTag();
            Assert::IsNull(context->blockName());
        }

        TEST_METHOD(TestBeginTagContext_NameWithoutSpaces)
        {
            ParserBuilder parserBuilder("@begin block");
            YWParser::BeginTagContext* context = parserBuilder.parser()->beginTag();
            YW::Assert::AreEqual("block", context->blockName()->getText());
        }

        TEST_METHOD(TestBeginTagContext_NameWithSpaces)
        {
            ParserBuilder parserBuilder("@begin b l o c k");
            YWParser::BeginTagContext* context = parserBuilder.parser()->beginTag();
            YW::Assert::AreEqual("b l o c k", context->blockName()->getText());
        }

        TEST_METHOD(TestBeginTagContext_NameWithSpacesTerminatedByNewline)
        {
            ParserBuilder parserBuilder(
                "@begin b l o "  "\n"
                "c k"           "\n" );
            YWParser::BeginTagContext* context = parserBuilder.parser()->beginTag();
            YW::Assert::AreEqual("b l o", context->blockName()->getText());
        }

        TEST_METHOD(TestBeginTagContext_BlockNameOnNextLineIsNotFound)
        {
            ParserBuilder parserBuilder(
                "@begin"    "\n"
                "b"         "\n");
            YWParser::BeginTagContext* context = parserBuilder.parser()->beginTag();
 
            YW::Assert::AreEqual("@begin", context->BeginKeyword()->getText());
            Assert::IsNull(context->blockName());
        }
    };
}