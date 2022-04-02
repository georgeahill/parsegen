#include <algorithm>

#include <iostream> // FIXME

#include "parsetreerenderer.h"

namespace ParseGen::GUI::ParseTree
{
    Parser::Util::Node *Renderer::firstWalk(Parser::Util::Node *v, float distance = 1.0)
    {
        if (v->isLeaf())
        {
            if (v->leftmostSibling() != nullptr)
            {
                v->x = v->leftBrother()->x + distance;
            }
            else
            {
                v->x = 0.0f;
            }
        }
        else
        {
            Parser::Util::Node *defaultAncestor = v->getChildren()[0];
            for (auto w : v->getChildren())
            {
                firstWalk(w);
                defaultAncestor = apportion(w, defaultAncestor, distance);
            }

            executeShifts(v);

            auto ell = v->getChildren()[0];
            auto arr = v->getChildren()[v->getChildren().size() - 1];
            float midpoint = (ell->x + arr->x) / 2;

            auto w = v->leftBrother();

            if (w != nullptr)
            {
                v->x = w->x + distance;
                v->mod = v->x - midpoint;
            }
            else
            {
                v->x = midpoint;
            }
        }

        return v;
    }

    Parser::Util::Node *Renderer::apportion(Parser::Util::Node *v, Parser::Util::Node *defaultAncestor, float distance)
    {
        Parser::Util::Node *w = v->leftBrother();
        if (w != nullptr)
        {
            auto vir = v;
            auto vor = v;
            auto vil = w;
            auto vol = v->leftmostSibling();
            auto sir = v->mod;
            auto sor = v->mod;
            auto sil = vil->mod;
            auto sol = vol->mod;
            while (vil->right() != nullptr && vir->left() != nullptr)
            {
                vil = vil->right();
                vir = vir->left();
                vol = vol->left();
                vor = vor->right();
                vor->ancestor = v;
                float shift = (vil->x + sil) - (vir->x + sir) + distance;
                if (shift > 0)
                {
                    Parser::Util::Node *a = ancestor(vil, v, defaultAncestor);
                    moveSubtree(a, v, shift);
                    sir = sir + shift;
                    sor = sor + shift;
                }
                sil += vil->mod;
                sir += vir->mod;
                sol += vol->mod;
                sor += vor->mod;
            }

            if (vil->right() != nullptr && vor->right() == nullptr)
            {
                vor->thread = vil->right();
                vor->mod += sil - sor;
            }
            else
            {
                if (vir->left() != nullptr && vol->left() == nullptr)
                {
                    vol->thread = vir->left();
                    vol->mod += sir - sol;
                }
                defaultAncestor = v;
            }
        }

        return defaultAncestor;
    }

    void Renderer::moveSubtree(Parser::Util::Node *wl, Parser::Util::Node *wr, float shift)
    {
        float subtrees = (float)(wr->number() - wl->number());
        wr->change -= shift / subtrees;
        wr->shift += shift;
        wl->change += shift / subtrees;
        wr->x += shift;
        wr->mod += shift;
    }

    void Renderer::executeShifts(Parser::Util::Node *v)
    {
        float shift = 0;
        float change = 0;
        auto vChildren = v->getChildren();
        for (auto it = vChildren.rbegin(); it != vChildren.rend(); ++it)
        {
            auto w = *it;
            w->x += shift;
            w->mod += shift;
            change += w->change;
            shift += w->shift + change;
        }
    }

    Parser::Util::Node *Renderer::ancestor(Parser::Util::Node *vil, Parser::Util::Node *v, Parser::Util::Node *defaultAncestor)
    {
        for (auto child : v->parent->getChildren())
        {
            if (child == vil->ancestor)
            {
                return vil->ancestor;
            }
        }

        return defaultAncestor;
    }

#define PG_FLOAT_INF 1000000.0f
    float Renderer::secondWalk(Parser::Util::Node *v, float m = 0, float depth = 0, float min = PG_FLOAT_INF)
    {
        v->x += m;
        v->y = depth;

        if (min == PG_FLOAT_INF or v->x < min)
        {
            min = v->x;
        }

        for (auto w : v->getChildren())
        {
            min = secondWalk(w, m + v->mod, depth + 1, min);
        }

        return min;
    }
#undef PG_FLOAT_INF

    Parser::Util::Node *Renderer::thirdWalk(Parser::Util::Node *tree, float n)
    {
        tree->x += n;
        for (auto c : tree->getChildren())
            thirdWalk(c, n);
        return tree;
    }

    Parser::Util::Node *Renderer::buchheim(Parser::Util::Node *parseTree)
    {
        Parser::Util::Node *dt = firstWalk(parseTree);
        float min = secondWalk(dt);
        if (min < 0)
            thirdWalk(dt, -min);

        return dt;
    }

    Parser::Util::Node *Renderer::simpleDraw(Parser::Util::Node *parseTree)
    {
        for (auto child : parseTree->getChildren())
        {
            simpleDraw(child);
        }
    }

    Renderer *Renderer::Get()
    {
        static Renderer instance;
        return &instance;
    }

    TreeRect Renderer::createGeometry(Parser::Util::Node *node)
    {
        const float padding = 10.0f;
        const float levelSize = 50.0f;
        const float xDist = 100.0f;

        std::string text = node->value.name;
        auto textSize = ImGui::CalcTextSize(text.c_str());

        textSize = ImGui::CalcTextSize(text.c_str());

        this->nodes.push_back({ImVec2(node->x * xDist, node->y * levelSize), ImVec2((padding * 2) + node->x * xDist + textSize.x, (padding * 2) + node->y * levelSize + textSize.y), text, node->value.isTerminal()});
        TreeRect parentNode = this->nodes.back();

        for (auto child : node->getChildren())
        {
            auto childNode = createGeometry(child);

            // link parent to child
            this->edges.push_back({ImVec2(parentNode.min.x + (parentNode.max.x - parentNode.min.x) / 2, parentNode.max.y), ImVec2(childNode.min.x + (childNode.max.x - childNode.min.x) / 2, childNode.min.y)});
        }

        return parentNode;
    }

    void Renderer::render(Parser::Util::ParseTree *parseTree)
    {
        if (this->parseTree != parseTree)
        {
            this->nodes.clear();
            this->edges.clear();
            this->parseTree = parseTree;
            auto dt = buchheim(parseTree);
            createGeometry(parseTree);
        }

        static ImVector<ImVec2> points;
        static ImVec2 scrolling(0.0f, 0.0f);
        static bool adding_line = false;

        // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
        // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
        // To use a child window instead we could use, e.g:
        //      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
        //      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
        //      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
        //      ImGui::PopStyleColor();
        //      ImGui::PopStyleVar();
        //      [...]
        //      ImGui::EndChild();

        // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail(); // Resize canvas to what's available
        if (canvas_sz.x < 50.0f)
            canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f)
            canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        // Draw border and background color
        ImGuiIO &io = ImGui::GetIO();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_hovered = ImGui::IsItemHovered();                            // Hovered
        const bool is_active = ImGui::IsItemActive();                              // Held
        const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
        const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
        {
            scrolling.x += io.MouseDelta.x;
            scrolling.y += io.MouseDelta.y;
        }

        // Context menu (under default mouse threshold)
        ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

        // Draw grid + all lines in the canvas
        draw_list->PushClipRect(canvas_p0, canvas_p1, true);
        // for (int n = 0; n < points.Size; n += 2)
        //     draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);

        for (auto node : nodes)
        {
            node.draw(draw_list, origin);
        }

        for (auto edge : edges)
        {
            edge.draw(draw_list, origin);
        }

        draw_list->PopClipRect();
    }

    void TreeEdge::draw(ImDrawList *drawList, ImVec2 origin)
    {
        ImVec2 startPos = ImVec2(min.x + origin.x, min.y + origin.y);
        ImVec2 endPos = ImVec2(max.x + origin.x, max.y + origin.y);
        drawList->AddLine(startPos, endPos, ImColor(255, 255, 255, 255), 1.0f); // TODO: elbows
    }

    void TreeRect::draw(ImDrawList *drawList, ImVec2 origin)
    {
        float padding = 10.0f;
        ImVec2 minPos = ImVec2(min.x + origin.x, min.y + origin.y);
        ImVec2 maxPos = ImVec2(max.x + origin.x, max.y + origin.y);
        ImVec2 contentPos = ImVec2(padding + minPos.x, padding + minPos.y);
        if (isTerminal)
            drawList->AddRectFilled(minPos, maxPos, ImColor(255, 255, 255, 255));
        else
            drawList->AddRectFilled(minPos, maxPos, ImColor(180, 180, 180, 255));

        drawList->AddText(contentPos, ImColor(0, 0, 0, 255), text.c_str());
    }
}
