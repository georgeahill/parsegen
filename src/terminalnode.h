#pragma once
#include "node.h"

namespace ParseGen::GUI
{
    class TerminalNode : public Node
    {
    public:
        TerminalNode(std::string value)
        {
            this->name = value;
            this->iconType = IconType::RoundSquare;
            this->pinColor = ImColor(ImVec4(0.96078f, 0.57647f, 0.25882f, 1.0f));
            this->innerPinColor = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        };
    };
}
