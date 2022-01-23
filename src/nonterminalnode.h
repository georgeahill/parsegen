#pragma once

#include "node.h"
#include "nonterminal.h"

namespace ParseGen::GUI
{
    class NonTerminalNode : public Node
    {
    public:
        NonTerminal *value;
        uint id;

        NonTerminalNode(NonTerminal *value) : Node(), value(value)
        {
            this->iconType = IconType::RoundSquare;
            this->pinColor = ImColor(ImVec4(0.3f, 0.0f, 0.8f, 1.0f));
            this->innerPinColor = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        };

        void Update() override
        {
            this->name = "<" + this->value->name + ">";
        }
    };
}