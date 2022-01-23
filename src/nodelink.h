#pragma once

class NodeLink;

#include "node.h"

#include <imgui/nodes/imgui_node_editor.h>
namespace ed = ax::NodeEditor;

namespace ParseGen::GUI
{
    class NodeLink
    {
    public:
        uint id;
        ed::LinkId linkId;

        Node *startNode;
        Node *endNode;

        inline static uint current_id = 1;

        NodeLink(Node *endNode, Node *startNode) : endNode(endNode), startNode(startNode)
        {
            id = current_id;
            linkId = current_id++;
        };
    };
}
