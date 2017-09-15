#include "workflow_grapher.h"
#include "dot_builder.h"

using namespace yw::config;
using namespace yw::db;
using namespace yw::sqlite;

using std::string;
using Visibility = SoftwareSetting::Visibility;

namespace yw {
    namespace graph {

        yw::config::Configuration WorkflowGrapher::defaults;

        const Configuration& WorkflowGrapher::getSoftwareSettings() {
            if (defaults.size() == 0) {

                defaults.insert(SoftwareSetting{ "graph.view", "combined", "Workflow view to render", { "PROCESS", "DATA", "COMBINED" } });
                defaults.insert(SoftwareSetting{ "graph.portlayout", "GROUP", "Layout mode for workflow ports",{ "GROUP", "RELAX", "HIDE" } });
                defaults.insert(SoftwareSetting{ "graph.programlabel", "NAME", "Info to display in program nodes", {"NAME", "DESCRIPTION", "BOTH" } });
                defaults.insert(SoftwareSetting{ "graph.workflowbox", "SHOW", "Box around nodes internal to workflow", { "SHOW", "HIDE" } });
                defaults.insert(SoftwareSetting{ "graph.workflow", null_string, "Name of workflow to graph" });

                defaults.insert(SoftwareSetting{ "graph.programshape", "box", "Shape of program block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.programstyle", "filled", "Styling of program block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.programfillcolor", "#CCFFCC", "Color of program block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.programfont", "Helvetica", "Font in program block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.programperipheries", "1", "Number of peripheries for program block nodes", {}, Visibility::INTERMEDIATE });

                defaults.insert(SoftwareSetting{ "graph.datashape", "box", "Shape of data block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.datastyle", "rounded,filled", "Styling of data block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.datafillcolor", "#FFFFCC", "Color of data block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.datafont", "Helvetica", "Font in data block nodes", {}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.dataperipheries", "1", "Number of peripheries for data block nodes",{}, Visibility::INTERMEDIATE });

                defaults.insert(SoftwareSetting{ "graph.portshape", "circle", "Shape of workflow port nodes",{}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.portfillcolor", "#FFFFFF", "Color of workflow portnodes",{}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.portperipheries", "1", "Number of peripheries for workflow port nodes",{}, Visibility::INTERMEDIATE });
                defaults.insert(SoftwareSetting{ "graph.portsize", "0.2", "Diameter of workflow port nodes",{}, Visibility::INTERMEDIATE });

                defaults.insert(SoftwareSetting{ "graph.styles", "ON", "Apply node and edge styles to graph elements",{ "ON", "OFF" }, Visibility::EXPERT });
                defaults.insert(SoftwareSetting{ "graph.layoutstyles", "ON", "Use clusters to group group elements",{ "ON", "OFF" }, Visibility::EXPERT });
            }
            return defaults;
        }

        WorkflowGrapher::WorkflowGrapher(
            yw::db::YesWorkflowDB& ywdb,
            const yw::config::Configuration& userConfiguration
        ) : ywdb(ywdb) {
            configuration.insertAll(getSoftwareSettings());
            configuration.insertAll(userConfiguration);
        }

        string WorkflowGrapher::config(const string& key) {
            return configuration.getStringValue(key);
        }

        string WorkflowGrapher::graph(const row_id& modelId) {
            auto workflowNameSetting = configuration.getSetting("graph.workflow");
            string workflowName;
            if (workflowNameSetting.value.hasValue()) {
                workflowName = workflowNameSetting.value.getValue();
            }
            else {
                auto topLevelProgramBlocks = ywdb.selectTopLevelProgramBlocksInModel(modelId);
                if (topLevelProgramBlocks.size() > 0) {
                    workflowName = topLevelProgramBlocks[0].name;
                }
                else {
                    throw std::runtime_error("No top-level workflow found in model.");
                }
            }
            auto workflow = ywdb.selectProgramBlockByModelIdAndBlockName(modelId, workflowName);
            auto workflowId = workflow.id.getValue();
            dot = std::make_shared<DotBuilder>(configuration);
            dot->beginGraph(workflow.name);
            beginWorkflowBox();
            drawProgramBlocksAsNodes(workflowId);
            drawDataBlocksAsNodes(workflowId);
            drawFlowEdgesBetweenProgramsAndData(workflowId);
            endWorkflowBox();
            drawWorkflowInputsAsNodes(workflowId);
            drawWorkflowOutputsAsNodes(workflowId);
            drawEdgesFromWorkflowInputsToDataNodes(workflowId);
            drawEdgesFromDataNodesToWorkflowOutputs(workflowId);
            dot->endGraph();
            return dot->str();
        }


        void WorkflowGrapher::beginWorkflowBox() {
            if (config("graph.layoutstyles") == "ON") {
                dot->comment("Start of box around nodes in workflow");
                dot->beginSubgraph("workflow_box", config("graph.workflowbox") == "SHOW");
            }
        }

        void WorkflowGrapher::endWorkflowBox() {
            if (config("graph.layoutstyles") == "ON") {
                dot->comment("End of box around nodes in workflow");
                dot->endSubgraph();
            }
        }

        void WorkflowGrapher::drawProgramBlockAsNode(const ProgramBlock& programBlock) {

            auto programLabelMode = config("graph.programlabel");
   
            if (programLabelMode == "NAME") { 
                dot->node(programBlock.name); 
                return; 
            }
            
            if (programLabelMode == "DESCRIPTION") { 
                dot->node(programBlock.name, "description"); 
                return;
            }
            
            if (programLabelMode == "BOTH") { 
                dot->recordNode(programBlock.name, programBlock.name, "description"); 
                return;
            }
        }

        void WorkflowGrapher::drawProgramBlocksAsNodes(const row_id& workflowId) {
            auto programBlocks = ywdb.selectProgramBlocksInWorkflow(workflowId);
            if (programBlocks.size() > 0) {

                if (config("graph.styles") == "ON") {
                    dot->comment("Style for nodes representing program blocks in workflow");
                    dot->setNodeFont(config("graph.programfont"));
                    dot->setNodeShape(config("graph.programshape"));
                    dot->setNodeStyle(config("graph.programstyle"));
                    dot->setNodeFillcolor(config("graph.programfillcolor"));
                    dot->setNodePeripheries(configuration.getIntValue("graph.programperipheries"));
                    dot->flushNodeStyle();
                }

                dot->comment("Nodes representing program blocks in workflow");
                for (auto programBlock : programBlocks) {
                    drawProgramBlockAsNode(programBlock);
                }
            }
        }

        void WorkflowGrapher::drawDataBlocksAsNodes(const row_id& workflowId) {
            auto dataBlocks = ywdb.selectDataBlocksByWorkflowId(workflowId);
            if (dataBlocks.size() > 0) {

                if (config("graph.styles") == "ON") {
                    dot->comment("Style for nodes representing data blocks in workflow");
                    dot->setNodeFont(config("graph.datafont"));
                    dot->setNodeShape(config("graph.datashape"));
                    dot->setNodeStyle(config("graph.datastyle"));
                    dot->setNodeFillcolor(config("graph.datafillcolor"));
                    dot->setNodePeripheries(configuration.getIntValue("graph.dataperipheries"));
                    dot->flushNodeStyle();
                }

                dot->comment("Nodes representing data blocks in workflow");
                for (auto dataBlock : dataBlocks) {
                    dot->node(dataBlock.name);
                }
            }
        }

        void WorkflowGrapher::drawFlowEdgesBetweenProgramsAndData(const row_id& workflowId) {
            auto flows = ywdb.selectFlowViewsByWorkflowId(workflowId);
            if (flows.size() > 0) {
                dot->comment("Edges representing flow of data into and out of code blocks");
                for (auto flow : flows) {
                    if (flow.direction == Flow::Direction::OUT) {
                        dot->edge(flow.programBlockName, flow.dataBlockName);
                    }
                    else {
                        dot->edge(flow.dataBlockName, flow.programBlockName);
                    }
                }
            }
        }

        void WorkflowGrapher::applyWorkflowPortNodeStyle() {
            if (config("graph.styles") == "ON") {
                dot->comment("Style for nodes representing workflow ports");
                dot->setNodeShape(config("graph.portshape"));
                dot->setNodeFillcolor(config("graph.portfillcolor"));
                dot->setNodePeripheries(configuration.getIntValue("graph.portperipheries"));
                dot->setNodeWidth(configuration.getDoubleValue("graph.portsize"));
                dot->flushNodeStyle();
            }
        }

        void WorkflowGrapher::drawWorkflowInputsAsNodes(const row_id& workflowId) {

            if (config("graph.portlayout") == "HIDE") return;

            auto portals = ywdb.selectWorkflowInputsByWorkflowId(workflowId);

            if (portals.size() > 0) {

                bool groupWorkflowPorts = config("graph.layoutstyles") == "ON"
                    && config("graph.portlayout") == "GROUP";

                if (groupWorkflowPorts) {
                    dot->comment("Start of hidden box around workflow inputs");
                    dot->beginSubgraph("workflow_inputs_box", false);
                }

                applyWorkflowPortNodeStyle();

                dot->comment("Nodes representing workflow input ports");
                for (auto portal : portals) {
                    dot->node("workflow input " + portal.programDataBlockName, "");
                }

                if (groupWorkflowPorts) {
                    dot->comment("End of hidden box around workflow inputs");
                    dot->endSubgraph();
                }
            }
        }

        void WorkflowGrapher::drawWorkflowOutputsAsNodes(const row_id& workflowId) {

            if (config("graph.portlayout") == "HIDE") return;

            auto portals = ywdb.selectWorkflowOutputsByWorkflowId(workflowId);

            if (portals.size() > 0) {

                bool groupWorkflowPorts = config("graph.layoutstyles") == "ON"
                    && config("graph.portlayout") == "GROUP";

                if (groupWorkflowPorts) {
                    dot->comment("Start of hidden box around workflow outputs");
                    dot->beginSubgraph("workflow_outputs_box", false);
                }

                applyWorkflowPortNodeStyle();

                dot->comment("Nodes representing workflow output ports");
                for (auto portal : portals) {
                    dot->node("workflow output " + portal.programDataBlockName, "");
                }

                if (groupWorkflowPorts) {
                    dot->comment("End of hidden box around workflow outputs");
                    dot->endSubgraph();
                }
            }
        }

        void WorkflowGrapher::drawEdgesFromWorkflowInputsToDataNodes(const row_id& workflowId) {

            if (config("graph.portlayout") == "HIDE") return;

            auto portals = ywdb.selectWorkflowInputsByWorkflowId(workflowId);

            if (portals.size() > 0) {

                dot->comment("Edges representing flow of workflow input data");
                for (auto portal : portals) {
                    dot->edge("workflow input " + portal.programDataBlockName, portal.programDataBlockName );
                }
            }
        }

        void WorkflowGrapher::drawEdgesFromDataNodesToWorkflowOutputs(const row_id& workflowId) {

            if (config("graph.portlayout") == "HIDE") return;

            auto portals = ywdb.selectWorkflowOutputsByWorkflowId(workflowId);

            if (portals.size() > 0) {

                dot->comment("Edges representing flow of workflow output data");
                for (auto portal : portals) {
                    dot->edge(portal.programDataBlockName, "workflow output " + portal.programDataBlockName);
                }
            }
        }

    }
}
