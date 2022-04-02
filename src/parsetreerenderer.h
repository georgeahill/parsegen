#pragma once

#include <vector>
#include <string>
#include <imgui.h>

#include "parsergenerator/util/parsetree.h"

namespace ParseGen::GUI::ParseTree
{
    struct TreeRect
    {
        ImVec2 min;
        ImVec2 max;
        std::string text;
        bool isTerminal;

        void draw(ImDrawList *drawList, ImVec2 origin);
    };

    struct TreeEdge
    {
        ImVec2 min;
        ImVec2 max;

        void draw(ImDrawList *drawList, ImVec2 origin);
    };

    class Renderer
    {
    public:
        static Renderer *Get();

        void render(Parser::Util::ParseTree *parseTree);

    private:
        TreeRect createGeometry(Parser::Util::Node *parseTree);
        Parser::Util::Node *buchheim(Parser::Util::Node *parseTree);
        Parser::Util::Node *firstWalk(Parser::Util::Node *v, float distance);
        Parser::Util::Node *apportion(Parser::Util::Node *v, Parser::Util::Node *defaultAncestor, float distance);
        void executeShifts(Parser::Util::Node *v);
        Parser::Util::Node *ancestor(Parser::Util::Node *vil, Parser::Util::Node *v, Parser::Util::Node *defaultAncestor);
        float secondWalk(Parser::Util::Node *v, float m, float depth, float min);
        Parser::Util::Node *thirdWalk(Parser::Util::Node *tree, float n);
        void moveSubtree(Parser::Util::Node *wl, Parser::Util::Node *wr, float shift);
        Parser::Util::Node *simpleDraw(Parser::Util::Node *parseTree);

    private:
        Parser::Util::ParseTree *parseTree;
        std::vector<TreeRect> nodes;
        std::vector<TreeEdge> edges;
    };
}
