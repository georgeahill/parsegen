#pragma once

#include <string>
#include <vector>

namespace ParseGen::GUI
{
    class NonTerminal
    {
    public:
        uint id;
        std::string name;
        bool isStartSymbol;

        inline static uint current_id = 0;

        NonTerminal(std::string name, bool isStart = false) : id(current_id++), name(name), isStartSymbol(isStart)
        {
            NonTerminal::allNonTerminals.push_back(this);
        };

        void rename(std::string newName)
        {
            name = newName;
        };

        inline static std::vector<NonTerminal *> allNonTerminals;
    };
}
