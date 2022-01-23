#pragma once

#include <string>
#include <vector>

class Node;

#include "nodelink.h"
#include "drawing.h"

#include <imgui/nodes/imgui_node_editor.h>
namespace ed = ax::NodeEditor;

#include <iostream>

namespace ParseGen::GUI
{
    class Node
    {
    public:
        uint id;

        ed::NodeId nodeId;
        // these can never be 0 (since they are offset from nodeId)
        // so we can use this to check if they've been set
        ed::PinId inPin = 0;
        ed::PinId outPin = 0;

        bool inIsConnected = false;
        bool outIsConnected = false;
        int numOutConnected = 0;
        int numInConnected = 0;

        int x;
        int y;

        std::string name;

        std::vector<NodeLink *> nextLinks;

        inline static uint currentId = 1;
        inline static const ImVec2 IconSize = ImVec2(24, 24);

        // should be set by individual derived classes; (non-static) option is left to set on a node-level
        IconType iconType;
        ImColor pinColor;
        ImColor innerPinColor = ImColor(ImVec4(0, 0, 0, 0));

        // this looks messy but simply assigns id to currentId
        // and increments currentId by 3 (for nodeId, inPin, outPin)
        Node()
        {
            id = currentId;
            nodeId = ed::NodeId(currentId++);
            inPin = ed::PinId(currentId++);
            outPin = ed::PinId(currentId++);
        };

        // optionally called each frame, if overridden
        virtual void Update(){};

        virtual void RenderFrame();

        virtual ~Node() = default;
    };
}
