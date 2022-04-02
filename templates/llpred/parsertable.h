#pragma once

#include <string>
#include <map>
#include <vector>

#include <iostream> // FIXME

#include "symbol.h"

class ParserTable
{
public:
    void insert(Util::Symbol nonTerminal, Util::Symbol terminal, std::vector<Util::Symbol> insert)
    {
        this->_map[std::make_pair(nonTerminal, terminal)] = std::vector<Util::Symbol>(insert);

    }

    std::vector<Util::Symbol> get(Util::Symbol nonTerminal, Util::Symbol terminal)
    {
        return this->_map.at(std::make_pair(nonTerminal, terminal));
    }

    bool isSynch(Util::Symbol nonTerminal, Util::Symbol terminal)
    {
        if (isError(nonTerminal, terminal))
            return false;

        auto value = this->_map[std::make_pair(nonTerminal, terminal)];

        if (value.size() == 0)
            return false;

        return value[0].getType() == Util::SymbolType::SYNCH;
    }

    bool isError(Util::Symbol nonTerminal, Util::Symbol terminal)
    {
        if (this->_map.find(std::make_pair(nonTerminal, terminal)) == this->_map.end())
            return true;
        auto vec = this->_map.at(std::make_pair(nonTerminal, terminal));
        if (vec.size() == 0)
            return true;

        return false;
    }

    ParserTable(Util::Symbol start) : start(start){};

public:
    Util::Symbol start;
    // FIRST(α) is the set of terminals that can begin strings derived from α
    std::map<std::vector<Util::Symbol>, std::set<Util::Symbol>> first;

    // FOLLOW(A) is the set of terminals that can immediately follow A
    std::map<Util::Symbol, std::set<Util::Symbol>> follow;

private:
    std::map<std::pair<Util::Symbol, Util::Symbol>, std::vector<Util::Symbol>> _map;
};
