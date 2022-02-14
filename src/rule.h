#pragma once

#include <map>

#include "nodelink.h"
#include "nonterminal.h"
#include "specialnode.h"
#include "terminalnode.h"

#include <imgui/nodes/imgui_node_editor.h>
namespace ed = ax::NodeEditor;

namespace ParseGen::GUI
{
    class Rule
    {
    public:
        NonTerminal *symbol;

        std::vector<NodeLink *> m_Links;
        std::vector<Node *> m_Nodes;
        std::map<std::string, int> m_Terminals;

        SpecialNode *m_StartNode, *m_EndNode;

        Rule(NonTerminal *symbol);
        // ~Rule();

        std::string getName();

        uint getNonTermId();

        std::string RenderFrame();
        std::string BNF();

        uint id;
        inline static uint currentId = 1;

    private:
        ed::EditorContext *m_Context;
        bool m_FirstFrame;
    };
}
