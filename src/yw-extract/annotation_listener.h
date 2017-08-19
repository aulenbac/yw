#pragma once

#include "ywdb.h"
#include "YWBaseListener.h"

namespace yw {
    namespace extract {

        class AnnotationListener : public YWBaseListener {

        protected:
            yw::db::YesWorkflowDB& ywdb;
            const row_id extractionId;
            const row_id sourceId;
            std::shared_ptr<yw::db::Annotation> currentPrimaryAnnotation = nullptr;
            std::stack<std::shared_ptr<yw::db::Annotation>> primaryAnnotationStack;

        private:
            long currentLineNumber = 0;
            long currentRankOnLine = 0;

        public:
            AnnotationListener(
                yw::db::YesWorkflowDB& ywdb,
                const row_id& extractionId,
                const row_id& sourceId
            ) : ywdb(ywdb), extractionId(extractionId), sourceId(sourceId)
            {}

            void enterBegin(YWParser::BeginContext *context) override;
            void enterEnd(YWParser::EndContext *context) override;
            void enterDesc(YWParser::DescContext *context) override;
            void enterPort(YWParser::PortContext *context) override;
            void enterAlias(YWParser::AliasContext *context) override;
            void enterCall(YWParser::CallContext *context) override;
            void enterUri(YWParser::UriContext *context) override;
            void enterFile(YWParser::FileContext *context) override;
            void enterResource(YWParser::ResourceContext *context) override;
            void exitIo(YWParser::IoContext *context) override;

        private:
            auto getLineId(antlr4::ParserRuleContext* context);
            auto getRangeInLine(antlr4::ParserRuleContext* context);
        };
    }
}

