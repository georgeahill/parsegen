#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>

#include "symbol.h"

namespace ParseGen::Parser::Util
{

    class ParseableInput
    {
    public:
        ParseableInput(std::string input, std::string start) : start({start, NON_TERMINAL})
        {
            std::stringstream ss(input);

            std::string line;
            std::string word;

            while (std::getline(ss, line))
            {

                std::stringstream iss(line);

                Symbol lhs("", NON_TERMINAL);
                std::vector<std::vector<Symbol>> rhses;
                std::vector<Symbol> rhs;

                while (iss >> word)
                {
                    if (lhs.name == "")
                    {
                        lhs.name = word;
                        iss >> word; // remove "::=" operator

                        if (this->start.name == "")
                        {
                            this->start.name = lhs.name;
                        }
                        continue;
                    }

                    if (word[0] == '"')
                    {
                        rhs.emplace_back(word.substr(1, word.size() - 2), TERMINAL);
                    }
                    else if (word == u8"ε" || word == "EMPTY")
                    {
                        rhs.emplace_back(word, EMPTY);
                    }
                    else if (word == "|")
                    {
                        // new production (alternation rule)
                        productions[lhs].push_back(rhs);
                        rhs.clear();
                    }
                    else
                    {
                        rhs.emplace_back(word, NON_TERMINAL);
                    }
                }

                productions[lhs].push_back(rhs);
            }

            for (auto prod : productions)
            {
                calcFirst(prod.first);
            }

            // other symbols we may have missed!
            // this is primarily used in FOLLOW()
            for (auto prod : productions)
            {
                for (auto rule : prod.second)
                    for (auto ruleMember : rule)
                        calcFirst(ruleMember);
            }

            // now calculate FIRST of entire productions

            for (auto prod : productions)
            {
                for (auto rule : prod.second)
                {
                    calcFirstRule(rule);
                }
            }

            // printFirst();

            // repeat until no new rules added
            int oldFollowSize, followSize;
            do
            {
                oldFollowSize = calcFollowSize();
                calcFollow();
                followSize = calcFollowSize();
            } while (oldFollowSize != followSize);

            // printFollow();
        };

    public:
        std::map<Symbol, std::vector<std::vector<Symbol>>> productions;

        // FIRST(α) is the set of terminals that can begin strings derived from α
        std::map<std::vector<Symbol>, std::set<Symbol>> first;

        // FOLLOW(A) is the set of terminals that can immediately follow A
        std::map<Symbol, std::set<Symbol>> follow;

        Symbol start;

    private:
        // TODO: ensure LL(1)?

        void calcFirst(Symbol X)
        {
            // check if we've been calculated already
            if (first.find(std::vector<Symbol>({X})) != std::end(first))
            {
                return;
            }

            if (X.isTerminal() || X.isEmpty())
            {
                this->first[std::vector<Symbol>({X})] = std::set<Symbol>({X});
                return;
            }
            else if (X.isNonTerminal())
            {
                // mark "in progress"
                this->first[std::vector<Symbol>({X})] = std::set<Symbol>();
                for (auto prod : productions)
                {
                    // if X is a nonterminal and X->Y1Y2...Yk is a production,
                    // place a in FIRST(X) if for some i, a is in FIRST(Yi)
                    // and ε is in all of FIRST(Y1),...,FIRST(Yi-1)
                    // If ε is in all FIRST(Yj) for 1<=j<=k

                    if (prod.first.name == X.name) // guaranteed to both be non-term
                    {
                        // X->Y1Y2...Yk, so place a in FIRST(X) if for some i,
                        // a ∈ Y_i and ∀ j=1..(i-1), ε ∈ Y_j

                        bool allSetsContainEpsilon = true;

                        for (auto rule : prod.second)
                        {
                            for (auto Y : rule)
                            {
                                // calculate first[Yi] if not already
                                calcFirst(Y);

                                for (auto first_y_i : first[std::vector<Symbol>({Y})])
                                {
                                    first[std::vector<Symbol>({X})].insert(first_y_i);
                                }

                                // is ε ∈ Y_i?
                                if (first[std::vector<Symbol>({Y})].find(Symbol({u8"ε", EMPTY})) == std::end(first[std::vector<Symbol>({Y})]))
                                {
                                    // time to stop! this set doesn't contain epsilon
                                    // so FIRST(X) can't contain Y(i+1)
                                    allSetsContainEpsilon = false;
                                    break;
                                }
                            }
                        }

                        if (allSetsContainEpsilon)
                        {
                            first[std::vector<Symbol>({X})].insert(Symbol{u8"ε", EMPTY});
                        }
                    }
                }
            }
        };

        void calcFirstRule(std::vector<Symbol> rule)
        {
            // TODO: check this works as expected with epsilon
            bool anyEpsilon = false;
            for (auto symb : rule)
            {
                anyEpsilon = false;
                for (auto xiSymb : first[std::vector<Symbol>({symb})])
                {
                    if (!xiSymb.isEmpty())
                    {
                        first[rule].insert(xiSymb);
                    }
                    else
                    {
                        anyEpsilon = true;
                    }
                }
                if (!anyEpsilon)
                    break;
            }

            if (anyEpsilon)
            {
                first[rule].insert(Symbol{u8"ε", EMPTY});
            }
        };

        void calcFollow()
        {
            // assuming this is first call
            follow[start].insert(Symbol("$", END));

            // if there is a production A->αBβ
            // FOLLOW(B).insert(FIRST(β)) (excl. ε)

            // if there is a production A->αB or A->αBβ where ε ∈ E

            for (auto prod : productions)
            {
                auto A = prod.first;
                for (auto rule : prod.second)
                {
                    for (int idx = 0; idx < rule.size(); ++idx)
                    {
                        if (rule.at(idx).isNonTerminal())
                        {
                            bool containsEpsilon = false;
                            if (idx < rule.size() - 1)
                            {
                                // A->αBβ
                                for (auto firstBeta : first[std::vector<Symbol>({rule.at(idx + 1)})])
                                {
                                    if (!firstBeta.isEmpty())
                                        follow[rule.at(idx)].insert(firstBeta);
                                    else
                                        containsEpsilon = true;
                                }

                                if (containsEpsilon)
                                {
                                }
                            }

                            if (containsEpsilon || idx == rule.size() - 1)
                            {
                                // A->αB or A->αBβ where ε ∈ E

                                // add FOLLOW(A) to FOLLOW(B)
                                for (auto followA : follow[A])
                                    follow[rule.at(idx)].insert(followA);
                            }
                        }
                    }
                }
            }
        }

        void printRules()
        {
            for (auto prod : productions)
            {
                for (auto rule : prod.second)
                {

                    std::cout << prod.first.name << " -> ";
                    for (auto x : rule)
                    {
                        std::cout << x.name /*<< " (" << (x.isTerminal() ? "TERMINAL" : "NON_TERMINAL") << ") "*/;
                    }
                    std::cout << std::endl;
                }
            }
        };

        void printFirst()
        {
            for (auto first_set_pair : first)
            {
                std::cout << "FIRST(";
                for (auto first_lhs_val : first_set_pair.first)
                {
                    std::cout << first_lhs_val.name << ", ";
                }
                std::cout << ") -> ";
                for (auto first_val : first_set_pair.second)
                {
                    std::cout << first_val.name << " ";
                }
                std::cout << std::endl;
            }
        }

        void printFollow()
        {
            for (auto symbolPair : follow)
            {
                std::cout << "FOLLOW(" << symbolPair.first.name << ") -> ";
                for (auto setMember : symbolPair.second)
                {
                    std::cout << setMember.name << " ";
                }
                std::cout << std::endl;
            }
        };

        int calcFollowSize()
        {
            int followSize = 0;
            for (auto symbolPair : follow)
            {
                for (auto setMember : symbolPair.second)
                {
                    ++followSize;
                }
            }
            return followSize;
        };
    };
}
