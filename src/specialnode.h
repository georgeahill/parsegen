#pragma once
#include "node.h"

namespace ParseGen::GUI
{
    class SpecialNode : public Node
    {
    public:
        SpecialNode(bool isStartNode)
        {
            if (isStartNode)
            {
                this->inPin = 0;
                this->name = "START";
            }
            else
            {
                this->outPin = 0;
                this->name = "END";
            }

            this->iconType = IconType::Diamond;
            this->pinColor = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            this->innerPinColor = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        };
    };
}
