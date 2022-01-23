#include "node.h"

namespace ParseGen::GUI
{
    void Node::RenderFrame()
    {
        Update();
        ed::BeginNode(this->nodeId);

        if (this->inPin)
        {
            ImGui::BeginGroup();
            ed::BeginPin(this->inPin, ed::PinKind::Input);
            Icon(iconType, Node::IconSize, inIsConnected, pinColor, innerPinColor);
            ed::EndPin();
            ImGui::EndGroup();
            ImGui::SameLine();
        }

        // align text to middle-ish
        if (this->inPin || this->outPin)
        {
            ImGui::AlignTextToFramePadding();
        }

        ImGui::TextUnformatted(name.c_str());

        if (this->outPin)
        {
            ImGui::SameLine();
            ImGui::BeginGroup();
            ed::BeginPin(this->outPin, ed::PinKind::Output);
            Icon(iconType, Node::IconSize, outIsConnected, pinColor, innerPinColor);
            ed::EndPin();
            ImGui::EndGroup();
        }

        ed::EndNode();
    };
}