#pragma once

#include "yw_db.h"

namespace yw {
    namespace db {

        struct BlockRow : sqlite::TableRow {

            const nullable_row_id id;
            const row_id modelId;
            const nullable_row_id workflowId;
            const nullable_row_id annotationId;
            const std::string name;

            BlockRow(
                const nullable_row_id& id,
                const row_id& modelId,
                const nullable_row_id workflowId,
                const nullable_row_id annotationId,
                const std::string& name
            ) : id(id), modelId(modelId), workflowId(workflowId), annotationId(annotationId), name(name)
            {}
        };
    }
}