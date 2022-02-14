#include "drawing.h"
#include <imgui_internal.h>

void Icon(IconType type, const ImVec2 &size, bool filled, const ImVec4 &color, const ImVec4 &innerColor)
{
    if (ImGui::IsRectVisible(size))
    {
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        DrawIcon(drawList, type, cursorPos, ImVec2(cursorPos.x + size.x, cursorPos.y + size.y), filled, color, innerColor);
    }

    ImGui::Dummy(size);
}

void DrawIcon(ImDrawList *drawList, IconType type, const ImVec2 &a, const ImVec2 &b, bool filled, ImColor color, ImColor innerColor)
{

    // ImVec2 a = cursorPos;
    // ImVec2 b = ImVec2(cursorPos.x + 10.0f, cursorPos.y + 10.0f);
    // bool filled = true;
    // ImColor color = ImColor(ImVec4(1.0, 0.0, 0.0, 1.0));
    // ImColor innerColor = ImColor(ImVec4(0.0, 1.0, 0.0, 1.0));

    auto rect = ImRect(a, b);
    auto rect_x = rect.Min.x;
    auto rect_y = rect.Min.y;
    auto rect_w = rect.Max.x - rect.Min.x;
    auto rect_h = rect.Max.y - rect.Min.y;
    auto rect_center_x = (rect.Min.x + rect.Max.x) * 0.5f;
    auto rect_center_y = (rect.Min.y + rect.Max.y) * 0.5f;
    auto rect_center = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle

    auto triangleStart = rect_center_x + 0.32f * rect_w;

    auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

    rect.Min.x += rect_offset;
    rect.Max.x += rect_offset;
    rect_x += rect_offset;
    rect_center_x += rect_offset * 0.5f;
    rect_center.x += rect_offset * 0.5f;

    if (type == IconType::Circle)
    {
        const auto c = rect_center;

        if (!filled)
        {
            const auto r = 0.5f * rect_w / 2.0f - 0.5f;

            if (innerColor & 0xFF000000)
                drawList->AddCircleFilled(c, r, innerColor, 12 + extra_segments);
            drawList->AddCircle(c, r, color, 12 + extra_segments, 2.0f * outline_scale);
        }
        else
            drawList->AddCircleFilled(c, 0.5f * rect_w / 2.0f, color, 12 + extra_segments);
    }

    if (type == IconType::Square)
    {
        if (filled)
        {
            const auto r = 0.5f * rect_w / 2.0f;
            const auto p0 = ImVec2(rect_center_x - r, rect_center_y - r);
            const auto p1 = ImVec2(rect_center_x + r, rect_center_y + r);

            drawList->AddRectFilled(p0, p1, color, 0, 15 + extra_segments);
        }
        else
        {
            const auto r = 0.5f * rect_w / 2.0f - 0.5f;
            const auto p0 = ImVec2(rect_center_x - r, rect_center_y - r);
            const auto p1 = ImVec2(rect_center_x + r, rect_center_y + r);

            if (innerColor & 0xFF000000)
                drawList->AddRectFilled(p0, p1, innerColor, 0, 15 + extra_segments);

            drawList->AddRect(p0, p1, color, 0, 15 + extra_segments, 2.0f * outline_scale);
        }
    }

    if (type == IconType::Grid)
    {
        const auto r = 0.5f * rect_w / 2.0f;
        const auto w = ceilf(r / 3.0f);

        const auto baseTl = ImVec2(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
        const auto baseBr = ImVec2(floorf(baseTl.x + w), floorf(baseTl.y + w));

        auto tl = baseTl;
        auto br = baseBr;
        for (int i = 0; i < 3; ++i)
        {
            tl.x = baseTl.x;
            br.x = baseBr.x;
            drawList->AddRectFilled(tl, br, color);
            tl.x += w * 2;
            br.x += w * 2;
            if (i != 1 || filled)
                drawList->AddRectFilled(tl, br, color);
            tl.x += w * 2;
            br.x += w * 2;
            drawList->AddRectFilled(tl, br, color);

            tl.y += w * 2;
            br.y += w * 2;
        }

        triangleStart = br.x + w + 1.0f / 24.0f * rect_w;
    }
    if (type == IconType::RoundSquare)
    {
        if (filled)
        {
            const auto r = 0.5f * rect_w / 2.0f;
            const auto cr = r * 0.5f;
            const auto p0 = ImVec2(rect_center_x - r, rect_center_y - r);
            const auto p1 = ImVec2(rect_center_x + r, rect_center_y + r);

            drawList->AddRectFilled(p0, p1, color, cr, 15);
        }
        else
        {
            const auto r = 0.5f * rect_w / 2.0f - 0.5f;
            const auto cr = r * 0.5f;
            const auto p0 = ImVec2(rect_center_x - r, rect_center_y - r);
            const auto p1 = ImVec2(rect_center_x + r, rect_center_y + r);

            if (innerColor & 0xFF000000)
                drawList->AddRectFilled(p0, p1, innerColor, cr, 15);

            drawList->AddRect(p0, p1, color, cr, 15, 2.0f * outline_scale);
        }
    }
    else if (type == IconType::Diamond)
    {
        if (filled)
        {
            const auto r = 0.607f * rect_w / 2.0f;
            const auto c = rect_center;

            drawList->PathLineTo(ImVec2(c.x + 0, c.y - r));
            drawList->PathLineTo(ImVec2(c.x + r, c.y + 0));
            drawList->PathLineTo(ImVec2(c.x + 0, c.y + r));
            drawList->PathLineTo(ImVec2(c.x - r, c.y + 0));
            drawList->PathFillConvex(color);
        }
        else
        {
            const auto r = 0.607f * rect_w / 2.0f - 0.5f;
            const auto c = rect_center;

            drawList->PathLineTo(ImVec2(c.x + 0, c.y - r));
            drawList->PathLineTo(ImVec2(c.x + r, c.y + 0));
            drawList->PathLineTo(ImVec2(c.x + 0, c.y + r));
            drawList->PathLineTo(ImVec2(c.x - r, c.y + 0));

            if (innerColor & 0xFF000000)
                drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);

            drawList->PathStroke(color, true, 2.0f * outline_scale);
        }
    }
}