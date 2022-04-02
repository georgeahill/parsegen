#pragma once

#include <string>
#include <sstream>
#include <map>
#include <vector>

#include <iostream> // FIXME

#include "../util/symbol.h"

namespace ParseGen::Parser::LLPredictiveParser
{
    class ParserTable
    {
    public:
        void insert(Util::Symbol nonTerminal, Util::Symbol terminal, std::vector<Util::Symbol> insert)
        {
            // copy constructor
            // if (nonTerminal.isNonTerminal() && terminal.isTerminal())
            this->_map[std::make_pair(nonTerminal, terminal)] = std::vector<Util::Symbol>(insert);

            // TODO: error messages!
            // if nonTerminal & terminal are wrong type (invalid insert somehow)
            // if already has a value (ambiguous grammar)
        }

        std::vector<Util::Symbol> get(Util::Symbol nonTerminal, Util::Symbol terminal)
        {
            // if (nonTerminal.isNonTerminal() && terminal.isTerminal())
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

        std::string creationCode()
        {
            std::stringstream ss;

            ss << "    "
               << "ParserTable* parseTable = new ParserTable(Util::Symbol(\"" << this->start.name << "\", (Util::SymbolType)" << this->start.getType() << "));" << std::endl;

            for (auto pair : _map)
            {
                ss << "    "
                   << "parseTable->insert(Util::Symbol(\""
                   << pair.first.first.name
                   << "\", (Util::SymbolType)"
                   << pair.first.first.getType()
                   << "), Util::Symbol(\""
                   << pair.first.second.name
                   << "\", (Util::SymbolType)"
                   << pair.first.second.getType()
                   << "), std::vector<Util::Symbol>({";

                for (auto val : pair.second)
                {
                    ss << "{\"" << val.name << "\", (Util::SymbolType)" << val.getType() << "}, ";
                }

                ss << "}));" << std::endl;
            }

            return ss.str();
        }

    public:
        Util::Symbol start;
        // FIRST(α) is the set of terminals that can begin strings derived from α
        std::map<std::vector<Util::Symbol>, std::set<Util::Symbol>> first;

        // FOLLOW(A) is the set of terminals that can immediately follow A
        std::map<Util::Symbol, std::set<Util::Symbol>> follow;

    private:
        // TODO: support LL(k) grammars by changing to std::map<std::pair<Symbol, Symbol>, std::vector<std::vector<Symbol>>> _map;
        std::map<std::pair<Util::Symbol, Util::Symbol>, std::vector<Util::Symbol>> _map;
    };
}
