#include "symbol.h"

namespace ParseGen::Parser::Util
{
    bool operator<(const ParseGen::Parser::Util::Symbol &first, const ParseGen::Parser::Util::Symbol &second)
    {
        if (first.type == ParseGen::Parser::Util::SymbolType::EMPTY && second.type == ParseGen::Parser::Util::SymbolType::EMPTY)
        {
            return false;
        }
        else if (first.type == ParseGen::Parser::Util::SymbolType::END && second.type == ParseGen::Parser::Util::SymbolType::END)
        {
            return false;
        }
        else if (first.type == second.type)
        {
            return first.name < second.name;
        }
        else if (first.type < second.type)
        {
            return true;
        }
        else
        {
            return false;
        }
    };

    bool operator==(const ParseGen::Parser::Util::Symbol &a, const ParseGen::Parser::Util::Symbol &b)
    {
        return !(a < b || b < a);
    }

    bool operator!=(const ParseGen::Parser::Util::Symbol &a, const ParseGen::Parser::Util::Symbol &b)
    {
        return !(a == b);
    }
}
