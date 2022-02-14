#pragma once

#include <imgui.h>

enum class IconType : ImU32
{
    Circle,
    Square,
    Grid,
    RoundSquare,
    Diamond
};

void DrawIcon(ImDrawList *drawList, IconType type, const ImVec2 &a, const ImVec2 &b, bool filled, ImColor color, ImColor innerColor);
void Icon(IconType type, const ImVec2 &size, bool filled, const ImVec4 &color, const ImVec4 &innerColor);