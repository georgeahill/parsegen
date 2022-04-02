#pragma once

#include <string>

namespace Util
{
    enum SymbolType
    {
        TERMINAL,
        NON_TERMINAL,
        EMPTY,
        END,
        SYNCH
    };

    class Symbol
    {
    public:
        std::string name;

        Symbol(std::string name, SymbolType type) : name(name), type(type){};

        bool isEmpty() const
        {
            return (this->type == EMPTY);
        };

        bool isNonTerminal() const
        {
            return (this->type == NON_TERMINAL);
        };

        bool isTerminal() const
        {
            return (this->type == TERMINAL || this->type == EMPTY);
        };

        bool isEnd() const
        {
            return (this->type == END);
        }

        SymbolType getType() const
        {
            return type;
        };

        friend bool operator<(const Symbol &, const Symbol &);
        friend bool operator==(const Symbol &, const Symbol &);
        friend bool operator!=(const Symbol &, const Symbol &);

    protected:
        SymbolType type;
    };
}

#include "symbol.impl"